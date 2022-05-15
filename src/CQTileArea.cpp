#include <CQTileArea.h>

#include <CQWidgetResizer.h>
#include <CQRubberBand.h>

#include <QMainWindow>
#include <QVBoxLayout>
#include <QStylePainter>
#include <QStyleOption>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QMenuBar>
#include <QMenu>
#include <QScreen>
#include <QTimer>

#include <cassert>
#include <set>
#include <iostream>
#include <cmath>

#include <images/detach.xpm>
#include <images/attach.xpm>
#include <images/maximize.xpm>
#include <images/restore.xpm>
#include <images/tile.xpm>
#include <images/close.xpm>

namespace CQTileAreaConstants {
  bool            debug_grid      = false;
  int             min_size        = 16;
  int             highlight_size  = 16;
  int             attach_timeout  = 10;
  QColor          bar_active_fg   = QColor(140,140,140);
  QColor          bar_inactive_fg = QColor(120,120,120);
  Qt::WindowFlags normalFlags     = Qt::Widget;
  Qt::WindowFlags floatingFlags   = Qt::Tool | Qt::FramelessWindowHint |
                                    Qt::X11BypassWindowManagerHint;
  Qt::WindowFlags detachedFlags   = Qt::Tool | Qt::FramelessWindowHint;
}

// create tile area
CQTileArea::
CQTileArea(QMainWindow *window) :
 window_(window), animateDrag_(true), border_(0), splitterSize_(3), currentArea_(nullptr),
 hasControls_(false), menuIcon_(), menuControls_(), defWidth_(-1), defHeight_(-1)
{
  setObjectName("tileArea");

  // title bar active/inactive colors
  titleActiveColor_   = QColor(190,190,190);
  titleInactiveColor_ = QColor(160,160,160);

  // track mouse move for splitters (TODO: widgets)
  setMouseTracking(true);

  // create global rubber band for highlight
  rubberBand_ = new CQRubberBand();

  rubberBand_->hide();
}

// destroy tile area
CQTileArea::
~CQTileArea()
{
  delete rubberBand_;
}

// set title bar active color
void
CQTileArea::
setTitleActiveColor(const QColor &color)
{
  titleActiveColor_ = color;
}

// set title bar inactive color
void
CQTileArea::
setTitleInactiveColor(const QColor &color)
{
  titleInactiveColor_ = color;
}

// add window as a new column in the grid
CQTileWindow *
CQTileArea::
addWindow(QWidget *w)
{
  int row, col, nrows, ncols;

  calcBestWindowArea(row, col, nrows, ncols);

  return addWindow(w, row, col, nrows, ncols);
}

// add window at specified row and column using specified number of rows and columns
CQTileWindow *
CQTileArea::
addWindow(QWidget *w, int row, int col, int nrows, int ncols)
{
  // create new area
  auto *windowArea = addArea();

  //------

  // add widget to window area
  auto *window = windowArea->addWidget(w);

  addWindowArea(windowArea, row, col, nrows, ncols);

  //------

  return window;
}

// add new window area
CQTileWindowArea *
CQTileArea::
addArea()
{
  auto *windowArea = new CQTileWindowArea(this);

  areas_[windowArea->id()] = windowArea;

  setCurrentArea(windowArea);

  return windowArea;
}

// add new window area at specified location and size
void
CQTileArea::
addWindowArea(CQTileWindowArea *windowArea, int row, int col, int nrows, int ncols)
{
  // insert top
  if      (row < 0) {
    insertRows(0, -row);
    row = 0;
  }
  // insert bottom
  else if (row >= grid_.nrows())
    insertRows(grid_.nrows(), row - grid_.nrows() + 1);

  // insert left
  if      (col < 0) {
    insertColumns(0, -col);
    col = 0;
  }
  // insert right
  else if (col >= grid_.ncols())
    insertColumns(grid_.ncols(), col - grid_.ncols() + 1);

  //------

  int row1 = row + nrows - 1;
  int col1 = col + ncols - 1;

  //------

  // check for overlap
  bool overlap = false;

  for (int r = row; ! overlap && r <= row1; ++r) {
    for (int c = col; ! overlap && c <= col1; ++c) {
      if (grid_.cell(r, c) < 0) continue;

      overlap = true;

      break;
    }
  }

  if (overlap) {
    if (nrows > ncols) {
      int splitCol = col;

      for ( ; splitCol > 0 && splitCol < grid_.ncols(); ++splitCol) {
        bool valid = true;

        for (int r = 0; r < grid_.nrows(); ++r) {
          if (grid_.cell(r, splitCol - 1) == grid_.cell(r, splitCol)) {
            valid = false;
            break;
          }
        }

        if (valid)
          break;
      }

      col  = splitCol;
      col1 = col + ncols - 1;

      insertColumns(col, ncols);
    }
    else {
      int splitRow = row;

      for ( ; splitRow > 0 && splitRow < grid_.nrows(); ++splitRow) {
        bool valid = true;

        for (int c = 0; c < grid_.ncols(); ++c) {
          if (grid_.cell(splitRow - 1, c) == grid_.cell(splitRow, c)) {
            valid = false;
            break;
          }
        }

        if (valid)
          break;
      }

      row  = splitRow;
      row1 = row + nrows - 1;

      insertRows(row, nrows);
    }
  }

  //------

  // store area in grid
  int fillId = (windowArea ? windowArea->id() : 0);

  grid_.fill(row, col, row1, col1, fillId);

  //------

  if (isVisible())
    updatePlacement();
}

// remove window area
void
CQTileArea::
removeArea(CQTileWindowArea *area)
{
  // remove area
  areas_.erase(area->id());

  // remove from grid
  grid_.replace(area->id(), -1);

  // remove from placement
  uint np = uint(placementAreas_.size());

  for (uint i = 0; i < np; ++i) {
    PlacementArea &placementArea = placementAreas_[i];

    if (placementArea.areaId == area->id())
      placementArea.areaId = 0;
  }

  // if current area then set new current
  if (currentArea_ == area) {
    if (! areas_.empty())
      setCurrentArea((*areas_.begin()).second);
    else
      setCurrentArea(nullptr);
  }
}

// remove window
void
CQTileArea::
removeWindow(CQTileWindow *window)
{
  // get area
  auto *area = getWindowArea(window);
  if (! area) return;

  // remove window from area and check if area now empty (if so delete it)
  bool empty = area->removeWindow(window);

  if (empty) {
    removeArea(area);

    area->deleteLater();
  }

  // reparent child widget so it is not deleted with the window
  if (window->isValid()) {
    auto *w = window->widget();

    w->setParent(this);
    w->hide();
  }

  if (isVisible())
    updatePlacement();

  // notify close
  emit windowClosed(window);

  // delete window
  window->deleteLater();

  // invalidate restore
  restoreState_.valid_ = false;

  // update titles
  updateTitles();
}

// calc best position for new area
void
CQTileArea::
calcBestWindowArea(int &row, int &col, int &nrows, int &ncols)
{
  // if bottom right cell spans multiple rows or columns the steal rows/columns
  bool steal = false;

  if (grid_.ncols() > 1 && grid_.nrows() > 1) {
    if      (grid_.cell(grid_.nrows() - 1, grid_.ncols() - 1) ==
             grid_.cell(grid_.nrows() - 1, grid_.ncols() - 2)) {
      steal = true;

      row   = grid_.nrows() - 1;
      col   = grid_.ncols() - 1;
      nrows = 1;
      ncols = 1;

      while (col > 1 && grid_.cell(grid_.nrows() - 1, col - 1) ==
                        grid_.cell(grid_.nrows() - 1, col - 2)) {
        --col; ++ncols;
      }

      for (int i = 0; i < ncols; ++i)
        grid_.cell(row, col + i) = -1;
    }
    else if (grid_.cell(grid_.nrows() - 1, grid_.ncols() - 1) ==
             grid_.cell(grid_.nrows() - 2, grid_.ncols() - 1)) {
      steal = true;

      row   = grid_.nrows() - 1;
      col   = grid_.ncols() - 1;
      nrows = 1;
      ncols = 1;

      while (row > 1 && grid_.cell(row - 1, grid_.ncols() - 1) ==
                        grid_.cell(row - 2, grid_.ncols() - 1)) {
        --row; ++nrows;
      }

      for (int i = 0; i < nrows; ++i)
        grid_.cell(row + i, col) = -1;
    }
  }

  //------

  // fill based on the smallest number of rows or columns, preference to new columns
  if (! steal) {
    // Smallest Columns: new grid position is next column with a height of all rows
    // (ensure at least one row if table empty)
    if (grid_.ncols() <= grid_.nrows()) {
      row   = 0;
      col   = grid_.ncols();
      nrows = std::max(grid_.nrows(), 1);
      ncols = 1;
    }
    // Smallest Rows: new grid position is next row with a width of all columns
    // (ensure at least one column if table empty)
    else {
      row   = grid_.nrows();
      col   = 0;
      nrows = 1;
      ncols = std::max(grid_.ncols(), 1);
    }
  }
}

// add new rows after the specified row
void
CQTileArea::
insertRows(int row, int nrows)
{
  grid_.insertRows(row, nrows);
}

// add new columns after specified column
void
CQTileArea::
insertColumns(int col, int ncols)
{
  grid_.insertColumns(col, ncols);
}

// detach window from placement
void
CQTileArea::
detachWindowArea(CQTileWindowArea *window)
{
  // reset cells for this window area to zero
  grid_.replace(window->id(), -1);

  // update placement
  if (isVisible())
    updatePlacement();
}

// replace window area with new window area
void
CQTileArea::
replaceWindowArea(CQTileWindowArea *oldArea, CQTileWindowArea *newArea)
{
  // reset cells of old area to new area
  grid_.replace(oldArea->id(), newArea->id());

  PlacementArea &placementArea = getPlacementAreaForArea(oldArea);

  placementArea.areaId = newArea->id();

  updatePlacementGeometry(placementArea);
}

