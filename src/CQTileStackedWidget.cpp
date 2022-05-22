#include <CQTileStackedWidget.h>
#include <CQTileArea.h>
#include <CQTileWindowArea.h>

#include <QApplication>

namespace CQWidgetUtil {
  QSize SmartMinSize(const QSize &sizeHint, const QSize &minSizeHint,
                     const QSize &minSize, const QSize &maxSize,
                     const QSizePolicy &sizePolicy) {
    QSize s(0, 0);

    if (sizePolicy.horizontalPolicy() != QSizePolicy::Ignored) {
      if (sizePolicy.horizontalPolicy() & uint(QSizePolicy::ShrinkFlag))
        s.setWidth(minSizeHint.width());
      else
        s.setWidth(qMax(sizeHint.width(), minSizeHint.width()));
    }

    if (sizePolicy.verticalPolicy() != QSizePolicy::Ignored) {
      if (sizePolicy.verticalPolicy() & uint(QSizePolicy::ShrinkFlag)) {
        s.setHeight(minSizeHint.height());
      } else {
        s.setHeight(qMax(sizeHint.height(), minSizeHint.height()));
      }
    }

    s = s.boundedTo(maxSize);

    if (minSize.width () > 0) s.setWidth (minSize.width ());
    if (minSize.height() > 0) s.setHeight(minSize.height());

    return s.expandedTo(QSize(0, 0));
  }

  QSize SmartMinSize(const QWidget *w) {
    return SmartMinSize(w->sizeHint(), w->minimumSizeHint(),
                        w->minimumSize(), w->maximumSize(),
                        w->sizePolicy());
  }
}

//---

CQTileStackedWidget::
CQTileStackedWidget(CQTileWindowArea *area) :
 QWidget(area), area_(area), currentIndex_(-1)
{
  setObjectName("stack");
}

int
CQTileStackedWidget::
addWidget(QWidget *widget)
{
  return insertWidget(count(), widget);
}

int
CQTileStackedWidget::
insertWidget(int index, QWidget *widget)
{
  if (index < 0 || index > count())
    index = count();

  auto *current = currentWidget();

  widget->setParent(this);

  widget->setVisible(true);

  widgets_.push_back(nullptr);

  for (int i = count() - 1; i > index && i > 0; --i)
    widgets_[uint(i)] = widgets_[uint(i - 1)];

  widgets_[uint(index)] = widget;

  if (currentIndex() == -1)
    setCurrentIndex(0);
  else
    setCurrentIndex(indexOf(current));

  return index;
}

void
CQTileStackedWidget::
removeWidget(QWidget *widget)
{
  auto *current = currentWidget();

  if (current == widget)
    current = nullptr;

  int ind = indexOf(widget);

  if (ind == -1)
    return;

  for (int i = ind; i < count() - 1; ++i)
    widgets_[uint(i)] = widgets_[uint(i + 1)];

  widgets_.pop_back();

  setCurrentIndex(indexOf(current));

  emit widgetRemoved(ind);
}

QWidget *
CQTileStackedWidget::
currentWidget() const
{
  if (currentIndex() >= 0 && currentIndex() < count())
    return widgets_[uint(currentIndex())];
  else
    return nullptr;
}

int
CQTileStackedWidget::
currentIndex() const
{
  return currentIndex_;
}

int
CQTileStackedWidget::
indexOf(QWidget *widget) const
{
  if (! widget) return -1;

  for (int i = 0; i < count(); ++i)
    if (widgets_[uint(i)] == widget)
      return i;

  return -1;
}

QWidget *
CQTileStackedWidget::
widget(int index) const
{
  if (index >= 0 && index < count())
    return widgets_[uint(index)];

  return nullptr;
}

int
CQTileStackedWidget::
count() const
{
  return int(widgets_.size());
}

void
CQTileStackedWidget::
setCurrentWidget(QWidget *widget)
{
  int ind = indexOf(widget);

  if (ind == -1)
    return;

  setCurrentIndex(ind);
}

void
CQTileStackedWidget::
setCurrentIndex(int index)
{
  if (index < -1 || index >= count())
    return;

  if (index == currentIndex())
    return;

  currentIndex_ = index;

  updateLayout();

  emit currentChanged(currentIndex());
}

void
CQTileStackedWidget::
showEvent(QShowEvent *)
{
  updateLayout();
}

void
CQTileStackedWidget::
resizeEvent(QResizeEvent *)
{
  updateLayout();
}

void
CQTileStackedWidget::
updateLayout()
{
  auto *current = currentWidget();

  if (current) {
    current->move(0, 0);
    current->resize(width(), height());

    current->setVisible(true);
    current->raise();
  }
}

QSize
CQTileStackedWidget::
sizeHint() const
{
  auto s = QApplication::globalStrut();

  for (int i = 0; i < count(); ++i) {
    auto *w = widgets_[uint(i)];

    if (w) {
      auto s1 = w->sizeHint();

      if (w->sizePolicy().horizontalPolicy() == QSizePolicy::Ignored) s1.setWidth (0);
      if (w->sizePolicy().verticalPolicy  () == QSizePolicy::Ignored) s1.setHeight(0);

      s = s.expandedTo(s1);
    }
  }

  return s;
}

QSize
CQTileStackedWidget::
minimumSizeHint() const
{
  auto s = QApplication::globalStrut();

  for (int i = 0; i < count(); ++i) {
    auto *w = widgets_[uint(i)];

    if (w) {
      auto s1 = CQWidgetUtil::SmartMinSize(w);

      s = s.expandedTo(s1);
    }
  }

  return s;
}
