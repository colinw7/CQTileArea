#include <CQTileWindowArea.h>
#include <CQTileWindowTitle.h>
#include <CQTileStackedWidget.h>
#include <CQTileWindowTabBar.h>
#include <CQTileWindow.h>
#include <CQTileAreaConstants.h>

#include <CQWidgetResizer.h>

#include <QApplication>
#include <QVBoxLayout>
#include <QVariant>
#include <QTimer>
#include <QMenu>
#include <QScreen>

#include <images/detach.xpm>
#include <images/attach.xpm>
#include <images/maximize.xpm>
#include <images/restore.xpm>
#include <images/tile.xpm>
#include <images/close.xpm>

int CQTileWindowArea::lastId_;

// create area
CQTileWindowArea::
CQTileWindowArea(CQTileArea *area) :
 area_(area), detached_(false), floating_(false)
{
  setObjectName("area");

  setCursor(Qt::ArrowCursor);

  // assign unique id to area
  id_ = ++lastId_;

  // add frame
  setFrameStyle(uint(QFrame::Panel) | uint(QFrame::Raised));
  setLineWidth(1);

  auto *layout = new QVBoxLayout(this);
  layout->setMargin(0); layout->setSpacing(0);

  // create titlebar, stacked widget (for views) and tabbar to switch between views
  title_  = new CQTileWindowTitle(this);
  stack_  = new CQTileStackedWidget(this);
  tabBar_ = new CQTileWindowTabBar(this);

  layout->addWidget(title_);
  layout->addWidget(stack_);
  layout->addWidget(tabBar_);

  connect(tabBar_, SIGNAL(currentChanged(int)), this, SLOT(tabChangedSlot(int)));

  // create timer for attach animation
  attachData_.timer = new QTimer(this);

  attachData_.timer->setSingleShot(true);

  connect(attachData_.timer, SIGNAL(timeout()), this, SLOT(attachPreviewSlot()));

  resizer_ = new CQWidgetResizer(this);

  resizer_->setMovingEnabled(false);
  resizer_->setActive(false);
}

// destroy area
CQTileWindowArea::
~CQTileWindowArea()
{
  delete resizer_;
}

// add widget (view) to area
CQTileWindow *
CQTileWindowArea::
addWidget(QWidget *w)
{
  auto *window = new CQTileWindow(this);

  window->setWidget(w);

  addWindow(window);

  return window;
}

// add window to area
void
CQTileWindowArea::
addWindow(CQTileWindow *window)
{
  window->setArea(this);

  stack_->addWidget(window);

  windows_.push_back(window);

  int ind = tabBar_->addTab(window->getIcon(), window->getTitle());

  tabBar_->setTabData(ind, window->widget()->objectName());

  // show tabbar if more than one window
  tabBar_->setVisible(tabBar_->count() > 1);
}

// remove window from area
bool
CQTileWindowArea::
removeWindow(CQTileWindow *window)
{
  window->setArea(nullptr);

  int ind = -1;

  uint nw = uint(windows_.size());

  for (uint i = 0; i < nw; ++i) {
    if (ind < 0)
      ind = (windows_[i] == window ? int(i) : -1);
    else
      windows_[i - 1] = windows_[i];
  }

  if (ind >= 0)
    windows_.pop_back();

  stack_->removeWidget(window);

  for (int i = 0; i < tabBar_->count(); ++i) {
    if (tabBar_->tabData(i).toString() == window->widget()->objectName()) {
      tabBar_->removeTab(i);
      break;
    }
  }

  return windows_.empty();
}

// is currently maximized
bool
CQTileWindowArea::
isMaximized() const
{
  return area_->isMaximized();
}

// get title for area
QString
CQTileWindowArea::
getTitle() const
{
  auto *window = currentWindow();

  return (window ? window->getTitle() : QString(""));
}

// get icon for area
QIcon
CQTileWindowArea::
getIcon() const
{
  auto *window = currentWindow();

  return (window ? window->getIcon() : QIcon());
}

// detach window from area
void
CQTileWindowArea::
detachSlot()
{
  if (! isDocked())
    return;

  detach(QPoint(), false, false);
}