// set grid from cell list (for debug)
void
CQTileArea::
setGrid(int nrows, int ncols, const std::vector<int> &cells)
{
  grid_.setSize(nrows, ncols);

  for (int i = 0; i < nrows*ncols; ++i) {
    int r = i / ncols;
    int c = i % ncols;

    grid_.cell(r, c) = cells[uint(i)];
  }

  updatePlacement();
}

// update physical placement from logical cell placement
void
CQTileArea::
updatePlacement(bool useExisting)
{
  // remove empty cells and cleanup duplicate rows and columns
  fillEmptyCells();

  removeDuplicateCells();

  //------

  // update placement
  gridToPlacement(useExisting);

  adjustToFit();

  updatePlacementGeometries();

  //------

  // update titles
  updateTitles();

  //------

  if (CQTileAreaConstants::debug_grid)
    grid_.print(std::cerr);
}

// expand occupied cells to fill empty ones
void
CQTileArea::
fillEmptyCells()
{
  grid_.fillEmptyCells();
}

// remove duplicate rows and columns to compress grid
void
CQTileArea::
removeDuplicateCells()
{
  grid_.removeDuplicateRows();
  grid_.removeDuplicateCols();
}

// convert logical grid to physical placement (including splitters)
void
CQTileArea::
gridToPlacement(bool useExisting)
{
  int ss = splitterSize();

  PlacementAreas placementAreas = placementAreas_;

  placementAreas_.clear();

  CTileGrid grid = grid_;

  for (int r = 0; r < grid.nrows(); ++r) {
    for (int c = 0; c < grid.ncols(); ++c) {
      int id = grid.cell(r, c);

      if (id < -1)
        continue;

      // get extent (nrow,ncols) of area with specified id
      int nr, nc;

      if (! grid.getRegion(id, r, c, &nr, &nc))
        continue;

      int r2 = r + nr - 1;
      int c2 = c + nc - 1;

      // clear cells with this id
      grid.fill(r, c, r2, c2, -2);

      // get area id
      int areaId = 0;

      if (id > 0) {
        auto *area = getAreaForId(id);

        if (area)
          areaId = area->id();
        else {
          std::cerr << "Invalid Area Id " << id << std::endl;
          //assert(false);
          continue;
        }
      }

      // create placement area for this area
      PlacementArea placementArea;

      placementArea.place(r, c, nr, nc, areaId);

      // use original size if possible
      if (useExisting) {
        int pid = getPlacementAreaIndex(placementAreas, id);

        if (pid >= 0) {
          PlacementArea &placementArea1 = placementAreas[uint(pid)];

          placementArea.width  = placementArea1.width ;
          placementArea.height = placementArea1.height;

          if (placementArea.col2() != grid.ncols()) placementArea.width  += ss/2;
          if (placementArea.col1() != 0           ) placementArea.width  += ss/2;
          if (placementArea.row2() != grid.nrows()) placementArea.height += ss/2;
          if (placementArea.row1() != 0           ) placementArea.height += ss/2;
        }
        else {
          if (defWidth_ > 0 && defHeight_ > 0) {
            placementArea.width  = defWidth_;
            placementArea.height = defHeight_;
          }
          else {
            placementArea.width  = (grid.ncols() > 1 ? width ()/(grid.ncols() - 1) : width ());
            placementArea.height = (grid.nrows() > 1 ? height()/(grid.nrows() - 1) : height());
          }
        }
      }
      else {
        placementArea.width  = (grid.ncols() > 1 ? width ()/(grid.ncols() - 1) : width ());
        placementArea.height = (grid.nrows() > 1 ? height()/(grid.nrows() - 1) : height());
      }

      placementAreas_.push_back(placementArea);
    }
  }

  addSplitters();
}

// add splitters between placement areas
void
CQTileArea::
addSplitters()
{
  // reset splitter widgets
  for (SplitterWidgets::iterator p = splitterWidgets_.begin(); p != splitterWidgets_.end(); ++p) {
    auto *splitter = (*p).second;

    splitter->setUsed(false);
  }

  //---

  hsplitters_.clear();
  vsplitters_.clear();

  uint np = uint(placementAreas_.size());

  for (uint i = 0; i < np; ++i) {
    PlacementArea &placementArea = placementAreas_[i];

    int row1 = placementArea.row1();
    int row2 = placementArea.row2();
    int col1 = placementArea.col1();
    int col2 = placementArea.col2();

    // add splitter for each edge
    if (row1 > 0            ) addHSplitter(int(i), row1, true ); // top
    if (row2 < grid_.nrows()) addHSplitter(int(i), row2, false); // bottom
    if (col1 > 0            ) addVSplitter(int(i), col1, true ); // left
    if (col2 < grid_.ncols()) addVSplitter(int(i), col2, false); // right
  }

  //------

  combineTouchingSplitters();

  //------

  // ensure no intersect of horizontal splitters with vertical ones
  for (RowHSplitterArray::iterator p = hsplitters_.begin(); p != hsplitters_.end(); ++p) {
    int row = (*p).first;

    HSplitterArray &hsplitters = (*p).second;

    uint ns = uint(hsplitters.size());

    for (uint i = 0; i < ns; ++i) {
      HSplitter &hsplitter = hsplitters[i];

      intersectVSplitter(row, hsplitter);
    }
  }

  //------

  for (RowHSplitterArray::iterator p = hsplitters_.begin(); p != hsplitters_.end(); ++p) {
    HSplitterArray &hsplitters = (*p).second;

    for (uint i = 0; i < hsplitters.size(); ++i)
      hsplitters[i].splitterId = createSplitterWidget(Qt::Horizontal, (*p).first, int(i));
  }

  for (ColVSplitterArray::iterator p = vsplitters_.begin(); p != vsplitters_.end(); ++p) {
    VSplitterArray &vsplitters = (*p).second;

    for (uint i = 0; i < vsplitters.size(); ++i)
      vsplitters[i].splitterId = createSplitterWidget(Qt::Vertical, (*p).first, int(i));
  }
}

// combine touching splitters
void
CQTileArea::
combineTouchingSplitters()
{
  // combine touching splitters
  for (int r = 0; r < grid_.nrows(); ++r) {
    RowHSplitterArray::iterator p = hsplitters_.find(r);
    if (p == hsplitters_.end()) continue;

    HSplitterArray &splitters = (*p).second;

    bool combine = true;

    while (combine) {
      combine = false;

      uint ns = uint(splitters.size());

      for (uint i = 0; ! combine && i < ns; ++i) {
        HSplitter &splitter1 = splitters[i];

        for (uint j = i + 1; ! combine && j < ns; ++j) {
          HSplitter &splitter2 = splitters[j];

          if (splitter2.col1 <= splitter1.col2 && splitter2.col2 >= splitter1.col1) {
            splitter1.col1 = std::min(splitter1.col1, splitter2.col1);
            splitter1.col2 = std::max(splitter1.col2, splitter2.col2);

            std::copy(splitter2.tareas.begin(), splitter2.tareas.end(),
                      std::inserter(splitter1.tareas, splitter1.tareas.end()));
            std::copy(splitter2.bareas.begin(), splitter2.bareas.end(),
                      std::inserter(splitter1.bareas, splitter1.bareas.end()));

            for (uint k = j + 1; k < ns; ++k)
              splitters[k - 1] = splitters[k];

            splitters.pop_back();

            combine = true;
          }
        }
      }
    }
  }

  for (int c = 0; c < grid_.ncols(); ++c) {
    ColVSplitterArray::iterator p = vsplitters_.find(c);
    if (p == vsplitters_.end()) continue;

    VSplitterArray &splitters = (*p).second;

    bool combine = true;

    while (combine) {
      combine = false;

      uint ns = uint(splitters.size());

      for (uint i = 0; ! combine && i < ns; ++i) {
        VSplitter &splitter1 = splitters[i];

        for (uint j = i + 1; ! combine && j < ns; ++j) {
          VSplitter &splitter2 = splitters[j];

          if (splitter2.row1 <= splitter1.row2 && splitter2.row2 >= splitter1.row1) {
            splitter1.row1 = std::min(splitter1.row1, splitter2.row1);
            splitter1.row2 = std::max(splitter1.row2, splitter2.row2);

            std::copy(splitter2.lareas.begin(), splitter2.lareas.end(),
                      std::inserter(splitter1.lareas, splitter1.lareas.end()));
            std::copy(splitter2.rareas.begin(), splitter2.rareas.end(),
                      std::inserter(splitter1.rareas, splitter1.rareas.end()));

            for (uint k = j + 1; k < ns; ++k)
              splitters[k - 1] = splitters[k];

            splitters.pop_back();

            combine = true;
          }
        }
      }
    }
  }
}

// update all area geometries from placement data
void
CQTileArea::
updatePlacementGeometries()
{
  uint np = uint(placementAreas_.size());

  for (uint i = 0; i < np; ++i) {
    PlacementArea &placementArea = placementAreas_[i];

    updatePlacementGeometry(placementArea);
  }
}

// update area geometry from placement data
void
CQTileArea::
updatePlacementGeometry(PlacementArea &placementArea)
{
  auto *area = getAreaForId(placementArea.areaId);

  if (area) {
    bool reparent = (area->parentWidget() != this ||
                     (area->windowFlags() & Qt::FramelessWindowHint));

    if (reparent)
      area->setParent(this, CQTileAreaConstants::normalFlags);

    area->move  (placementArea.x1 (), placementArea.y1  ());
    area->resize(placementArea.width, placementArea.height);

    if (reparent)
      area->show();
  }
}

