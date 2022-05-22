#ifndef CQTileWindow_H
#define CQTileWindow_H

#include <QWidget>

class CQTileWindowArea;

//! class to represent a tile window
//! a tile window has a titlebar
class CQTileWindow : public QWidget {
  Q_OBJECT

 public:
  //! create window
  CQTileWindow(CQTileWindowArea *area);

  //! destroy window
 ~CQTileWindow();

  //! get parent window area
  CQTileWindowArea *area() const { return area_; }

  //! set child widget
  void setWidget(QWidget *w);
  //! get child widget
  QWidget *widget() const { return w_; }

  //! get title
  QString getTitle() const;

  //! get icon
  QIcon getIcon() const;

 private:
  friend class CQTileArea;
  friend class CQTileWindowArea;

  //! is current valid (not being destroyed)
  bool isValid() const { return valid_; }

  //! set parent window area
  void setArea(CQTileWindowArea *area);

  //! handle hover events
  bool event(QEvent *e) override;

  //! handle close event
  void closeEvent(QCloseEvent *closeEvent) override;

 private slots:
  //! handle focus change
  void focusChangedSlot(QWidget *old, QWidget *now);

  //! handle child widget destroyed
  void widgetDestroyed();

 private:
  CQTileWindowArea *area_  { nullptr }; //!< parent area
  QWidget          *w_     { nullptr }; //!< child widget
  bool              valid_ { false };   //!< is child widget valid
};

#endif