// detach window from area with mouse position (if any)
void
CQTileWindowArea::
detach(const QPoint &pos, bool floating, bool dragAll)
{
  // remove window area from grid and display as floating window
  assert(isDocked());

  //----

  setDetached(! floating);
  setFloating(floating);

  //-----

  auto lpos = (! pos.isNull() ? mapFromGlobal(pos) : pos);

  // remove window from window area
  if (! dragAll && tabBar_->count() > 1) {
    // create new window area for non-current tabs
    auto *windowArea = area_->addArea();

    //----

    // get current window and add all other windows to new area
    auto *currentWindow = this->currentWindow();

    for (uint i = 0; i < windows_.size(); ++i)
      if (windows_[i] != currentWindow)
        windowArea->addWindow(windows_[i]);

    // remove all windows, stacked widgets and tabs
    windows_.clear();

    int numStack = stack_->count();

    for (int i = 0; i < numStack; ++i)
      stack_ ->removeWidget(stack_->widget(0));

    int numTabs = tabBar_->count();

    for (int i = 0; i < numTabs; ++i)
      tabBar_->removeTab(0);

    // restore current window
    addWindow(currentWindow);

    //--

    // detach
    if (floating)
      setParent(area_, CQTileAreaConstants::floatingFlags);
    else
      setParent(area_, CQTileAreaConstants::detachedFlags);

    if (! pos.isNull())
      move(pos - lpos);
    else {
      int detachPos = getDetachPos(width(), height());

      move(detachPos, detachPos);
    }

    show();

    area_->replaceWindowArea(this, windowArea);
  }
  // detach whole window area
  else {
    if (floating)
      setParent(area_, CQTileAreaConstants::floatingFlags);
    else
      setParent(area_, CQTileAreaConstants::detachedFlags);

    if (! pos.isNull())
      move(pos - lpos);
    else {
      int detachPos = getDetachPos(width(), height());

      move(detachPos, detachPos);
    }

    show();

    area_->detachWindowArea(this);

    area_->updateCurrentWindow();
  }
}

// attach window to area
void
CQTileWindowArea::
attachSlot()
{
  if (isDocked())
    return;

  attach(false);
}

// get position of detached area
int
CQTileWindowArea::
getDetachPos(int w, int h) const
{
  static int detachPos = 16;

#if 0
  const auto &screenRect = QApplication::desktop()->availableGeometry();
#else
  auto screenRect = QApplication::screens().at(0)->availableGeometry();
#endif

  if (detachPos + w >= screenRect.right () ||
      detachPos + h >= screenRect.bottom())
    detachPos = 16;

  int pos = detachPos;

  detachPos += 16;

  return pos;
}

// initialize detach/attach animation data
void
CQTileWindowArea::
initAttach()
{
  // save initially docked and placement state
  attachData_.initDocked = isDocked();

  if (attachData_.initDocked)
    area()->saveState(attachData_.initState);
}

// cancel attach (restore to original placement)
void
CQTileWindowArea::
cancelAttach()
{
  if (attachData_.initDocked) {
    area()->restoreState(attachData_.initState);

    setParent(area_, CQTileAreaConstants::normalFlags);

    show();

    setDetached(false);
    setFloating(false);

    area_->updateTitles();
  }

  // hide rubber band and stop timer
  area()->hideRubberBand();

  attachData_.timer->stop();
}

// start animation preview (after window detached)
void
CQTileWindowArea::
startAttachPreview()
{
  stopAttachPreview();

  // save original state (but mark invalid so we don't reset unless we need to)
  area()->saveState(attachData_.state);

  attachData_.state.valid_ = false;

  // no drop point set
  attachData_.rect = QRect();
  attachData_.side = CQTileArea::NO_SIDE;
  attachData_.row1 = 0;
  attachData_.col1 = 0;
  attachData_.row2 = 0;
  attachData_.col2 = 0;
}

// stop animation preview
void
CQTileWindowArea::
stopAttachPreview()
{
  // restore to original detached state
  if (attachData_.state.valid_) {
    area()->restoreState(attachData_.state);

    attachData_.state.valid_ = false;
  }

  // hide rubber band and stop timer
  area()->hideRubberBand();

  attachData_.timer->stop();
}

// perform detach/attach (delayed)
void
CQTileWindowArea::
doAttachPreview()
{
  attachData_.timer->start(CQTileAreaConstants::attach_timeout);
}

// perform detach/attach
void
CQTileWindowArea::
attachPreviewSlot()
{
  // if inside current highlight ignore
  auto pos = QCursor::pos();

  if (! attachData_.rect.isNull() && attachData_.rect.contains(pos))
    return;

  // restore placement
  attachData_.state.valid_ = true;

  area()->restoreState(attachData_.state);

  // set highlight to current mouse position
  area()->setHighlight(pos);

  // if we have found a drop point use it
  CQTileArea::Side side;
  int              row1, col1, row2, col2;

  if (area()->getHighlightPos(side, row1, col1, row2, col2)) {
    // preview attached
    attach(side, row1, col1, row2, col2, true);

    auto rect = area()->updateRubberBand();

    // update state
    attachData_.rect = rect;
    attachData_.side = side;
    attachData_.row1 = row1;
    attachData_.col1 = col1;
    attachData_.row2 = row2;
    attachData_.col2 = col2;
  }
  else {
    // keep detached
    auto rect = area()->updateRubberBand();

    // update state
    attachData_.rect = rect;
    attachData_.side = CQTileArea::NO_SIDE;
  }
}