// add horizontal splitter to placement area row
void
CQTileArea::
addHSplitter(int i, int row, bool top)
{
  PlacementArea &placementArea = placementAreas_[uint(i)];

  int ss = splitterSize();

  // adjust size for splitter
  if (top) { placementArea.y      += ss/2; placementArea.height -= ss/2; }
  else     { placementArea.height -= ss/2; }

  // get column range
  int col1 = placementArea.col1();
  int col2 = placementArea.col2();

  // check overlap with existing splitters for this row
  int is;

  if (getHSplitter(row, col1, col2, is)) {
    HSplitter &splitter = hsplitters_[row][uint(is)];

    // handle exact overlap
    if (col1 == splitter.col1 && col2 == splitter.col2) {
      if (top)
        splitter.bareas.insert(i);
      else
        splitter.tareas.insert(i);

      return;
    }

    // handle join
    splitter.col1 = std::min(splitter.col1, col1);
    splitter.col2 = std::max(splitter.col2, col2);

    if (top)
      splitter.bareas.insert(i);
    else
      splitter.tareas.insert(i);

    return;
  }

  HSplitter splitter;

  splitter.col1 = col1;
  splitter.col2 = col2;

  if (top)
    splitter.bareas.insert(i);
  else
    splitter.tareas.insert(i);

  hsplitters_[row].push_back(splitter);
}

// add vertical splitter to placement area column
void
CQTileArea::
addVSplitter(int i, int col, bool left)
{
  PlacementArea &placementArea = placementAreas_[uint(i)];

  int ss = splitterSize();

  // adjust size for splitter
  if (left) { placementArea.x     += ss/2; placementArea.width -= ss/2; }
  else      { placementArea.width -= ss/2; }

  // get row range
  int row1 = placementArea.row1();
  int row2 = placementArea.row2();

  // check overlap with existing splitters for this column
  int is;

  if (getVSplitter(col, row1, row2, is)) {
    VSplitter &splitter = vsplitters_[col][uint(is)];

    // handle exact overlap
    if (row1 == splitter.row1 && row2 == splitter.row2) {
      if (left)
        splitter.rareas.insert(i);
      else
        splitter.lareas.insert(i);

      return;
    }

    // handle join
    splitter.row1 = std::min(splitter.row1, row1);
    splitter.row2 = std::max(splitter.row2, row2);

    if (left)
      splitter.rareas.insert(i);
    else
      splitter.lareas.insert(i);

    return;
  }

  VSplitter splitter;

  splitter.row1 = row1;
  splitter.row2 = row2;

  if (left)
    splitter.rareas.insert(i);
  else
    splitter.lareas.insert(i);

  vsplitters_[col].push_back(splitter);
}

// check for vertical and horizontal splitter intersection
void
CQTileArea::
intersectVSplitter(int row, const HSplitter &hsplitter)
{
  for (ColVSplitterArray::iterator p = vsplitters_.begin(); p != vsplitters_.end(); ++p) {
    std::vector<VSplitter> newVSplitters;

    int col = (*p).first;

    if (col <= hsplitter.col1 || col >= hsplitter.col2) continue;

    VSplitterArray &vsplitters = (*p).second;

    uint ns = uint(vsplitters.size());

    for (uint i = 0; i < ns; ++i) {
      VSplitter &vsplitter = vsplitters[i];

      if (row <= vsplitter.row1 || row >= vsplitter.row2) continue;

      // split
      AreaSet lareas = vsplitter.lareas;
      AreaSet rareas = vsplitter.rareas;

      vsplitter.lareas.clear();
      vsplitter.rareas.clear();

      VSplitter vsplitter1 = vsplitter;

      vsplitter .row2 = row;
      vsplitter1.row1 = row;

      for (AreaSet::iterator pl = lareas.begin(); pl != lareas.end(); ++pl) {
        const PlacementArea &placementArea = placementAreas_[uint(*pl)];

        if (placementArea.row1() < row)
          vsplitter .lareas.insert(*pl);
        else
          vsplitter1.lareas.insert(*pl);
      }

      for (AreaSet::iterator pr = rareas.begin(); pr != rareas.end(); ++pr) {
        const PlacementArea &placementArea = placementAreas_[uint(*pr)];

        if (placementArea.row1() < row)
          vsplitter .rareas.insert(*pr);
        else
          vsplitter1.rareas.insert(*pr);
      }

      newVSplitters.push_back(vsplitter1);
    }

    for (uint i = 0; i < newVSplitters.size(); ++i)
      vsplitters_[col].push_back(newVSplitters[i]);
  }
}

// adjust placement area sizes to fit new larger/smaller widget geometry
void
CQTileArea::
adjustToFit()
{
  uint np = uint(placementAreas_.size());

  std::vector<bool> sized(np);

  int b  = border();
  int ss = splitterSize();

  //------

  // adjust areas to fit width
  int w = width();

  std::fill(sized.begin(), sized.end(), false);

  for (int r = 0; r < grid_.nrows(); ++r) {
    std::vector<int> pids;

    // get list of placement area ids on this row
    int c = 0;

    while (c < grid_.ncols()) {
      int cell = grid_.cell(r, c);

      ++c;

      while (c < grid_.ncols() && grid_.cell(r, c) == cell)
        ++c;

      int pid = getPlacementAreaIndex(cell);

      if (pid >= 0)
        pids.push_back(pid);
    }

    // resize placement areas on row
    int x1 = b;

    uint i2 = 0;

    while (i2 < pids.size()) {
      // get range to next sized width column
      uint i1 = i2;

      while (i2 < pids.size() && ! sized[uint(pids[i2])])
        ++i2;

      int ns = std::max(int(i2 - i1), 1);

      //---

      // calc start and end of range and width to fill
      int x2;

      if (i2 < pids.size())
        x2 = placementAreas_[uint(pids[i2])].x1() - ss;
      else
        x2 = w - b;

      int w1 = x2 - x1 - (ns - 1)*ss;

      for (uint i = i1; i < i2; ++i) {
        PlacementArea &placementArea = placementAreas_[uint(pids[i])];

        w1 -= placementArea.width;
      }

      // ensure we have not invalidated the row
      int  dw    = (w1 > 0 ? (w1 + ns - 1)/ns : (w1 - ns + 1)/ns);
      bool reset = false;

      for (uint i = i1; i < i2; ++i) {
        PlacementArea &placementArea = placementAreas_[uint(pids[i])];

        auto *area = getAreaForId(placementArea.areaId);

        int min_size = (area ? area->minimumSizeHint().width() :
                               CQTileAreaConstants::min_size + ss);

        if (placementArea.width + dw < min_size)
          reset = true;
      }

      if (reset) {
        int reset_width = 100;

        for (uint i = i1; i < i2; ++i) {
           PlacementArea &placementArea = placementAreas_[uint(pids[i])];

           placementArea.width = reset_width;
        }

        w1 = x2 - x1 - (ns - 1)*ss - ns*reset_width;
      }

      //---

      // share fill between columns
      for (uint i = i1; i < i2; ++i) {
        PlacementArea &placementArea = placementAreas_[uint(pids[i])];

        // adjust width
        int dw = w1/ns;

        placementArea.x      = x1;
        placementArea.width += dw;

        sized[uint(pids[i])] = true;

        w1 -= dw;

        --ns;

        x1 = placementArea.x2() + ss;
      }

      //---

      // move to next non sized column
      i1 = i2++;

      while (i2 < pids.size() && sized[uint(pids[i2])])
        ++i2;

      if (i2 < pids.size()) {
        PlacementArea &placementArea = placementAreas_[uint(pids[i2 - 1])];

        x1 = placementArea.x2() + ss;
      }
    }
  }

  //------

  // adjust areas to fit height
  int h = height();

  std::fill(sized.begin(), sized.end(), false);

  for (int c = 0; c < grid_.ncols(); ++c) {
    std::vector<int> pids;

    // get list of placement area ids on this column
    int r = 0;

    while (r < grid_.nrows()) {
      int cell = grid_.cell(r, c);

      ++r;

      while (r < grid_.nrows() && grid_.cell(r, c) == cell)
        ++r;

      int pid = getPlacementAreaIndex(cell);

      if (pid >= 0)
        pids.push_back(pid);
    }

    // resize placement areas on column
    int y1 = b;

    uint i2 = 0;

    while (i2 < pids.size()) {
      // get range to next sized width row
      uint i1 = i2;

      while (i2 < pids.size() && ! sized[uint(pids[i2])])
        ++i2;

      int ns = std::max(int(i2 - i1), 1);

      //---

      // calc start and end of range and height to fill
      int y2;

      if (i2 < pids.size())
        y2 = placementAreas_[uint(pids[i2])].y1() - ss;
      else
        y2 = h - b;

      int h1 = y2 - y1 - (ns - 1)*ss;

      for (uint i = i1; i < i2; ++i) {
        PlacementArea &placementArea = placementAreas_[uint(pids[i])];

        h1 -= placementArea.height;
      }

      // ensure we have not invalidated the column
      int  dh    = (h1 > 0 ? (h1 + ns - 1)/ns : (h1 - ns + 1)/ns);
      bool reset = false;

      for (uint i = i1; i < i2; ++i) {
        PlacementArea &placementArea = placementAreas_[uint(pids[i])];

        auto *area = getAreaForId(placementArea.areaId);

        int min_size = (area ? area->minimumSizeHint().height() :
                               CQTileAreaConstants::min_size + ss);

        if (placementArea.height + dh < min_size)
          reset = true;
      }

      if (reset) {
        int reset_height = 100;

        for (uint i = i1; i < i2; ++i) {
           PlacementArea &placementArea = placementAreas_[uint(pids[i])];

           placementArea.height = reset_height;
        }

        h1 = y2 - y1 - (ns - 1)*ss - ns*reset_height;
      }

      //---

      // share fill between rows
      for (uint i = i1; i < i2; ++i) {
        PlacementArea &placementArea = placementAreas_[uint(pids[i])];

        // adjust height
        int dh = h1/ns;

        placementArea.y       = y1;
        placementArea.height += dh;

        sized[uint(pids[i])] = true;

        h1 -= dh;

        --ns;

        y1 = placementArea.y2() + ss;
      }

      //---

      // move to next non sized row
      i1 = i2++;

      while (i2 < pids.size() && sized[uint(pids[i2])])
        ++i2;

      if (i2 < pids.size()) {
        PlacementArea &placementArea = placementAreas_[uint(pids[i2 - 1])];

        y1 = placementArea.y2() + ss;
      }
    }
  }

  //-------

  for (int r = 0; r < grid_.nrows(); ++r) {
    for (int c = 0; c < grid_.ncols(); ++c) {
      int cell = grid_.cell(r, c);

      int pid = getPlacementAreaIndex(cell);

      if (pid >= 0) {
        PlacementArea &placementArea = placementAreas_[uint(pid)];

        adjustSameWidthPlacements (placementArea);
        adjustSameHeightPlacements(placementArea);
      }
    }
  }
}

