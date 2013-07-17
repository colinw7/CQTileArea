#ifndef CQWidgetResizer_H
#define CQWidgetResizer_H

#include <QWidget>

class QMouseEvent;
class QKeyEvent;

class CQWidgetResizer : public QObject {
  Q_OBJECT

 public:
  enum Action {
    Move   = 0x01,
    Resize = 0x02,
    Any    = Move|Resize
  };

  explicit CQWidgetResizer(QWidget *parent, QWidget *cw = 0);

  void setActive(bool b) { setActive(Any, b); }
  void setActive(Action ac, bool b);

  bool isActive() const { return isActive(Any); }
  bool isActive(Action ac) const;

  void setMovingEnabled(bool b) { movingEnabled_ = b; }
  bool isMovingEnabled() const { return movingEnabled_; }

  bool isButtonDown() const { return buttonDown; }

  void setExtraHeight(int h) { extraHeight_ = h; }
  void setSizeProtection(bool b) { sizeProtect_ = b; }

  void setFrameWidth(int w) { fw_ = w; }

  void doResize();
  void doMove();

 signals:
  void activate();

 protected:
  bool eventFilter(QObject *o, QEvent *e);
  void mouseMoveEvent(QMouseEvent *e);
  void keyPressEvent(QKeyEvent *e);

 private:
  enum MousePosition {
    Nowhere,
    TopLeft, BottomRight, BottomLeft, TopRight,
    Top, Bottom, Left, Right,
    Center
  };

 private:
  void setMouseCursor(MousePosition m);

  bool isMove  () const { return moveResizeMode_ && mode_ == Center; }
  bool isResize() const { return moveResizeMode_ && ! isMove(); }

 private:
  static bool resizeHorizontalDirectionFixed_;
  static bool resizeVerticalDirectionFixed_;

  QWidget       *widget_;
  QWidget       *childWidget_;
  QPoint         moveOffset_;
  QPoint         invMoveOffset_;
  MousePosition  mode_;
  int            fw_;
  int            extraHeight_;
  int            range_;
  bool           buttonDown;
  bool           moveResizeMode_;
  bool           activeForResize_;
  bool           sizeProtect_;
  bool           movingEnabled_;
  bool           activeForMove_;
};

#endif
