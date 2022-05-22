#ifndef CQTileAreaSplitter_H
#define CQTileAreaSplitter_H

#include <QWidget>

class CQTileArea;

//! splitter widget
class CQTileAreaSplitter : public QWidget {
  Q_OBJECT

 public:
  //! create splitter
  CQTileAreaSplitter(CQTileArea *area);

  //! set orientation and splitter key
  void init(Qt::Orientation orient, int pos, int ind);

  //! get/set used
  bool used() const { return used_; }
  void setUsed(bool used);

  //! handle mouse events
  void mousePressEvent  (QMouseEvent *e) override;
  void mouseMoveEvent   (QMouseEvent *e) override;
  void mouseReleaseEvent(QMouseEvent *e) override;

  //! handle enter/leave events
  void enterEvent(QEvent *e) override;
  void leaveEvent(QEvent *e) override;

 private:
  //! handle paint
  void paintEvent(QPaintEvent *) override;

 private:
  //! structure for mouse state
  struct MouseState {
    bool   pressed { false }; //!< mouse pressed
    QPoint pressPos;          //!< mouse press position

    MouseState() { }
  };

  CQTileArea      *area_       { nullptr };      //!< parent area
  Qt::Orientation  orient_     { Qt::Vertical }; //!< orientation
  int              pos_        { -1 };           //!< position (row or column)
  int              ind_        { -1 };           //!< splitter index
  bool             used_       { false };        //!< used
  MouseState       mouseState_;                  //!< mouse state
  bool             mouseOver_  { false };        //!< mouseOver
};

#endif