// ensure all cells to left and right of vertical splitter have same x/width
void
CQTileArea::
adjustSameWidthPlacements(PlacementArea &area)
{
  int b = border();

  int is;

  // right splitters
  if (getVSplitter(area.col1(), area.row1(), area.row2(), is)) {
    VSplitter &splitter = vsplitters_[area.col1()][uint(is)];

    for (AreaSet::iterator pr = splitter.rareas.begin(); pr != splitter.rareas.end(); ++pr) {
      int pid = *pr;

      PlacementArea &area1 = placementAreas_[uint(pid)];

      if (area1.x1() != area.x1())
        area1.x = area.x;
    }
  }

  // left splitters
  if (getVSplitter(area.col2(), area.row1(), area.row2(), is)) {
    VSplitter &splitter = vsplitters_[area.col2()][uint(is)];

    for (AreaSet::iterator pl = splitter.lareas.begin(); pl != splitter.lareas.end(); ++pl) {
      int pid = *pl;

      PlacementArea &area1 = placementAreas_[uint(pid)];

      if (area1.x2() != area.x2()) {
        area1.width = area.x + area.width - area1.x;
      }
    }
  }

  if (area.col2() == grid_.ncols())
    area.width = width() - b - area.x;
}

// ensure all cells above and below horizontal splitter have same y/height
void
CQTileArea::
adjustSameHeightPlacements(PlacementArea &area)
{
  int b = border();

  int is;

  // above splitters
  if (getHSplitter(area.row1(), area.col1(), area.col2(), is)) {
    HSplitter &splitter = hsplitters_[area.row1()][uint(is)];

    for (AreaSet::iterator pb = splitter.bareas.begin(); pb != splitter.bareas.end(); ++pb) {
      int pid = *pb;

      PlacementArea &area1 = placementAreas_[uint(pid)];

      if (area1.y1() != area.y1()) {
        area1.y = area.y;
      }
    }
  }

  // below splitters
  if (getHSplitter(area.row2(), area.col1(), area.col2(), is)) {
    HSplitter &splitter = hsplitters_[area.row2()][uint(is)];

    for (AreaSet::iterator pt = splitter.tareas.begin(); pt != splitter.tareas.end(); ++pt) {
      int pid = *pt;

      PlacementArea &area1 = placementAreas_[uint(pid)];

      if (area1.y2() != area.y2()) {
        area1.height = area.y + area.height - area1.y;
      }
    }
  }

  if (area.row2() == grid_.nrows())
    area.height = height() - b - area.y;
}

// update all area title bars
void
CQTileArea::
updateTitles()
{
  updateMenuBar();

  for (WindowAreas::iterator p = areas_.begin(); p != areas_.end(); ++p) {
    auto *area = (*p).second;

    area->updateTitle();
  }
}

// update menu bar
void
CQTileArea::
updateMenuBar()
{
  bool hasControls = isMaximized();

  if (hasControls != hasControls_) {
    hasControls_ = hasControls;

    if (hasControls_ && menuIcon_.isNull()) {
      menuIcon_     = new CQTileAreaMenuIcon(this);
      menuControls_ = new CQTileAreaMenuControls(this);
    }

    if (hasControls_) {
      auto *menuBar = window_->menuBar();

      if (menuBar->cornerWidget(Qt::TopLeftCorner) != menuIcon_)
        menuBar->setCornerWidget(menuIcon_, Qt::TopLeftCorner);

      if (menuBar->cornerWidget(Qt::TopRightCorner) != menuControls_)
        menuBar->setCornerWidget(menuControls_, Qt::TopRightCorner);

      menuIcon_    ->show();
      menuControls_->show();
    }
    else {
      if (! menuIcon_.isNull()) {
        menuIcon_    ->hide();
        menuControls_->hide();
      }
    }
  }

  //---

  if (! menuIcon_.isNull()) {
    menuIcon_    ->updateState();
    menuControls_->updateState();
  }
}

// get current window
CQTileWindow *
CQTileArea::
currentWindow() const
{
  if (! currentArea()) {
    if (! areas_.empty())
      return (*areas_.begin()).second->currentWindow();
    else
      return nullptr;
  }

  if (currentArea())
    return currentArea()->currentWindow();
  else
    return nullptr;
}

// set current window
void
CQTileArea::
setCurrentWindow(CQTileWindow *window)
{
  if (window == currentWindow())
    return;

  auto *area = getWindowArea(window);

  if (area) {
    area->setCurrentWindow(window);

    setCurrentArea(area);
  }

  updateCurrentWindow();

  window = currentWindow();

  if (window)
    window->widget()->setFocus(Qt::OtherFocusReason);
}

// update current window title and icon
void
CQTileArea::
updateCurrentWindow()
{
  auto *window = currentWindow();

  if (window) {
    window->area()->setWindowTitle(window->getTitle());
    window->area()->setWindowIcon (window->getIcon ());

    setWindowTitle(window->getTitle());
    setWindowIcon (window->getIcon ());
  }
}

// get area for window
CQTileWindowArea *
CQTileArea::
getWindowArea(CQTileWindow *window) const
{
  for (WindowAreas::const_iterator p = areas_.begin(); p != areas_.end(); ++p) {
    auto *area = (*p).second;

    if (area->hasWindow(window))
      return area;
  }

  return nullptr;
}

// notify current window has changed
void
CQTileArea::
emitCurrentWindowChanged()
{
  emit currentWindowChanged(currentWindow());
}

// set current area
void
CQTileArea::
setCurrentArea(CQTileWindowArea *area)
{
  if (area == currentArea_)
    return;

  currentArea_ = area;

  // update all areas
  for (WindowAreas::const_iterator p = areas_.begin(); p != areas_.end(); ++p) {
    auto *area = (*p).second;

    area->update();
  }

  // set focus to current area's current window
  if (currentArea_) {
    auto *currentWindow = currentArea_->currentWindow();

    if (currentWindow)
      currentWindow->widget()->setFocus(Qt::OtherFocusReason);
  }

  // notify current window changed
  emitCurrentWindowChanged();
}

// check if full screen display
bool
CQTileArea::
isFullScreen() const
{
  return grid_.isSingleCell();
}

// get number of windows
int
CQTileArea::
getNumWindows() const
{
  uint num = 0;

  for (WindowAreas::const_iterator p = areas_.begin(); p != areas_.end(); ++p) {
    auto *area = (*p).second;

    num += area->getWindows().size();
  }

  return int(num);
}

// get list of all windows
CQTileArea::Windows
CQTileArea::
getAllWindows() const
{
  Windows windows;

  for (WindowAreas::const_iterator p = areas_.begin(); p != areas_.end(); ++p) {
    auto *area = (*p).second;

    const auto &areaWindows = area->getWindows();

    std::copy(areaWindows.begin(), areaWindows.end(), std::back_inserter(windows));
  }

  return windows;
}

CQTileArea::PlacementArea &
CQTileArea::
getPlacementAreaForArea(CQTileWindowArea *area)
{
  int pid = getPlacementAreaIndex(area->id());

  return placementAreas_[uint(pid)];
}

// get array index of placement area of specified id
int
CQTileArea::
getPlacementAreaIndex(int id) const
{
  return getPlacementAreaIndex(placementAreas_, id);
}

// get array index of placement area of specified id
int
CQTileArea::
getPlacementAreaIndex(const PlacementAreas &placementAreas, int id) const
{
  uint np = uint(placementAreas.size());

  for (uint i = 0; i < np; ++i) {
    const PlacementArea &placementArea = placementAreas[i];

    // check for id match of area
    if (placementArea.areaId == id)
      return int(i);
  }

  return -1;
}

// get array at specified row, column
CQTileWindowArea *
CQTileArea::
getAreaAt(int row, int col) const
{
  int cell = grid_.cell(row, col);

  int ind = getPlacementAreaIndex(cell);

  if (ind >= 0)
    return getAreaForId(placementAreas_[uint(ind)].areaId);
  else
    return nullptr;
}

// get horizontal splitter at specified row and column range (index returned if found)
bool
CQTileArea::
getHSplitter(int row, int col1, int col2, int &is)
{
  is = -1;

  RowHSplitterArray::iterator p = hsplitters_.find(row);

  if (p == hsplitters_.end())
    return false;

  int il = 0;

  HSplitterArray &splitters = (*p).second;

  uint ns = uint(splitters.size());

  for (uint i = 0; i < ns; ++i) {
    HSplitter &splitter1 = splitters[i];

    if (col1 > splitter1.col2 || col2 < splitter1.col1) continue;

    if (col1 == splitter1.col1 && col2 == splitter1.col2) {
      is = int(i);
      return true;
    }

    int il1 = std::min(col2, splitter1.col2) - std::max(col1, splitter1.col1);

    if (is < 0 || il1 > il) {
      is = int(i);
      il = il1;
    }
  }

  return (is >= 0);
}

// get vertical splitter at specified column and row range (index returned if found)
bool
CQTileArea::
getVSplitter(int col, int row1, int row2, int &is)
{
  is = -1;

  ColVSplitterArray::iterator p = vsplitters_.find(col);

  if (p == vsplitters_.end())
    return false;

  int il = 0;

  VSplitterArray &splitters = (*p).second;

  uint ns = uint(splitters.size());

  for (uint i = 0; i < ns; ++i) {
    VSplitter &splitter1 = splitters[i];

    if (row1 > splitter1.row2 || row2 < splitter1.row1) continue;

    if (row1 == splitter1.row1 && row2 == splitter1.row2) {
      is = int(i);
      return true;
    }

    int il1 = std::min(row2, splitter1.row2) - std::max(row1, splitter1.row1);

    if (is < 0 || il1 > il) {
      is = int(i);
      il = il1;
    }
  }

  return (is >= 0);
}

