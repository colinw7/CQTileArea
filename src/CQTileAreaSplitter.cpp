#include <CQTileAreaSplitter.h>
#include <CQTileArea.h>

#include <QStylePainter>
#include <QStyleOption>
#include <QMouseEvent>

// create splitter
CQTileAreaSplitter::
CQTileAreaSplitter(CQTileArea *area) :
 QWidget(area), area_(area)
{
  setCursor(Qt::SplitHCursor);

  setVisible(false);
}

void
CQTileAreaSplitter::
init(Qt::Orientation orient, int pos, int ind)
{
  orient_ = orient;
  pos_    = pos;
  ind_    = ind;

  if (orient_ == Qt::Vertical)
    setCursor(Qt::SplitHCursor);
  else
    setCursor(Qt::SplitVCursor);
}

void
CQTileAreaSplitter::
setUsed(bool used)
{
  used_ = used;

  setVisible(used_);
}

void
CQTileAreaSplitter::
paintEvent(QPaintEvent *)
{
  QStylePainter ps(this);

  QStyleOption opt;

  opt.initFrom(this);

  opt.rect  = rect();
  opt.state = (orient_ == Qt::Horizontal ? QStyle::State_None : QStyle::State_Horizontal);

  if (mouseState_.pressed)
    opt.state |= QStyle::State_Sunken;

  if (mouseOver_)
    opt.state |= QStyle::State_MouseOver;

  ps.drawControl(QStyle::CE_Splitter, opt);
}

// handle mouse press (start move of splitter)
void
CQTileAreaSplitter::
mousePressEvent(QMouseEvent *e)
{
  mouseState_.pressed  = true;
  mouseState_.pressPos = e->globalPos();

  update();
}

// handle mouse move (move splitter to resize placement areas or highlight splitter under mouse)
void
CQTileAreaSplitter::
mouseMoveEvent(QMouseEvent *e)
{
  if (! mouseState_.pressed) return;

  if (orient_ == Qt::Horizontal) {
    int dy = e->globalPos().y() - mouseState_.pressPos.y();

    area_->moveHSplitter(pos_, ind_, dy);
  }
  else {
    int dx = e->globalPos().x() - mouseState_.pressPos.x();

    area_->moveVSplitter(pos_, ind_, dx);
  }

  mouseState_.pressPos = e->globalPos();

  update();
}

// handle mouse release
void
CQTileAreaSplitter::
mouseReleaseEvent(QMouseEvent *)
{
  mouseState_.pressed = false;

  update();
}

void
CQTileAreaSplitter::
enterEvent(QEvent *)
{
  mouseOver_ = true;

  update();
}

void
CQTileAreaSplitter::
leaveEvent(QEvent *)
{
  mouseOver_ = false;

  update();
}
