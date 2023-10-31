#ifndef CQTileWindowTitle_H
#define CQTileWindowTitle_H

#include <CQTitleBar.h>

class CQTileWindowArea;

//! widget for window area title bar
class CQTileWindowTitle : public CQTitleBar {
  Q_OBJECT

 public:
  //! create title
  CQTileWindowTitle(CQTileWindowArea *area);

  //! update state
  void updateState();

 private:
  //! get title
  QString title() const override;

  //! get icon
  QIcon icon() const override;

  //! get background color
  QColor backgroundColor() const override;

  //! get bar's color
  QColor barColor() const override;

  //! handle mouse events
  void mousePressEvent  (QMouseEvent *e) override;
  void mouseMoveEvent   (QMouseEvent *e) override;
  void mouseReleaseEvent(QMouseEvent *e) override;

  //! handle double click (expand)
  void mouseDoubleClickEvent(QMouseEvent *e) override;

  //! handle key press (escape)
  void keyPressEvent(QKeyEvent *e) override;

  //! display context menu
  void contextMenuEvent(QContextMenuEvent *e) override;

  //! handle hover
  bool event(QEvent *e) override;

 private Q_SLOTS:
  //! detach
  void detachSlot();
  //! maximize
  void maximizeSlot();

 private:
  struct MouseState {
    bool   pressed    { false }; //!< is mouse pressed
    bool   moved      { false }; //!< has move started
    bool   escapePress{ false }; //!< escape pressed
    QPoint pressPos;             //!< press position
    bool   dragAll    { false }; //!< drag all tabs

    MouseState() {
      reset();
    }

    void reset() {
      pressed     = false;
      moved       = false;
      escapePress = false;
      dragAll     = false;
    }
  };

  CQTileWindowArea *area_           { nullptr }; //!< parent area
  MouseState        mouseState_;                 //!< current mouse state
  CQTitleBarButton *detachButton_   { nullptr }; //!< attach/detach button
  CQTitleBarButton *maximizeButton_ { nullptr }; //!< maximize/restore button
  CQTitleBarButton *closeButton_    { nullptr }; //!< close button
  QMenu            *contextMenu_    { nullptr }; //!< context menu
};

#endif