// get bounding box of horizontal specified splitter
QRect
CQTileArea::
getHSplitterRect(const HSplitter &splitter) const
{
  int ss = splitterSize();

  int x1 = INT_MAX, x2 = INT_MIN, yt = INT_MIN, yb = INT_MAX;

  for (AreaSet::const_iterator pt = splitter.tareas.begin(); pt != splitter.tareas.end(); ++pt) {
    int pid = *pt;

    const PlacementArea &placementArea = placementAreas_[uint(pid)];

    yt = std::max(yt, placementArea.y2());

    x1 = std::min(x1, placementArea.x1());
    x2 = std::max(x2, placementArea.x2());
  }

  for (AreaSet::const_iterator pb = splitter.bareas.begin(); pb != splitter.bareas.end(); ++pb) {
    int pid = *pb;

    const PlacementArea &placementArea = placementAreas_[uint(pid)];

    yb = std::min(yb, placementArea.y1());

    x1 = std::min(x1, placementArea.x1());
    x2 = std::max(x2, placementArea.x2());
  }

  if (yt == INT_MIN) yt = yb - ss;
  if (yb == INT_MAX) yb = yt + ss;

  return QRect(x1, (yt + yb)/2 - ss/2, x2 - x1, ss);
}

// get bounding box of vertical specified splitter
QRect
CQTileArea::
getVSplitterRect(const VSplitter &splitter) const
{
  int ss = splitterSize();

  int y1 = INT_MAX, y2 = INT_MIN, xl = INT_MIN, xr = INT_MAX;

  for (AreaSet::const_iterator pl = splitter.lareas.begin(); pl != splitter.lareas.end(); ++pl) {
    int pid = *pl;

    const PlacementArea &placementArea = placementAreas_[uint(pid)];

    xl = std::max(xl, placementArea.x2());

    y1 = std::min(y1, placementArea.y1());
    y2 = std::max(y2, placementArea.y2());
  }

  for (AreaSet::const_iterator pr = splitter.rareas.begin(); pr != splitter.rareas.end(); ++pr) {
    int pid = *pr;

    const PlacementArea &placementArea = placementAreas_[uint(pid)];

    xr = std::min(xr, placementArea.x1());

    y1 = std::min(y1, placementArea.y1());
    y2 = std::max(y2, placementArea.y2());
  }

  if (xl == INT_MIN) xl = xr - ss;
  if (xr == INT_MAX) xr = xl + ss;

  return QRect((xl + xr)/2 - ss/2, y1, ss, y2 - y1);
}

// maximize all areas
void
CQTileArea::
maximizeSlot()
{
  maximizeWindows();
}

// maximize all windows
// TODO: skip non-docked
void
CQTileArea::
maximizeWindows()
{
  if (isMaximized())
    return;

  if (areas_.size() == 1)
    return;

  saveState(restoreState_, false);

  // save all windows
  // TODO: only docked
  std::vector<CQTileWindow *> windows;

  for (WindowAreas::const_iterator p = areas_.begin(); p != areas_.end(); ++p) {
    auto *area = (*p).second;

    const auto &areaWindows = area->getWindows();

    std::copy(areaWindows.begin(), areaWindows.end(), std::back_inserter(windows));
  }

  // reparent so not deleted
  for (auto p = windows.begin(); p != windows.end(); ++p)
    (*p)->setParent(this);

  // delete old areas (TODO: save one ?)
  for (auto p = areas_.begin(); p != areas_.end(); ++p) {
    auto *area = (*p).second;

    area->deleteLater();
  }

  // reset
  areas_.clear();
  grid_ .reset();

  currentArea_ = nullptr;

  // create new area
  auto *windowArea = addArea();

  // add windows to new area (as tabs)
  for (auto p = windows.begin(); p != windows.end(); ++p) {
    auto *window = *p;

    windowArea->addWindow(window);
  }

  addWindowArea(windowArea, 0, 0, 1, 1);
}

// restore all windows
void
CQTileArea::
restoreSlot()
{
  restoreWindows();
}

// restore all windows
// TODO: skip non-docked
void
CQTileArea::
restoreWindows()
{
  if (! restoreState_.valid_)
    return;

  PlacementState newState = restoreState_;

  saveState(restoreState_, false);

  restoreState(newState);

  updateTitles();
}

// tile all windows
void
CQTileArea::
tileSlot()
{
  tileWindows();
}

// tile all windows
// TODO: skip non-docked
void
CQTileArea::
tileWindows()
{
  // save all windows
  std::vector<CQTileWindow *> windows;

  for (WindowAreas::const_iterator p = areas_.begin(); p != areas_.end(); ++p) {
    auto *area = (*p).second;

    const auto &areaWindows = area->getWindows();

    std::copy(areaWindows.begin(), areaWindows.end(), std::back_inserter(windows));
  }

  bool newAreas = (areas_.size() != windows.size());

  if (newAreas) {
    // reparent so not deleted
    for (auto p = windows.begin(); p != windows.end(); ++p)
      (*p)->setParent(this);

    // delete old areas
    for (WindowAreas::const_iterator p = areas_.begin(); p != areas_.end(); ++p) {
      auto *area = (*p).second;

      area->deleteLater();
    }

    areas_.clear();
  }

  // reset
  grid_.reset();

  currentArea_ = nullptr;

  // determine grid size
  int nrows = int(std::sqrt(double(windows.size()) + 0.5));
  int ncols = int(windows.size())/nrows;

  if (int(windows.size()) % nrows) ++ncols;

  grid_.setSize(nrows, ncols);

  grid_.clear();

  // create new areas (one per window)
  int r = 0, c = 0;

  for (auto p = windows.begin(); p != windows.end(); ++p) {
    auto *window = *p;

    CQTileWindowArea *windowArea;

    if (newAreas) {
      windowArea = addArea();

      windowArea->addWindow(window);
    }
    else
      windowArea = window->area();

    //---

    grid_.cell(r, c) = windowArea->id();

    ++c;

    if (c >= ncols) {
      ++r;

      c = 0;
    }
  }

  // update placement (use new placement sizes)
  updatePlacement(false);
}

// is restore state valid
bool
CQTileArea::
isRestoreStateValid() const
{
  return restoreState_.valid_;
}

// detach current window
void
CQTileArea::
detachSlot()
{
  auto *area = currentArea();

  if (area)
    area->detachSlot();
}

// close current window
void
CQTileArea::
closeSlot()
{
  auto *area = currentArea();

  if (area)
    area->closeSlot();
}

// is maximized
bool
CQTileArea::
isMaximized() const
{
  return (areas_.size() <= 1);
}

// update placement on show (if non-visible placement is deferred)
void
CQTileArea::
showEvent(QShowEvent *)
{
  updatePlacement(false);
}

// update placement sizes on resize
void
CQTileArea::
resizeEvent(QResizeEvent *)
{
  adjustToFit();

  updatePlacementGeometries();
}

// draw splitters
void
CQTileArea::
paintEvent(QPaintEvent *)
{
  // draw horizontal splitters
  for (RowHSplitterArray::iterator p = hsplitters_.begin(); p != hsplitters_.end(); ++p) {
    HSplitterArray &splitters = (*p).second;

    uint ns = uint(splitters.size());

    for (uint i = 0; i < ns; ++i) {
      HSplitter &splitter = splitters[i];

      auto rect = getHSplitterRect(splitter);

      //if (splitter.splitterId == -1)
      //  splitter.splitterId = createSplitterWidget(Qt::Horizontal, (*p).first, i);

      auto *splitterWidget = getSplitterWidget(splitter.splitterId);

      if (splitterWidget)
        splitterWidget->setGeometry(rect);
    }
  }

  // draw vertical splitters
  for (ColVSplitterArray::iterator p = vsplitters_.begin(); p != vsplitters_.end(); ++p) {
    VSplitterArray &splitters = (*p).second;

    uint ns = uint(splitters.size());

    for (uint i = 0; i < ns; ++i) {
      VSplitter &splitter = splitters[i];

      auto rect = getVSplitterRect(splitter);

      //if (splitter.splitterId == -1)
      //  splitter.splitterId = createSplitterWidget(Qt::Vertical, (*p).first, i);

      auto *splitterWidget = getSplitterWidget(splitter.splitterId);

      if (splitterWidget)
        splitterWidget->setGeometry(rect);
    }
  }
}

int
CQTileArea::
createSplitterWidget(Qt::Orientation orient, int pos, int ind)
{
  CQTileAreaSplitter *splitter = nullptr;
  int                 id       = -1;

  for (SplitterWidgets::iterator p = splitterWidgets_.begin(); p != splitterWidgets_.end(); ++p) {
    auto *splitter1 = (*p).second;

    if (splitter1->used()) continue;

    splitter = splitter1;
    id       = (*p).first;

    break;
  }

  if (! splitter) {
    id = int(splitterWidgets_.size());

    splitter = new CQTileAreaSplitter(this);

    splitter->setObjectName(QString("splitter%1").arg(id));

    splitterWidgets_[id] = splitter;
  }

  splitter->init(orient, pos, ind);

  splitter->setUsed(true);

  return id;
}

CQTileAreaSplitter *
CQTileArea::
getSplitterWidget(int ind) const
{
  SplitterWidgets::const_iterator p = splitterWidgets_.find(ind);

  if (p != splitterWidgets_.end())
    return (*p).second;

  return nullptr;
}

