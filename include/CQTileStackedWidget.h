#ifndef CQTileStackedWidget_H
#define CQTileStackedWidget_H

#include <QWidget>

class CQTileWindowArea;

//! class for widget stack
//! (all stack widgets are visible, just raised)
class CQTileStackedWidget : public QWidget {
 Q_OBJECT

  Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentChanged)
  Q_PROPERTY(int count        READ count)

 public:
  //! create stack
  CQTileStackedWidget(CQTileWindowArea *area);

  //! add child widget
  int addWidget(QWidget *w);

  //! add child widget at position
  int insertWidget(int index, QWidget *w);

  //! remove widget
  void removeWidget(QWidget *w);

  //! get current widget
  QWidget *currentWidget() const;

  //! get current widget index
  int currentIndex() const;

  //! get index of widget
  int indexOf(QWidget *) const;

  //! get widget at index
  QWidget *widget(int index) const;

  //! get number of widgets
  int count() const;

  //! size hint
  QSize sizeHint() const override;

  //! minimum size hint
  QSize minimumSizeHint() const override;

 private:
  //! handle show
  void showEvent(QShowEvent *) override;

  //! handle resize
  void resizeEvent(QResizeEvent *) override;

  //! update layout
  void updateLayout();

 public slots:
  //! set current index
  void setCurrentIndex(int index);

  //! set current widget
  void setCurrentWidget(QWidget *w);

 signals:
  //! notify current index changed
  void currentChanged(int index);

  //! notify widget removed
  void widgetRemoved(int index);

 private:
  typedef std::vector<QWidget *> Widgets;

  CQTileWindowArea *area_;         //! parent area
  int               currentIndex_; //! current index
  Widgets           widgets_;      //! child widgets
};

#endif
