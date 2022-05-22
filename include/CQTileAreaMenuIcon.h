#ifndef CQTileAreaMenuIcon_H
#define CQTileAreaMenuIcon_H

#include <QLabel>

class CQTileArea;

class CQTileAreaMenuIcon : public QLabel {
  Q_OBJECT

 public:
  CQTileAreaMenuIcon(CQTileArea *area);

  void updateState();

 private:
  void setIcon(const QIcon &icon);

 private:
  CQTileArea *area_ { nullptr }; //!> parent area
};

#endif
