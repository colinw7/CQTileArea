#ifndef CQWidgetResizer_H
#define CQWidgetResizer_H

#include <QWidget>

class QMouseEvent;
class QKeyEvent;

class CQWidgetResizer : public QObject {
  Q_OBJECT

 public:
  // action types
  enum Action {
    Move   = (1<<0),
    Resize = (1<<1),
    Any    = Move|Resize
  };

  explicit CQWidgetResizer(QWidget *parent, QWidget *cw = 0);

  // set active for move and resize
  void setActive(bool b) { setActive(Any, b); }

  // set active for move or resize
  void setActive(Action ac, bool b);

  // is active for move and resize
  bool isActive() const { return isActive(Any); }

  // is active for move or resize
  bool isActive(Action ac) const;

  // enable/disable move
  void setMovingEnabled(bool b) { movingEnabled_ = b; }
  // is move enabled
  bool isMovingEnabled() const { return movingEnabled_; }

  // is button pressed (moving/resizing)
  bool isButtonDown() const { return buttonDown_; }

  // extra height for min/max size
  void setExtraHeight(int h) { extraHeight_ = h; }

  // ensure not resized bigger than parent size
  void setSizeProtection(bool b) { sizeProtect_ = b; }

  // set frame widget (border for resize handle detection)
  void setFrameWidth(int w) { fw_ = w; }

  // activate resize mode (usually from menu)
  void doResize();

  // activate move mode (usually from menu)
  void doMove();

 signals:
  // signal emitted when resize/move started
  void activate();

 protected:
  // handle widget events
  bool eventFilter(QObject *o, QEvent *e);

  // handle move event
  void mouseMoveEvent(QMouseEvent *e);
  // handle key event
  void keyPressEvent(QKeyEvent *e);

 private:
  // valid mouse positions
  enum MousePosition {
    Nowhere,
    TopLeft, BottomRight, BottomLeft, TopRight,
    Top    , Bottom     , Left      , Right,
    Center
  };

 private:
  // set cursor for position
  void setMouseCursor(MousePosition m);

  // is move mode active
  bool isMove  () const { return moveResizeMode_ && mode_ == Center; }
  // is resize mode active
  bool isResize() const { return moveResizeMode_ && ! isMove(); }

 private:
  // has resize direction been fixed in resize mode
  static bool resizeHorizontalDirectionFixed_;
  static bool resizeVerticalDirectionFixed_;

  QWidget       *widget_;          // parent widget
  QWidget       *childWidget_;     // optional child widget (for size constaint)
  QPoint         tlOffset_;        // move offset (from top left)
  QPoint         brOffset_;        // move offset (from bottom right)
  MousePosition  mode_;            // mouse mode (resize/move position)
  int            fw_;              // frame width
  int            extraHeight_;     // extra height
  int            range_;           // resize border range
  bool           buttonDown_;      // is mouse button pressed
  bool           moveResizeMode_;  // in move/resize mode
  bool           activeForMove_;   // move allowed
  bool           activeForResize_; // resize allowed
  bool           sizeProtect_;     // is size limited to parent size
  bool           movingEnabled_;   // is move enabled
};

#endif
