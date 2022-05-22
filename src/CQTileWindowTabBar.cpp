#include <CQTileWindowTabBar.h>
#include <CQTileWindowArea.h>

#include <QMenu>
#include <QContextMenuEvent>

// create tabbar
CQTileWindowTabBar::
CQTileWindowTabBar(CQTileWindowArea *area) :
 area_(area), contextMenu_(nullptr)
{
  setObjectName("tabbar");

  setShape(QTabBar::RoundedSouth);
  setMovable(true);

  setContextMenuPolicy(Qt::DefaultContextMenu);
}

// mouse press activates current window
void
CQTileWindowTabBar::
mousePressEvent(QMouseEvent *e)
{
  area_->area()->setCurrentArea(area_);

  QTabBar::mousePressEvent(e);
}

// display context menu
void
CQTileWindowTabBar::
contextMenuEvent(QContextMenuEvent *e)
{
  if (! contextMenu_)
    contextMenu_ = area_->createContextMenu(this);

  area_->updateContextMenu(contextMenu_);

  contextMenu_->popup(e->globalPos());

  e->accept();
}
