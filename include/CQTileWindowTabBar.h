#ifndef CQTileWindowTabBar_H
#define CQTileWindowTabBar_H

#include <QTabBar>

class CQTileWindowArea;
class QMenu;

class CQTileWindowTabBar : public QTabBar {
  Q_OBJECT

 public:
  //! create tabbar
  CQTileWindowTabBar(CQTileWindowArea *area);

 private:
  //! handle mouse press
  void mousePressEvent(QMouseEvent *e) override;

  //! display context menu
  void contextMenuEvent(QContextMenuEvent *e) override;

 private:
  CQTileWindowArea *area_        { nullptr }; //!< parent area
  QMenu            *contextMenu_ { nullptr }; //!< context menu
};

#endif
