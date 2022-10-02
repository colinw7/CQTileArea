#include <CQWidgetResizer.h>
#include <CQWidgetUtil.h>

#include <QFrame>
#include <QApplication>
#include <QDesktopWidget>
#include <QCursor>
#include <QSizeGrip>
#include <QMouseEvent>
#include <QScreen>

namespace {
  const int RANGE = 4;
}

bool CQWidgetResizer::resizeHorizontalDirectionFixed_ = false;
bool CQWidgetResizer::resizeVerticalDirectionFixed_   = false;

CQWidgetResizer::
CQWidgetResizer(QWidget *parent, QWidget *cw) :
 QObject(parent), widget_(parent), childWidget_(cw ? cw : parent),
 fw_(0), extraHeight_(0), buttonDown_(false), moveResizeMode_(false),
 sizeProtect_(true), movingEnabled_(true)
{
  mode_ = Nowhere;

  widget_->setMouseTracking(true);

  auto *frame = qobject_cast<QFrame*>(widget_);

  range_ = frame ? frame->frameWidth() : RANGE;
  range_ = qMax(RANGE, range_);

  activeForMove_ = activeForResize_ = true;

  widget_->installEventFilter(this);
}

void
CQWidgetResizer::
setActive(Action ac, bool b)
{
  if (ac & Move  ) activeForMove_   = b;
  if (ac & Resize) activeForResize_ = b;

  if (! isActive())
    setMouseCursor(Nowhere);
}

bool
CQWidgetResizer::
isActive(Action ac) const
{
  bool b = false;

  if (ac & Move  ) b  = activeForMove_;
  if (ac & Resize) b |= activeForResize_;

  return b;
}

bool
CQWidgetResizer::
eventFilter(QObject *o, QEvent *ee)
{
  if (! isActive() ||
      (ee->type() != QEvent::MouseButtonPress &&
       ee->type() != QEvent::MouseButtonRelease &&
       ee->type() != QEvent::MouseMove &&
       ee->type() != QEvent::KeyPress &&
       ee->type() != QEvent::ShortcutOverride))
    return false;

  Q_ASSERT(o == widget_);

  auto *w = widget_;

  if (QApplication::activePopupWidget()) {
    if (isButtonDown() && ee->type() == QEvent::MouseButtonRelease)
      buttonDown_ = false;

    return false;
  }

  auto *e = static_cast<QMouseEvent *>(ee);

  switch (e->type()) {
    case QEvent::MouseButtonPress: {
      if (w->isMaximized())
        break;

      if (! widget_->rect().contains(widget_->mapFromGlobal(e->globalPos())))
        return false;

      if (e->button() == Qt::LeftButton) {
        /* Implicit grabs do not stop the X server from changing
           the cursor in children, which looks *really* bad when
           doing resizing, so we grab the cursor. Note that we do
           not do this on Windows since double clicks are lost due
           to the grab (see change 198463). */
        if (e->spontaneous())
          widget_->grabMouse(widget_->cursor());

        buttonDown_ = false;

        emit activate();

        bool me = isMovingEnabled();

        setMovingEnabled(me && o == widget_);

        mouseMoveEvent(e);

        setMovingEnabled(me);

        buttonDown_ = true;

        tlOffset_ = widget_->mapFromGlobal(e->globalPos());
        brOffset_ = widget_->rect().bottomRight() - tlOffset_;

        if (mode_ == Center) {
          if (isMovingEnabled())
            return true;
        }
        else
          return true;
      }

      break;
    }
    case QEvent::MouseButtonRelease: {
      if (w->isMaximized())
        break;

      if (e->button() == Qt::LeftButton) {
        moveResizeMode_ = false;
        buttonDown_     = false;

        widget_->releaseMouse();
        widget_->releaseKeyboard();

        if (mode_ == Center) {
          if (isMovingEnabled())
            return true;
        }
        else
          return true;
      }

      break;
    }
    case QEvent::MouseMove: {
      if (w->isMaximized())
        break;

      buttonDown_ = buttonDown_ && (e->buttons() & Qt::LeftButton); // safety, state machine broken!

      bool me = isMovingEnabled();

      setMovingEnabled(me && o == widget_ && (buttonDown_ || moveResizeMode_));

      mouseMoveEvent(e);

      setMovingEnabled(me);

      if (mode_ == Center) {
        if (isMovingEnabled())
          return true;
      }
      else
        return true;

      break;
    }
    case QEvent::KeyPress: {
      keyPressEvent(static_cast<QKeyEvent *>(ee));

      break;
    }
    case QEvent::ShortcutOverride: {
      if (isButtonDown()) {
        static_cast<QKeyEvent *>(ee)->accept();
        return true;
      }

      break;
    }
    default:
      break;
  }

  return false;
}