// draw rubberband for currently highlighted area side (returns rectangle used)
QRect
CQTileArea::
updateRubberBand()
{
  QRect rect;

  auto tl = mapToGlobal(QPoint(0, 0));

  if (! animateDrag()) {
    if (highlight_.side != NO_SIDE) {
      int hs = CQTileAreaConstants::highlight_size;

      if      (highlight_.ind >= 0) {
        const PlacementArea &area = placementAreas_[uint(highlight_.ind)];

        if      (highlight_.side == LEFT_SIDE)
          rect = QRect(area.x1() - hs/2, area.y1(), hs/2, area.y2() - area.y1());
        else if (highlight_.side == RIGHT_SIDE)
          rect = QRect(area.x2(), area.y1(), hs/2, area.y2() - area.y1());
        else if (highlight_.side == TOP_SIDE)
          rect = QRect(area.x1(), area.y1() - hs/2, area.x2() - area.x1(), hs/2);
        else if (highlight_.side == BOTTOM_SIDE)
          rect = QRect(area.x1(), area.y2(), area.x2() - area.x1(), hs/2);
        else if (highlight_.side == MIDDLE_SIDE)
          rect = QRect(area.x1(), area.y1(), area.x2() - area.x1(),  area.y2() - area.y1());
      }
      else
        rect = QRect(0, 0, width(), height());

      rect.adjust(tl.x(), tl.y(), tl.x(), tl.y());

      rubberBand_->setGeometry(rect);

      rubberBand_->show();
    }
    else
      hideRubberBand();
  }
  else {
    int pid = getPlacementAreaIndex(0);

    if (pid < 0)
      pid = highlight_.ind;

    if (pid >= 0) {
      PlacementArea &placementArea = placementAreas_[uint(pid)];

      rect = placementArea.rect().adjusted(tl.x(), tl.y(), tl.x(), tl.y());

      rubberBand_->setGeometry(rect);

      rubberBand_->show();
    }
    else
      hideRubberBand();
  }

  return rect;
}

// hide rubberband
void
CQTileArea::
hideRubberBand()
{
  rubberBand_->hide();
}

void
CQTileArea::
moveHSplitter(int row, int ind, int dy)
{
  HSplitter &splitter = hsplitters_[row][uint(ind)];

  // limit dy
  for (AreaSet::iterator pt = splitter.tareas.begin(); pt != splitter.tareas.end(); ++pt) {
    int pid = *pt;

    PlacementArea &placementArea = placementAreas_[uint(pid)];

    auto *area = getAreaForId(placementArea.areaId);

    int min_size = (area ? area->minimumSizeHint().height() : CQTileAreaConstants::min_size);

    if (dy < 0 && placementArea.height + dy <= min_size)
      dy = min_size - placementArea.height;
  }

  for (AreaSet::iterator pb = splitter.bareas.begin(); pb != splitter.bareas.end(); ++pb) {
    int pid = *pb;

    PlacementArea &placementArea = placementAreas_[uint(pid)];

    auto *area = getAreaForId(placementArea.areaId);

    int min_size = (area ? area->minimumSizeHint().height() : CQTileAreaConstants::min_size);

    if (dy > 0 && placementArea.height - dy <= min_size)
      dy = placementArea.height - min_size;
  }

  // apply dy to placement areas
  for (AreaSet::iterator pt = splitter.tareas.begin(); pt != splitter.tareas.end(); ++pt) {
    int pid = *pt;

    PlacementArea &placementArea = placementAreas_[uint(pid)];

    placementArea.height += dy;

    updatePlacementGeometry(placementArea);
  }

  for (AreaSet::iterator pb = splitter.bareas.begin(); pb != splitter.bareas.end(); ++pb) {
    int pid = *pb;

    PlacementArea &placementArea = placementAreas_[uint(pid)];

    placementArea.y      += dy;
    placementArea.height -= dy;

    updatePlacementGeometry(placementArea);
  }
}

void
CQTileArea::
moveVSplitter(int col, int ind, int dx)
{
  // move current vertical splitter by mouse delta
  VSplitter &splitter = vsplitters_[col][uint(ind)];

  // limit dx
  for (AreaSet::iterator pl = splitter.lareas.begin(); pl != splitter.lareas.end(); ++pl) {
    int pid = *pl;

    PlacementArea &placementArea = placementAreas_[uint(pid)];

    auto *area = getAreaForId(placementArea.areaId);

    int min_size = (area ? area->minimumSizeHint().width() : CQTileAreaConstants::min_size);

    if (dx < 0 && placementArea.width + dx <= min_size)
      dx = min_size - placementArea.width;
  }

  for (AreaSet::iterator pr = splitter.rareas.begin(); pr != splitter.rareas.end(); ++pr) {
    int pid = *pr;

    PlacementArea &placementArea = placementAreas_[uint(pid)];

    auto *area = getAreaForId(placementArea.areaId);

    int min_size = (area ? area->minimumSizeHint().width() : CQTileAreaConstants::min_size);

    if (dx > 0 && placementArea.width - dx <= min_size)
      dx = placementArea.width - min_size;
  }

  // apply dx to placement areas
  for (AreaSet::iterator pl = splitter.lareas.begin(); pl != splitter.lareas.end(); ++pl) {
    int pid = *pl;

    PlacementArea &placementArea = placementAreas_[uint(pid)];

    placementArea.width += dx;

    updatePlacementGeometry(placementArea);
  }

  for (AreaSet::iterator pr = splitter.rareas.begin(); pr != splitter.rareas.end(); ++pr) {
    int pid = *pr;

    PlacementArea &placementArea = placementAreas_[uint(pid)];

    placementArea.x     += dx;
    placementArea.width -= dx;

    updatePlacementGeometry(placementArea);
  }
}

// get horizontal splitter at position
CQTileArea::SplitterInd
CQTileArea::
getHSplitterAtPos(const QPoint &pos) const
{
  for (RowHSplitterArray::const_iterator p = hsplitters_.begin(); p != hsplitters_.end(); ++p) {
    const HSplitterArray &splitters = (*p).second;

    uint ns = uint(splitters.size());

    for (uint i = 0; i < ns; ++i) {
      const HSplitter &splitter = splitters[i];

      auto *splitterWidget = getSplitterWidget(splitter.splitterId);

      if (splitterWidget->rect().contains(pos))
        return SplitterInd((*p).first, i);
    }
  }

  return SplitterInd(-1,-1);
}

// get vertical splitter at position
CQTileArea::SplitterInd
CQTileArea::
getVSplitterAtPos(const QPoint &pos) const
{
  for (ColVSplitterArray::const_iterator p = vsplitters_.begin(); p != vsplitters_.end(); ++p) {
    const VSplitterArray &splitters = (*p).second;

    uint ns = uint(splitters.size());

    for (uint i = 0; i < ns; ++i) {
      const VSplitter &splitter = splitters[i];

      auto *splitterWidget = getSplitterWidget(splitter.splitterId);

      if (splitterWidget->rect().contains(pos))
        return SplitterInd((*p).first, i);
    }
  }

  return SplitterInd(-1,-1);
}

// set highlighted area and area side for specified (global) position
void
CQTileArea::
setHighlight(const QPoint &pos)
{
  highlight_ = Highlight(-1, NO_SIDE);

  // ignore if not inside area
  auto tl = mapToGlobal(QPoint(          0,            0));
  auto br = mapToGlobal(QPoint(width() - 1, height() - 1));

  if (pos.x() < tl.x() || pos.x() > br.x() || pos.y() < tl.y() || pos.y() > br.y()) {
    update();
    return;
  }

  //-------

  // check inside each placement area
  int  minD    = INT_MAX;
  int  minI    = -1     ;
  Side minSide = LEFT_SIDE;

  uint np = uint(placementAreas_.size());

  for (uint i = 0; i < np; ++i) {
    const PlacementArea &placementArea = placementAreas_[i];

    // determine nearest side or center depending on distance to edges or rectangle center
    int dx1 = abs(tl.x() + placementArea.x1() - pos.x());
    int dy1 = abs(tl.y() + placementArea.y1() - pos.y());
    int dx2 = abs(tl.x() + placementArea.x2() - pos.x());
    int dy2 = abs(tl.y() + placementArea.y2() - pos.y());

    int  d;
    Side side = NO_SIDE;

    // to left of area
    if      (pos.x() < tl.x() + placementArea.x1()) {
      if      (pos.y() < tl.y() + placementArea.y1()) {
        d    = int(std::sqrt(double(dx1*dx1 + dy1*dy1)));
        side = (dx1 < dy1 ? LEFT_SIDE : TOP_SIDE);
      }
      else if (pos.y() > tl.y() + placementArea.y2()) {
        d    = int(std::sqrt(double(dx1*dx1 + dy2*dy2)));
        side = (dx1 < dy1 ? LEFT_SIDE : BOTTOM_SIDE);
      }
      else {
        d    = dx1;
        side = LEFT_SIDE;
      }
    }
    // to right of area
    else if (pos.x() > tl.x() + placementArea.x2()) {
      if      (pos.y() < tl.y() + placementArea.y1()) {
        d    = int(std::sqrt(double(dx2*dx2 + dy1*dy1)));
        side = (dx1 < dy1 ? RIGHT_SIDE : TOP_SIDE);
      }
      else if (pos.y() > tl.y() + placementArea.y2()) {
        d    = int(std::sqrt(double(dx2*dx2 + dy2*dy2)));
        side = (dx1 < dy1 ? RIGHT_SIDE : BOTTOM_SIDE);
      }
      else {
        d    = dx2;
        side = RIGHT_SIDE;
      }
    }
    // inside (x) of area
    else {
      if      (pos.y() < tl.y() + placementArea.y1()) {
        d    = dy1;
        side = TOP_SIDE;
      }
      else if (pos.y() > tl.y() + placementArea.y2()) {
        d    = dy2;
        side = BOTTOM_SIDE;
      }
      else {
        d = std::min(dx1, std::min(dx2, std::min(dy1, dy2)));

        if      (dx1 == d) side = LEFT_SIDE;
        else if (dx2 == d) side = RIGHT_SIDE;
        else if (dy1 == d) side = TOP_SIDE;
        else if (dy2 == d) side = BOTTOM_SIDE;
        else assert(false);
      }
    }

    // update nearest side
    if (d < minD) { minD = d; minI = int(i); minSide = side; }

    int dxm = abs(tl.x() + placementArea.xm() - pos.x());
    int dym = abs(tl.y() + placementArea.ym() - pos.y());

    //----

    // check center distance
    d = int(std::sqrt(double(dxm*dxm + dym*dym)));

    if (d < minD) { minD = d; minI = int(i); minSide = MIDDLE_SIDE; }
  }

  //------

  // set highlight to nearest
  highlight_ = Highlight(minI, minSide);

  if (! animateDrag())
    (void) updateRubberBand();
}

