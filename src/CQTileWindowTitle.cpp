#include <CQTileWindowTitle.h>
#include <CQTileWindowArea.h>
#include <CQTileAreaConstants.h>

#include <QMenu>
#include <QMouseEvent>
#include <QApplication>

#include <images/maximize.xpm>
#include <images/attach.xpm>
#include <images/detach.xpm>
#include <images/restore.xpm>
#include <images/close.xpm>

// create title bar
CQTileWindowTitle::
CQTileWindowTitle(CQTileWindowArea *area) :
 area_(area), contextMenu_(nullptr)
{
  // create detach/attach, maximize/restore and close buttons
  // TODO: tile button
  detachButton_   = addButton(QPixmap(detach_data  ));
  maximizeButton_ = addButton(QPixmap(maximize_data));
  closeButton_    = addButton(QPixmap(close_data   ));

  connect(detachButton_  , SIGNAL(clicked()), this, SLOT(detachSlot()));
  connect(maximizeButton_, SIGNAL(clicked()), this, SLOT(maximizeSlot()));
  connect(closeButton_   , SIGNAL(clicked()), area, SLOT(closeSlot()));

  detachButton_  ->setToolTip("Detach");
  maximizeButton_->setToolTip("Maximize");
  closeButton_   ->setToolTip("Close");

  // add hover for cursor update
  setAttribute(Qt::WA_Hover);

  // enable context menu
  setContextMenuPolicy(Qt::DefaultContextMenu);

  // no focus (except when dragging)
  setFocusPolicy(Qt::NoFocus);
}

// update from window state
void
CQTileWindowTitle::
updateState()
{
  // update icon for detach/attach, maximize/restore buttons from state
  maximizeButton_->setIcon(area_->isMaximized() ? QPixmap(restore_data) : QPixmap(maximize_data));
  detachButton_  ->setIcon(! area_->isDocked () ? QPixmap(attach_data ) : QPixmap(detach_data  ));

  maximizeButton_->setToolTip(area_->isMaximized() ? "Restore" : "Maximize");
  detachButton_  ->setToolTip(! area_->isDocked () ? "Attach"  : "Detach"  );

  if (area_->isMaximized())
    maximizeButton_->setEnabled(area_->area()->isRestoreStateValid());
  else
    maximizeButton_->setEnabled(true);
}

// handle detach/attach
void
CQTileWindowTitle::
detachSlot()
{
  if (! area_->isDocked())
    area_->attachSlot();
  else
    area_->detachSlot();
}

// handle maximize/restore
void
CQTileWindowTitle::
maximizeSlot()
{
  if (area_->isMaximized())
    area_->restoreSlot();
  else
    area_->maximizeSlot();
}

QString
CQTileWindowTitle::
title() const
{
  return area_->getTitle();
}

QIcon
CQTileWindowTitle::
icon() const
{
  return area_->getIcon();
}

QColor
CQTileWindowTitle::
backgroundColor() const
{
  auto *tileArea = area_->area();

  bool current = (area_ == tileArea->currentArea());

  return (current ? tileArea->titleActiveColor() : tileArea->titleInactiveColor());
}

QColor
CQTileWindowTitle::
barColor() const
{
  auto *tileArea = area_->area();

  bool current = (area_ == tileArea->currentArea());

  return (current ? CQTileAreaConstants::bar_active_fg : CQTileAreaConstants::bar_inactive_fg);
}

// display context menu
void
CQTileWindowTitle::
contextMenuEvent(QContextMenuEvent *e)
{
  assert(area_);

  if (! contextMenu_)
    contextMenu_ = area_->createContextMenu(this);

  area_->updateContextMenu(contextMenu_);

  contextMenu_->popup(e->globalPos());

  e->accept();
}

// handle mouse press
void
CQTileWindowTitle::
mousePressEvent(QMouseEvent *e)
{
  // init mouse state
  mouseState_.reset();

  mouseState_.pressed  = true;
  mouseState_.pressPos = e->globalPos();
  mouseState_.dragAll  = (e->modifiers() & Qt::ShiftModifier);

  // ensure we are the current area
  area_->area()->setCurrentArea(area_);

  // grab key focus
  setFocusPolicy(Qt::StrongFocus);

  // initialize attach data
  area_->initAttach();
}

// handle mouse move
void
CQTileWindowTitle::
mouseMoveEvent(QMouseEvent *e)
{
  // ignore if mouse not pressed or user escaped operation
  if (! mouseState_.pressed || mouseState_.escapePress) return;

  // if not moved yet ensure move far enough
  if (! mouseState_.moved) {
    if ((e->globalPos() - mouseState_.pressPos).manhattanLength() <
         QApplication::startDragDistance())
      return;

    mouseState_.moved = true;
  }

  // detach and start animate
  if (area_->isDocked()) {
    area_->detach(e->globalPos(), true, mouseState_.dragAll);

    if (area_->area()->animateDrag())
      area_->startAttachPreview();
  }

  // move detached window
  int dx = e->globalPos().x() - mouseState_.pressPos.x();
  int dy = e->globalPos().y() - mouseState_.pressPos.y();

  area_->move(area_->pos() + QPoint(dx, dy));

  mouseState_.pressPos = e->globalPos();

  // show drop bound position (animated if needed)
  if (area_->area()->animateDrag())
    area_->doAttachPreview();
  else
    area_->area()->setHighlight(mouseState_.pressPos);

  // redraw
  area_->area()->update();
}

// handle mouse release
void
CQTileWindowTitle::
mouseReleaseEvent(QMouseEvent *)
{
  // attach (if escape not pressed)
  if (! mouseState_.escapePress) {
    // stop animating
    if (area_->area()->animateDrag())
      area_->stopAttachPreview();

    //---

    CQTileArea::Side side;
    int              row1, col1, row2, col2;

    if (area_->area()->getHighlightPos(side, row1, col1, row2, col2))
      area_->attach(false);
    else {
      if (mouseState_.moved)
        area_->setFloated();
    }

    area_->area()->clearHighlight();
  }

  // reset state
  mouseState_.reset();

  setFocusPolicy(Qt::NoFocus);

  // update titles
  area_->area()->updateTitles();
}

// double click maximizes/restores
void
CQTileWindowTitle::
mouseDoubleClickEvent(QMouseEvent *)
{
  maximizeSlot();
}

// handle key press (escape cancel)
// TODO: other shortcuts
void
CQTileWindowTitle::
keyPressEvent(QKeyEvent *e)
{
  if (e->key() == Qt::Key_Escape) {
    if (! mouseState_.escapePress && mouseState_.pressed && ! area_->isDocked()) {
      mouseState_.escapePress = true;

      area_->cancelAttach();

      area_->area()->clearHighlight();
    }
  }
}

// update cursor
bool
CQTileWindowTitle::
event(QEvent *e)
{
  switch (e->type()) {
    case QEvent::HoverEnter:
    case QEvent::HoverMove: {
      if (insideTitle(dynamic_cast<QHoverEvent *>(e)->pos()))
        setCursor(Qt::SizeAllCursor);
      else
        setCursor(Qt::ArrowCursor);
      break;
    }
    case QEvent::HoverLeave:
      unsetCursor();
      break;
    default:
      break;
  }

  return QWidget::event(e);
}