void
CQWidgetResizer::
mouseMoveEvent(QMouseEvent *e)
{
  auto pos = widget_->mapFromGlobal(e->globalPos());

  if (! moveResizeMode_ && ! isButtonDown()) {
    if      (pos.y() <= range_ && pos.x() <= range_)
      mode_ = TopLeft;
    else if (pos.y() >= widget_->height() - range_ && pos.x() >= widget_->width() - range_)
      mode_ = BottomRight;
    else if (pos.y() >= widget_->height() - range_ && pos.x() <= range_)
      mode_ = BottomLeft;
    else if (pos.y() <= range_ && pos.x() >= widget_->width() - range_)
      mode_ = TopRight;
    else if (pos.y() <= range_)
      mode_ = Top;
    else if (pos.y() >= widget_->height() - range_)
      mode_ = Bottom;
    else if (pos.x() <= range_)
      mode_ = Left;
    else if (pos.x() >= widget_->width() - range_)
      mode_ = Right;
    else if (widget_->rect().contains(pos))
      mode_ = Center;
    else
      mode_ = Nowhere;

    if (widget_->isMinimized() || ! isActive(Resize))
      mode_ = Center;

    setMouseCursor(mode_);

    return;
  }

  if (mode_ == Center && ! isMovingEnabled())
    return;

  if (widget_->testAttribute(Qt::WA_WState_ConfigPending))
    return;

  auto globalPos = (! widget_->isWindow() && widget_->parentWidget()) ?
                    widget_->parentWidget()->mapFromGlobal(e->globalPos()) : e->globalPos();

  if (! widget_->isWindow() && ! widget_->parentWidget()->rect().contains(globalPos)) {
    if (globalPos.x() < 0) globalPos.rx() = 0;
    if (globalPos.y() < 0) globalPos.ry() = 0;

    if (sizeProtect_ && globalPos.x() > widget_->parentWidget()->width ())
      globalPos.rx() = widget_->parentWidget()->width();
    if (sizeProtect_ && globalPos.y() > widget_->parentWidget()->height())
      globalPos.ry() = widget_->parentWidget()->height();
  }

  auto p  = globalPos + brOffset_;
  auto pp = globalPos - tlOffset_;

  // Workaround for window managers which refuse to move a tool window partially offscreen.
  auto desktop = CQWidgetUtil::desktopAvailableGeometry(widget_);

  pp.rx() = qMax(pp.x(), desktop.left  ());
  pp.ry() = qMax(pp.y(), desktop.top   ());
  p .rx() = qMin( p.x(), desktop.right ());
  p .ry() = qMin( p.y(), desktop.bottom());

  auto ms = CQWidgetUtil::SmartMinSize(childWidget_);

  int mw = ms.width();
  int mh = ms.height();

  if (childWidget_ != widget_) {
    mw += 2*fw_;
    mh += 2*fw_ + extraHeight_;
  }

  QSize maxsize(childWidget_->maximumSize());

  if (childWidget_ != widget_)
    maxsize += QSize(2*fw_, 2*fw_ + extraHeight_);

  QSize mpsize(widget_->geometry().right () - pp.x() + 1,
               widget_->geometry().bottom() - pp.y() + 1);

  mpsize = mpsize.expandedTo(widget_->minimumSize()).expandedTo(QSize(mw, mh)).boundedTo(maxsize);

  QPoint mp(widget_->geometry().right () - mpsize.width () + 1,
            widget_->geometry().bottom() - mpsize.height() + 1);

  auto geom = widget_->geometry();

  switch (mode_) {
    case TopLeft:
      geom = QRect(mp, widget_->geometry().bottomRight());
      break;
    case BottomRight:
      geom = QRect(widget_->geometry().topLeft(), p);
      break;
    case BottomLeft:
      geom = QRect(QPoint(mp.x(), widget_->geometry().y()),
                   QPoint(widget_->geometry().right(), p.y()));
      break;
    case TopRight:
      geom = QRect(QPoint(widget_->geometry().x(), mp.y()),
                   QPoint(p.x(), widget_->geometry().bottom()));
      break;
    case Top:
      geom = QRect(QPoint(widget_->geometry().left(), mp.y()), widget_->geometry().bottomRight());
      break;
    case Bottom:
      geom = QRect(widget_->geometry().topLeft(), QPoint(widget_->geometry().right(), p.y()));
      break;
    case Left:
      geom = QRect(QPoint(mp.x(), widget_->geometry().top()), widget_->geometry().bottomRight());
      break;
    case Right:
      geom = QRect(widget_->geometry().topLeft(), QPoint(p.x(), widget_->geometry().bottom()));
      break;
    case Center:
      geom.moveTopLeft(pp);
      break;
    default:
      break;
  }

  geom = QRect(geom.topLeft(), geom.size().expandedTo(widget_->minimumSize()).
                                           expandedTo(QSize(mw, mh)).boundedTo(maxsize));

  if (geom != widget_->geometry() &&
      (widget_->isWindow() || widget_->parentWidget()->rect().intersects(geom))) {
    if (mode_ == Center)
      widget_->move(geom.topLeft());
    else
      widget_->setGeometry(geom);
  }

  //QApplication::syncX();
}