// clear highlight
void
CQTileArea::
clearHighlight()
{
  highlight_.ind  = -1;
  highlight_.side = NO_SIDE;

  if (! animateDrag())
    hideRubberBand();
}

// get highlight grid row and column ranges
bool
CQTileArea::
getHighlightPos(Side &side, int &row1, int &col1, int &row2, int &col2)
{
  if (highlight_.side == NO_SIDE) return false;

  if (highlight_.ind >= 0) {
    side = highlight_.side;

    const PlacementArea &area = placementAreas_[uint(highlight_.ind)];

    if      (highlight_.side == LEFT_SIDE) {
      row1 = area.row1(); row2 = area.row2();
      col1 = area.col1(); col2 = area.col1();
    }
    else if (highlight_.side == RIGHT_SIDE) {
      row1 = area.row1(); row2 = area.row2();
      col1 = area.col2(); col2 = area.col2();
    }
    else if (highlight_.side == TOP_SIDE) {
      row1 = area.row1(); row2 = area.row1();
      col1 = area.col1(); col2 = area.col2();
    }
    else if (highlight_.side == BOTTOM_SIDE) {
      row1 = area.row2(); row2 = area.row2();
      col1 = area.col1(); col2 = area.col2();
    }
    else if (highlight_.side == MIDDLE_SIDE) {
      row1 = area.row1(); row2 = area.row2();
      col1 = area.col1(); col2 = area.col2();
    }
    else
      return false;
  }
  else {
    side = LEFT_SIDE;

    row1 = 0; row2 = 1;
    col1 = 0; col2 = 1;
  }

  return true;
}

// save placement state
void
CQTileArea::
saveState(PlacementState &state, bool transient)
{
  // save grid, placement and splitters
  state.valid_          = true;
  state.transient_      = transient;
  state.grid_           = grid_;
  state.placementAreas_ = placementAreas_;
  state.hsplitters_     = hsplitters_;
  state.vsplitters_     = vsplitters_;

  // for transient we know we are not modifying the data so we don't
  // need to save the windows for each area
  if (! transient) {
    for (uint i = 0; i < placementAreas_.size(); ++i) {
      auto *area = getAreaForId(placementAreas_[i].areaId);
      assert(area);

      const auto &areaWindows = area->getWindows();

      state.placementAreas_[i].windows = areaWindows;
    }
  }
}

// restore placement state
void
CQTileArea::
restoreState(const PlacementState &state)
{
  assert(state.valid_);

  // restore grid, placement areas and splitters
  grid_           = state.grid_;
  placementAreas_ = state.placementAreas_;
  hsplitters_     = state.hsplitters_;
  vsplitters_     = state.vsplitters_;

  // if not transient then rebuild all the areas from the saved area windows
  if (! state.transient_) {
    int currentAreaInd = -1;

    for (uint i = 0; i < placementAreas_.size(); ++i) {
      PlacementArea &area = placementAreas_[i];

      // reparent windows so not deleted
      for (Windows::iterator p = area.windows.begin(); p != area.windows.end(); ++p)
        (*p)->setParent(this);

      if (area.areaId == currentArea_->id())
        currentAreaInd = int(i);
    }

    // save old areas and clear areas
    WindowAreas areas = areas_;

    areas_.clear();

    currentArea_ = nullptr;

    // create new areas for placement areas
    for (uint i = 0; i < placementAreas_.size(); ++i) {
      PlacementArea &area = placementAreas_[i];

      auto *newArea = addArea();

      // add windows to area
      for (Windows::iterator p = area.windows.begin(); p != area.windows.end(); ++p)
        newArea->addWindow(*p);

      // replace old id in grid with new id
      grid_.replace(placementAreas_[i].areaId, newArea->id());

      // update placement area id
      placementAreas_[i].areaId = newArea->id();

      // update current area
      if (int(i) == currentAreaInd)
        currentArea_ = newArea;
    }

    // delete old areas
    for (WindowAreas::const_iterator p = areas.begin(); p != areas.end(); ++p) {
      auto *area = (*p).second;

      area->deleteLater();
    }

    if (CQTileAreaConstants::debug_grid)
      grid_.print(std::cerr);

    emitCurrentWindowChanged();
  }

  // update widgets to new sizes
  adjustToFit();

  updatePlacementGeometries();
}

// get area for id
CQTileWindowArea *
CQTileArea::
getAreaForId(int areaId) const
{
  WindowAreas::const_iterator p = areas_.find(areaId);

  if (p != areas_.end())
    return (*p).second;
  else
    return nullptr;
}

// set default placement size
void
CQTileArea::
setDefPlacementSize(int w, int h)
{
  defWidth_  = w;
  defHeight_ = h;
}

// print grid (debug)
void
CQTileArea::
printSlot()
{
  grid_.print(std::cerr);
}

// fill empty areas (debug)
void
CQTileArea::
fillSlot()
{
  fillEmptyCells();
}

// remove duplicate rows/columns (debug)
void
CQTileArea::
dupSlot()
{
  removeDuplicateCells();
}

// update placement (debug)
void
CQTileArea::
placeSlot()
{
  gridToPlacement();

  updatePlacementGeometries();

  update();
}

// adjust to size (debug)
void
CQTileArea::
adjustSlot()
{
  adjustToFit();

  updatePlacementGeometries();
}

// return size hint
QSize
CQTileArea::
sizeHint() const
{
  std::map<int,int> widths;
  std::map<int,int> heights;

  int ncells = grid_.nrows()*grid_.ncols();

  for (int ci = 0; ci < ncells; ++ci) {
    int cell = grid_.cell(ci);
    if (cell < 0) continue;

    int pid = getPlacementAreaIndex(cell);
    if (pid < 0) continue;

    const PlacementArea &placementArea = placementAreas_[uint(pid)];
    if (! placementArea.areaId) continue;

    auto *area = getAreaForId(placementArea.areaId);
    if (! area) continue;

    int r = ci / grid_.ncols();
    int c = ci % grid_.ncols();

    auto s = area->sizeHint();

    widths [c] = std::max(widths [c], s.width ()/placementArea.ncols);
    heights[r] = std::max(heights[r], s.height()/placementArea.nrows);
  }

  int w = 0, h = 0;

  for (int c = 0; c > grid_.ncols(); ++c)
    w += widths[c];

  for (int r = 0; r > grid_.nrows(); ++r)
    h += heights[r];

  return QSize(w, h);
}

// return minimum size hint
QSize
CQTileArea::
minimumSizeHint() const
{
  QFontMetrics fm(font());

  int fh = fm.height() + 4;

  int w = grid_.ncols()*4;
  int h = grid_.nrows()*(fh + 4);

  return QSize(w, h);
}

//-------

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

  //const auto &screenRect = QApplication::desktop()->availableGeometry();
  auto *srn = QApplication::screens().at(0);
  auto screenRect = srn->availableGeometry();


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

//-------

// create tabbar
CQTileWindowTabBar::
CQTileWindowTabBar(CQTileWindowArea *area) :
 area_(area), contextMenu_(nullptr)
{
  setObjectName("tabbar");

  setShape(QTabBar::RoundedSouth);
  setMovable(true);

  setContextMenuPolicy(Qt::DefaultContextMenu);
}

// mouse press activates current window
void
CQTileWindowTabBar::
mousePressEvent(QMouseEvent *e)
{
  area_->area()->setCurrentArea(area_);

  QTabBar::mousePressEvent(e);
}

// display context menu
void
CQTileWindowTabBar::
contextMenuEvent(QContextMenuEvent *e)
{
  if (! contextMenu_)
    contextMenu_ = area_->createContextMenu(this);

  area_->updateContextMenu(contextMenu_);

  contextMenu_->popup(e->globalPos());

  e->accept();
}

//------

// create title bar
CQTileWindowTitle::
CQTileWindowTitle(CQTileWindowArea *area) :
 area_(area), contextMenu_(nullptr)
{
  // create detach/attach, maximize/restore and close buttons
  // TODO: tile button
  detachButton_   = addButton(QPixmap(detach_data  ));
  maximizeButton_ = addButton(QPixmap(maximize_data));
  closeButton_    = addButton(QPixmap(close_data   ));

  connect(detachButton_  , SIGNAL(clicked()), this, SLOT(detachSlot()));
  connect(maximizeButton_, SIGNAL(clicked()), this, SLOT(maximizeSlot()));
  connect(closeButton_   , SIGNAL(clicked()), area, SLOT(closeSlot()));

  detachButton_  ->setToolTip("Detach");
  maximizeButton_->setToolTip("Maximize");
  closeButton_   ->setToolTip("Close");

  // add hover for cursor update
  setAttribute(Qt::WA_Hover);

  // enable context menu
  setContextMenuPolicy(Qt::DefaultContextMenu);

  // no focus (except when dragging)
  setFocusPolicy(Qt::NoFocus);
}

// update from window state
void
CQTileWindowTitle::
updateState()
{
  // update icon for detach/attach, maximize/restore buttons from state
  maximizeButton_->setIcon(area_->isMaximized() ? QPixmap(restore_data) : QPixmap(maximize_data));
  detachButton_  ->setIcon(! area_->isDocked () ? QPixmap(attach_data ) : QPixmap(detach_data  ));

  maximizeButton_->setToolTip(area_->isMaximized() ? "Restore" : "Maximize");
  detachButton_  ->setToolTip(! area_->isDocked () ? "Attach"  : "Detach"  );

  if (area_->isMaximized())
    maximizeButton_->setEnabled(area_->area()->isRestoreStateValid());
  else
    maximizeButton_->setEnabled(true);
}