// attach window to area at highlight position
void
CQTileWindowArea::
attach(bool preview)
{
  CQTileArea::Side side;
  int              row1, col1, row2, col2;

  bool rc = area_->getHighlightPos(side, row1, col1, row2, col2);

  if (! rc) {
    // TODO: where to reattach if detached via menu
    side = CQTileArea::NO_SIDE;
    row1 = 0; col1 = 0;
    row2 = 0; col2 = 0;
  }

  attach(side, row1, col1, row2, col2, preview);
}

// attach window to area at specified position
void
CQTileWindowArea::
attach(CQTileArea::Side side, int row1, int col1, int row2, int col2, bool preview)
{
  if (! preview) {
    setParent(area_, CQTileAreaConstants::normalFlags);

    show();
  }

  //---

  area_->setDefPlacementSize(stack_->width(), stack_->height());

  //---

  auto *attachArea = (! preview ? this : nullptr);

  // add rows for top/bottom and add at specified row and column range
  if      (side == CQTileArea::TOP_SIDE || side == CQTileArea::BOTTOM_SIDE) {
    area_->insertRows(row1, 1);

    area_->addWindowArea(attachArea, row1, col1, 1, col2 - col1);
  }
  // add columns for left/right and add at specified column and row range
  else if (side == CQTileArea::LEFT_SIDE || side == CQTileArea::RIGHT_SIDE) {
    area_->insertColumns(col1, 1);

    area_->addWindowArea(attachArea, row1, col1, row2 - row1, 1);
  }
  // add to existing area (only needs to update placement on actual (non-preview) action
  else if (side == CQTileArea::MIDDLE_SIDE) {
    if (! preview) {
      // get area to drop on
      auto *area = area_->getAreaAt(row1, col1);

      if (! area) { // assert ?
        std::cerr << "no area at " << row1 << " " << col1 << std::endl;
        return;
      }

      // determine tab index of last new window
      int ind = tabBar_->currentIndex() + area->tabBar_->count();

      // add all windows to drop area
      for (Windows::iterator p = windows_.begin(); p != windows_.end(); ++p)
        area->addWindow(*p);

      // set tab
      area->tabBar_->setCurrentIndex(ind);

      // delete this area
      area_->removeArea(this);

      deleteLater();

      // update current area to drop area
      area_->setCurrentArea(area);
    }
  }
  // add to best position
  else {
    int row, col, nrows, ncols;

    area_->calcBestWindowArea(row, col, nrows, ncols);

    area_->addWindowArea(attachArea, row, col, nrows, ncols);
  }

  area_->setDefPlacementSize(-1, -1);

  // update state
  if (! preview) {
    setDetached(false);
    setFloating(false);
  }
}

// reattach window (floating) to area (docked)
void
CQTileWindowArea::
reattach()
{
  setParent(area_, CQTileAreaConstants::normalFlags);

  show();
}

// make floating (detach)
void
CQTileWindowArea::
setFloated()
{
  // set as standalone window
  if (! isDetached()) {
    setParent(area_, CQTileAreaConstants::detachedFlags);

    show();

    setDetached(true);
    setFloating(false);
  }
}

// update detached state
void
CQTileWindowArea::
setDetached(bool detached)
{
  if (detached_ == detached)
    return;

  detached_ = detached;

  resizer_->setActive(detached_);

  updateTitle();
}

// update floating state
void
CQTileWindowArea::
setFloating(bool floating)
{
  if (floating_ == floating)
    return;

  floating_ = floating;

  updateTitle();
}

// update title bar
void
CQTileWindowArea::
updateTitle()
{
  title_->updateState();

  // if floating ensure we don't change geometry or it will screw up the drag
  if (isFloating()) return;

  bool titleVisible = true;

  if (isDocked() && area_->isMaximized())
    titleVisible = false;

  title_->setVisible(titleVisible);

  if (area_->isFullScreen())
    setFrameStyle(uint(QFrame::NoFrame) | uint(QFrame::Plain));
  else
    setFrameStyle(uint(QFrame::Panel) | uint(QFrame::Raised));
}

// get current window
CQTileWindow *
CQTileWindowArea::
currentWindow() const
{
  auto *w = stack_->widget(stack_->currentIndex());

  return qobject_cast<CQTileWindow *>(w);
}