void
CQWidgetResizer::
setMouseCursor(MousePosition m)
{
  auto children = widget_->children();

  for (int i = 0; i < children.size(); ++i) {
    if (auto *w = qobject_cast<QWidget*>(children.at(i))) {
      if (! w->testAttribute(Qt::WA_SetCursor) && ! w->inherits("QWorkspaceTitleBar")) {
        w->setCursor(Qt::ArrowCursor);
      }
    }
  }

  switch (m) {
    case TopLeft:
    case BottomRight:
      widget_->setCursor(Qt::SizeFDiagCursor);
      break;
    case BottomLeft:
    case TopRight:
      widget_->setCursor(Qt::SizeBDiagCursor);
      break;
    case Top:
    case Bottom:
      widget_->setCursor(Qt::SizeVerCursor);
      break;
    case Left:
    case Right:
      widget_->setCursor(Qt::SizeHorCursor);
      break;
    default:
      widget_->setCursor(Qt::ArrowCursor);
      break;
  }
}

void
CQWidgetResizer::
keyPressEvent(QKeyEvent *e)
{
  if (! isMove() && ! isResize())
    return;

  bool is_control = e->modifiers() & Qt::ControlModifier;

  int delta = is_control ? 1 : 8;

  auto pos = QCursor::pos();

  switch (e->key()) {
    case Qt::Key_Left: {
      pos.rx() -= delta;

      if (pos.x() <= QApplication::desktop()->geometry().left()) {
        if (mode_ == TopLeft || mode_ == BottomLeft) {
          tlOffset_.rx() += delta;
          brOffset_.rx() += delta;
        }
        else {
          tlOffset_.rx() -= delta;
          brOffset_.rx() -= delta;
        }
      }

      if (isResize() && ! resizeHorizontalDirectionFixed_) {
        resizeHorizontalDirectionFixed_ = true;

        if      (mode_ == BottomRight)
          mode_ = BottomLeft;
        else if (mode_ == TopRight)
          mode_ = TopLeft;

        setMouseCursor(mode_);

        widget_->grabMouse(widget_->cursor());
      }

      break;
    }
    case Qt::Key_Right: {
      pos.rx() += delta;

      if (pos.x() >= QApplication::desktop()->geometry().right()) {
        if (mode_ == TopRight || mode_ == BottomRight) {
          tlOffset_.rx() += delta;
          brOffset_.rx() += delta;
        }
        else {
          tlOffset_.rx() -= delta;
          brOffset_.rx() -= delta;
        }
      }

      if (isResize() && ! resizeHorizontalDirectionFixed_) {
        resizeHorizontalDirectionFixed_ = true;

        if      (mode_ == BottomLeft)
          mode_ = BottomRight;
        else if (mode_ == TopLeft)
          mode_ = TopRight;

        setMouseCursor(mode_);

        widget_->grabMouse(widget_->cursor());
      }

      break;
    }
    case Qt::Key_Up: {
      pos.ry() -= delta;

      if (pos.y() <= QApplication::desktop()->geometry().top()) {
        if (mode_ == TopLeft || mode_ == TopRight) {
          tlOffset_.ry() += delta;
          brOffset_.ry() += delta;
        }
        else {
          tlOffset_.ry() -= delta;
          brOffset_.ry() -= delta;
        }
      }

      if (isResize() && ! resizeVerticalDirectionFixed_) {
        resizeVerticalDirectionFixed_ = true;

        if      (mode_ == BottomLeft)
          mode_ = TopLeft;
        else if (mode_ == BottomRight)
          mode_ = TopRight;

        setMouseCursor(mode_);

        widget_->grabMouse(widget_->cursor());
      }

      break;
    }
    case Qt::Key_Down: {
      pos.ry() += delta;

      if (pos.y() >= QApplication::desktop()->geometry().bottom()) {
        if (mode_ == BottomLeft || mode_ == BottomRight) {
          tlOffset_.ry() += delta;
          brOffset_.ry() += delta;
        }
        else {
          tlOffset_.ry() -= delta;
          brOffset_.ry() -= delta;
        }
      }

      if (isResize() && ! resizeVerticalDirectionFixed_) {
        resizeVerticalDirectionFixed_ = true;

        if      (mode_ == TopLeft)
          mode_ = BottomLeft;
        else if (mode_ == TopRight)
          mode_ = BottomRight;

        setMouseCursor(mode_);

        widget_->grabMouse(widget_->cursor());
      }

      break;
    }
    case Qt::Key_Space:
    case Qt::Key_Return:
    case Qt::Key_Enter:
    case Qt::Key_Escape: {
      moveResizeMode_ = false;

      widget_->releaseMouse();
      widget_->releaseKeyboard();

      buttonDown_ = false;

      break;
    }
    default:
      return;
  }

  QCursor::setPos(pos);
}

void
CQWidgetResizer::
doResize()
{
  if (! activeForResize_)
    return;

  moveResizeMode_ = true;
  tlOffset_       = widget_->mapFromGlobal(QCursor::pos());

  if (tlOffset_.x() < widget_->width()/2) {
    if (tlOffset_.y() < widget_->height()/2)
      mode_ = TopLeft;
    else
      mode_ = BottomLeft;
  }
  else {
    if (tlOffset_.y() < widget_->height()/2)
      mode_ = TopRight;
    else
      mode_ = BottomRight;
  }

  brOffset_ = widget_->rect().bottomRight() - tlOffset_;

  setMouseCursor(mode_);

  widget_->grabMouse(widget_->cursor());
  widget_->grabKeyboard();

  resizeHorizontalDirectionFixed_ = false;
  resizeVerticalDirectionFixed_   = false;
}

void
CQWidgetResizer::
doMove()
{
  if (! activeForMove_)
    return;

  mode_           = Center;
  moveResizeMode_ = true;

  tlOffset_ = widget_->mapFromGlobal(QCursor::pos());
  brOffset_ = widget_->rect().bottomRight() - tlOffset_;

  widget_->grabMouse(Qt::SizeAllCursor);
  widget_->grabKeyboard();
}