// handle detach/attach
void
CQTileWindowTitle::
detachSlot()
{
  if (! area_->isDocked())
    area_->attachSlot();
  else
    area_->detachSlot();
}

// handle maximize/restore
void
CQTileWindowTitle::
maximizeSlot()
{
  if (area_->isMaximized())
    area_->restoreSlot();
  else
    area_->maximizeSlot();
}

QString
CQTileWindowTitle::
title() const
{
  return area_->getTitle();
}

QIcon
CQTileWindowTitle::
icon() const
{
  return area_->getIcon();
}

QColor
CQTileWindowTitle::
backgroundColor() const
{
  auto *tileArea = area_->area();

  bool current = (area_ == tileArea->currentArea());

  return (current ? tileArea->titleActiveColor() : tileArea->titleInactiveColor());
}

QColor
CQTileWindowTitle::
barColor() const
{
  auto *tileArea = area_->area();

  bool current = (area_ == tileArea->currentArea());

  return (current ? CQTileAreaConstants::bar_active_fg : CQTileAreaConstants::bar_inactive_fg);
}

// display context menu
void
CQTileWindowTitle::
contextMenuEvent(QContextMenuEvent *e)
{
  assert(area_);

  if (! contextMenu_)
    contextMenu_ = area_->createContextMenu(this);

  area_->updateContextMenu(contextMenu_);

  contextMenu_->popup(e->globalPos());

  e->accept();
}

// handle mouse press
void
CQTileWindowTitle::
mousePressEvent(QMouseEvent *e)
{
  // init mouse state
  mouseState_.reset();

  mouseState_.pressed  = true;
  mouseState_.pressPos = e->globalPos();
  mouseState_.dragAll  = (e->modifiers() & Qt::ShiftModifier);

  // ensure we are the current area
  area_->area()->setCurrentArea(area_);

  // grab key focus
  setFocusPolicy(Qt::StrongFocus);

  // initialize attach data
  area_->initAttach();
}

// handle mouse move
void
CQTileWindowTitle::
mouseMoveEvent(QMouseEvent *e)
{
  // ignore if mouse not pressed or user escaped operation
  if (! mouseState_.pressed || mouseState_.escapePress) return;

  // if not moved yet ensure move far enough
  if (! mouseState_.moved) {
    if ((e->globalPos() - mouseState_.pressPos).manhattanLength() <
         QApplication::startDragDistance())
      return;

    mouseState_.moved = true;
  }

  // detach and start animate
  if (area_->isDocked()) {
    area_->detach(e->globalPos(), true, mouseState_.dragAll);

    if (area_->area()->animateDrag())
      area_->startAttachPreview();
  }

  // move detached window
  int dx = e->globalPos().x() - mouseState_.pressPos.x();
  int dy = e->globalPos().y() - mouseState_.pressPos.y();

  area_->move(area_->pos() + QPoint(dx, dy));

  mouseState_.pressPos = e->globalPos();

  // show drop bound position (animated if needed)
  if (area_->area()->animateDrag())
    area_->doAttachPreview();
  else
    area_->area()->setHighlight(mouseState_.pressPos);

  // redraw
  area_->area()->update();
}

// handle mouse release
void
CQTileWindowTitle::
mouseReleaseEvent(QMouseEvent *)
{
  // attach (if escape not pressed)
  if (! mouseState_.escapePress) {
    // stop animating
    if (area_->area()->animateDrag())
      area_->stopAttachPreview();

    //---

    CQTileArea::Side side;
    int              row1, col1, row2, col2;

    if (area_->area()->getHighlightPos(side, row1, col1, row2, col2))
      area_->attach(false);
    else {
      if (mouseState_.moved)
        area_->setFloated();
    }

    area_->area()->clearHighlight();
  }

  // reset state
  mouseState_.reset();

  setFocusPolicy(Qt::NoFocus);

  // update titles
  area_->area()->updateTitles();
}

// double click maximizes/restores
void
CQTileWindowTitle::
mouseDoubleClickEvent(QMouseEvent *)
{
  maximizeSlot();
}

// handle key press (escape cancel)
// TODO: other shortcuts
void
CQTileWindowTitle::
keyPressEvent(QKeyEvent *e)
{
  if (e->key() == Qt::Key_Escape) {
    if (! mouseState_.escapePress && mouseState_.pressed && ! area_->isDocked()) {
      mouseState_.escapePress = true;

      area_->cancelAttach();

      area_->area()->clearHighlight();
    }
  }
}

// update cursor
bool
CQTileWindowTitle::
event(QEvent *e)
{
  switch (e->type()) {
    case QEvent::HoverEnter:
    case QEvent::HoverMove: {
      if (insideTitle(dynamic_cast<QHoverEvent *>(e)->pos()))
        setCursor(Qt::SizeAllCursor);
      else
        setCursor(Qt::ArrowCursor);
      break;
    }
    case QEvent::HoverLeave:
      unsetCursor();
      break;
    default:
      break;
  }

  return QWidget::event(e);
}

//-------

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

//------

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

//------

// create splitter
CQTileAreaSplitter::
CQTileAreaSplitter(CQTileArea *area) :
 QWidget(area), area_(area), orient_(Qt::Vertical), pos_(-1), ind_(-1),
 used_(false), mouseOver_(false)
{
  setCursor(Qt::SplitHCursor);

  setVisible(false);
}

void
CQTileAreaSplitter::
init(Qt::Orientation orient, int pos, int ind)
{
  orient_ = orient;
  pos_    = pos;
  ind_    = ind;

  if (orient_ == Qt::Vertical)
    setCursor(Qt::SplitHCursor);
  else
    setCursor(Qt::SplitVCursor);
}

void
CQTileAreaSplitter::
setUsed(bool used)
{
  used_ = used;

  setVisible(used_);
}

void
CQTileAreaSplitter::
paintEvent(QPaintEvent *)
{
  QStylePainter ps(this);

  QStyleOption opt;

  opt.initFrom(this);

  opt.rect  = rect();
  opt.state = (orient_ == Qt::Horizontal ? QStyle::State_None : QStyle::State_Horizontal);

  if (mouseState_.pressed)
    opt.state |= QStyle::State_Sunken;

  if (mouseOver_)
    opt.state |= QStyle::State_MouseOver;

  ps.drawControl(QStyle::CE_Splitter, opt);
}

// handle mouse press (start move of splitter)
void
CQTileAreaSplitter::
mousePressEvent(QMouseEvent *e)
{
  mouseState_.pressed  = true;
  mouseState_.pressPos = e->globalPos();

  update();
}

// handle mouse move (move splitter to resize placement areas or highlight splitter under mouse)
void
CQTileAreaSplitter::
mouseMoveEvent(QMouseEvent *e)
{
  if (! mouseState_.pressed) return;

  if (orient_ == Qt::Horizontal) {
    int dy = e->globalPos().y() - mouseState_.pressPos.y();

    area_->moveHSplitter(pos_, ind_, dy);
  }
  else {
    int dx = e->globalPos().x() - mouseState_.pressPos.x();

    area_->moveVSplitter(pos_, ind_, dx);
  }

  mouseState_.pressPos = e->globalPos();

  update();
}

// handle mouse release
void
CQTileAreaSplitter::
mouseReleaseEvent(QMouseEvent *)
{
  mouseState_.pressed = false;

  update();
}

void
CQTileAreaSplitter::
enterEvent(QEvent *)
{
  mouseOver_ = true;

  update();
}

void
CQTileAreaSplitter::
leaveEvent(QEvent *)
{
  mouseOver_ = false;

  update();
}

//------

CQTileAreaMenuIcon::
CQTileAreaMenuIcon(CQTileArea *area) :
 QLabel(area), area_(area)
{
  setObjectName("menuIcon");

  QFontMetrics fm(font());

  int s = fm.height() - 2;

  setFixedSize(s, s);
}

void
CQTileAreaMenuIcon::
updateState()
{
  auto *window = area_->currentWindow();

  if (window)
    setIcon(window->getIcon());
  else
    setIcon(QIcon());
}

void
CQTileAreaMenuIcon::
setIcon(const QIcon &icon)
{
  int s = height();

  if (! icon.isNull())
    setPixmap(icon.pixmap(s,s));
  else
    setPixmap(QPixmap(s,s));
}

//------

CQTileAreaMenuControls::
CQTileAreaMenuControls(CQTileArea *area) :
 QFrame(area), area_(area)
{
  setObjectName("menuControls");

  auto *layout = new QHBoxLayout(this);
  layout->setMargin(0); layout->setSpacing(1);

  detachButton_  = createButton(detach_data , "Detach" );
  restoreButton_ = createButton(restore_data, "Restore");
  closeButton_   = createButton(close_data  , "Close"  );

  layout->addWidget(detachButton_ );
  layout->addWidget(restoreButton_);
  layout->addWidget(closeButton_  );

  connect(detachButton_ , SIGNAL(clicked()), area_, SLOT(detachSlot()));
  connect(restoreButton_, SIGNAL(clicked()), area_, SLOT(restoreSlot()));
  connect(closeButton_  , SIGNAL(clicked()), area_, SLOT(closeSlot()));
}

void
CQTileAreaMenuControls::
updateState()
{
  restoreButton_->setEnabled(area_->isRestoreStateValid());
}

QToolButton *
CQTileAreaMenuControls::
createButton(const char **data, const QString &tip)
{
  QFontMetrics fm(font());

  int s = std::min(fm.height() + 2, 15);

  auto *button = new QToolButton;

  button->setObjectName(tip);

  button->setFixedSize(s, s);

  button->setIcon(QPixmap(data));
  button->setAutoRaise(true);
  button->setToolTip(tip);

  return button;
}