// check if window is in this area
bool
CQTileWindowArea::
hasWindow(CQTileWindow *window) const
{
  for (Windows::const_iterator p = windows_.begin(); p != windows_.end(); ++p) {
    if (*p == window)
      return true;
  }

  return false;
}

// set current window
void
CQTileWindowArea::
setCurrentWindow(CQTileWindow *window)
{
  stack_->setCurrentWidget(window);

  for (int i = 0; i < tabBar_->count(); ++i) {
    if (tabBar_->tabData(i).toString() == window->widget()->objectName()) {
      tabBar_->setCurrentIndex(i);
      break;
    }
  }
}

// called when tab changed
void
CQTileWindowArea::
tabChangedSlot(int tabNum)
{
  // set stacked widget index from variant data
  auto name = tabBar_->tabData(tabNum).toString();

  for (Windows::const_iterator p = windows_.begin(); p != windows_.end(); ++p) {
    if (name == (*p)->widget()->objectName()) {
      stack_->setCurrentWidget(*p);
      break;
    }
  }

  // update title
  title_->update();

  // notify window changed
  area()->emitCurrentWindowChanged();
}

// tile windows
void
CQTileWindowArea::
tileSlot()
{
  auto *area = this->area();

  auto *window = currentWindow();

  area->tileSlot();

  area->setCurrentWindow(window);
}

// maximize windows
void
CQTileWindowArea::
maximizeSlot()
{
  auto *area = this->area();

  auto *window = currentWindow();

  area->maximizeSlot();

  area->setCurrentWindow(window);
}

// restore windows
void
CQTileWindowArea::
restoreSlot()
{
  auto *area = this->area();

  auto *window = currentWindow();

  area->restoreSlot();

  area->setCurrentWindow(window);
}

// close window
void
CQTileWindowArea::
closeSlot()
{
  // close current window
  auto *window = currentWindow();

  if (window)
    area_->removeWindow(window);
}

// create context menu (for titlebar or tabbar)
QMenu *
CQTileWindowArea::
createContextMenu(QWidget *parent) const
{
  auto *menu = new QMenu(parent);

  auto *detachAction   = menu->addAction(QPixmap(detach_data  ), "Detach");
  auto *attachAction   = menu->addAction(QPixmap(attach_data  ), "Attach");
  auto *maximizeAction = menu->addAction(QPixmap(maximize_data), "Maximize");
  auto *restoreAction  = menu->addAction(QPixmap(restore_data ), "Restore");
  auto *tileAction     = menu->addAction(QPixmap(tile_data    ), "Tile");
  (void)                 menu->addSeparator();
  auto *closeAction    = menu->addAction(QPixmap(close_data   ), "Close");

  connect(detachAction  , SIGNAL(triggered()), this, SLOT(detachSlot()));
  connect(attachAction  , SIGNAL(triggered()), this, SLOT(attachSlot()));
  connect(maximizeAction, SIGNAL(triggered()), this, SLOT(maximizeSlot()));
  connect(restoreAction , SIGNAL(triggered()), this, SLOT(restoreSlot()));
  connect(tileAction    , SIGNAL(triggered()), this, SLOT(tileSlot()));
  connect(closeAction   , SIGNAL(triggered()), this, SLOT(closeSlot()));

  return menu;
}

// update menu to show only relevant items
void
CQTileWindowArea::
updateContextMenu(QMenu *menu) const
{
  auto actions = menu->actions();

  for (int i = 0; i < actions.size(); ++i) {
    auto *action = actions.at(i);

    const auto &text = action->text();

    if      (text == "Detach")
      action->setVisible(  isDocked());
    else if (text == "Attach")
      action->setVisible(! isDocked());
    else if (text == "Maximize")
      action->setVisible(! isMaximized());
    else if (text == "Restore") {
      action->setVisible(  isMaximized());
      action->setEnabled(area_->isRestoreStateValid());
    }
  }
}

QSize
CQTileWindowArea::
sizeHint() const
{
  int w = 4;
  int h = 4;

  h += title_->minimumSizeHint().height();

  w  = std::max(w, stack_->sizeHint().width());
  h += stack_->sizeHint().height();

  if (tabBar_->count() > 1) {
    w  = std::max(w, tabBar_->sizeHint().width());
    h += tabBar_->sizeHint().height();
  }

  return QSize(w, h);
}

QSize
CQTileWindowArea::
minimumSizeHint() const
{
  int w = 4;
  int h = 4;

  h += title_->minimumSizeHint().height();

  w  = std::max(w, stack_->minimumSizeHint().width());
  h += stack_->minimumSizeHint().height();

  if (tabBar_->count() > 1) {
    w  = std::max(w, tabBar_->minimumSizeHint().width());
    h += tabBar_->minimumSizeHint().height();
  }

  return QSize(w, h);
}
