#include <CQTileAreaMenuIcon.h>
#include <CQTileArea.h>
#include <CQTileWindow.h>

#include <QIcon>

CQTileAreaMenuIcon::
CQTileAreaMenuIcon(CQTileArea *area) :
 QLabel(area), area_(area)
{
  setObjectName("menuIcon");

  QFontMetrics fm(font());

  int s = fm.height() - 2;

  setFixedSize(s, s);
}

void
CQTileAreaMenuIcon::
updateState()
{
  auto *window = area_->currentWindow();

  if (window)
    setIcon(window->getIcon());
  else
    setIcon(QIcon());
}

void
CQTileAreaMenuIcon::
setIcon(const QIcon &icon)
{
  int s = height();

  if (! icon.isNull())
    setPixmap(icon.pixmap(s,s));
  else
    setPixmap(QPixmap(s,s));
}
