#include <CQTileWindow.h>
#include <CQTileWindowArea.h>

#include <QVBoxLayout>
#include <QApplication>
#include <QCloseEvent>
#include <QIcon>

// create window
CQTileWindow::
CQTileWindow(CQTileWindowArea *area) :
 area_(area), w_(nullptr), valid_(false)
{
  setObjectName("window");

  setAutoFillBackground(true);

  auto *layout = new QVBoxLayout(this);
  layout->setMargin(0); layout->setSpacing(0);

  // monitor focus changed
  // TODO: use single focus change monitor
  connect(qApp, SIGNAL(focusChanged(QWidget*,QWidget*)),
          this, SLOT(focusChangedSlot(QWidget*,QWidget*)));
}

CQTileWindow::
~CQTileWindow()
{
}

// set window widget (view)
void
CQTileWindow::
setWidget(QWidget *w)
{
  if (w_)
    disconnect(w_, SIGNAL(destroyed(QObject *)), this, SLOT(widgetDestroyed()));

  w_     = w;
  valid_ = true;

  if (w_)
    connect(w_, SIGNAL(destroyed(QObject *)), this, SLOT(widgetDestroyed()));

  layout()->addWidget(w);
}

// set window parent area
void
CQTileWindow::
setArea(CQTileWindowArea *area)
{
  area_ = area;
}

// get window title from widget (view)
QString
CQTileWindow::
getTitle() const
{
  return (w_ && valid_ ? w_->windowTitle() : QString(""));
}

// get window icon from widget (view)
QIcon
CQTileWindow::
getIcon() const
{
  return (w_ && valid_ ? w_->windowIcon() : QIcon());
}

// if focus changed to child then make this the current window
void
CQTileWindow::
focusChangedSlot(QWidget * /*old*/, QWidget *now)
{
  if (now && (now == this || isAncestorOf(now)))
    area_->area()->setCurrentWindow(this);
}

void
CQTileWindow::
widgetDestroyed()
{
  valid_ = false;

  area_->area()->removeWindow(this);
}

// update cursor
bool
CQTileWindow::
event(QEvent *e)
{
  if      (e->type() == QEvent::WindowTitleChange) {
    if (w_ && valid_) w_->setWindowTitle(windowTitle());
  }
  else if (e->type() == QEvent::WindowIconChange) {
    if (w_ && valid_) w_->setWindowIcon(windowIcon());
  }

  return QWidget::event(e);
}

void
CQTileWindow::
closeEvent(QCloseEvent *closeEvent)
{
  bool acceptClose = true;

  if (w_)
    acceptClose = w_->close();

  if (! acceptClose) {
    closeEvent->ignore();
    return;
  }

  if (parentWidget() && testAttribute(Qt::WA_DeleteOnClose)) {
    QChildEvent childRemoved(QEvent::ChildRemoved, this);

    QApplication::sendEvent(parentWidget(), &childRemoved);
  }

  closeEvent->accept();
}
