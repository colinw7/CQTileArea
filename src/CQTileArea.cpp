#include <CQTileArea.h>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QTabBar>
#include <QStylePainter>
#include <QStyleOption>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QRubberBand>

#include <set>
#include <iostream>
#include <cassert>
#include <cmath>

#include <images/close.xpm>
#include <images/maximize.xpm>
#include <images/minimize.xpm>
#include <images/restore.xpm>

namespace CQTileAreaConstants {
  bool debug_grid        = true;
  int  min_size          = 4;
  int  splitter_size     = 6;
  bool combine_splitters = true;
  bool detach_tab        = true;
  bool animated_preview  = true;
  bool rand_resize       = false;
}

CQTileArea::
CQTileArea() :
 maximized_(false), currentArea_(0)
{
  setMouseTracking(true);

  rubberBand_ = new QRubberBand(QRubberBand::Rectangle);

  rubberBand_->hide();
}

// add window as a new column in the grid
CQTileWindow *
CQTileArea::
addWindow(QWidget *w)
{
  // new grid postion is next column with a height of all rows
  // (ensure at least one row if table empty)
  int row   = 0;
  int col   = grid_.ncols;
  int nrows = std::max(grid_.nrows, 1);
  int ncols = 1;

  return addWindow(w, row, col, nrows, ncols);
}

// add window at specified row and column using specified number of rows and colums
CQTileWindow *
CQTileArea::
addWindow(QWidget *w, int row, int col, int nrows, int ncols)
{
  // create new area
  CQTileWindowArea *windowArea = addArea();

  //------

  // add widget to window area
  CQTileWindow *window = windowArea->addWidget(w);

  //------

  addWindowArea(windowArea, row, col, nrows, ncols);

  //------

  return window;
}

CQTileWindowArea *
CQTileArea::
addArea()
{
  CQTileWindowArea *windowArea = new CQTileWindowArea(this);

  areas_[windowArea->id()] = windowArea;

  setCurrentArea(windowArea);

  return windowArea;
}

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
  else if (row >= grid_.nrows)
    insertRows(grid_.nrows, row - grid_.nrows + 1);

  // insert left
  if      (col < 0) {
    insertColumns(0, -col);
    col = 0;
  }
  // insert right
  else if (col >= grid_.ncols)
    insertColumns(grid_.ncols, col - grid_.ncols + 1);

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

      for ( ; splitCol > 0 && splitCol < grid_.ncols; ++splitCol) {
        bool valid = true;

        for (int r = 0; r < grid_.nrows; ++r) {
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

      for ( ; splitRow > 0 && splitRow < grid_.nrows; ++splitRow) {
        bool valid = true;

        for (int c = 0; c < grid_.ncols; ++c) {
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

  for (int r = row; r <= row1; ++r)
    for (int c = col; c <= col1; ++c)
      grid_.cell(r, c) = fillId;

  //------

  if (isVisible()) {
    // remove empty cells and cleanup duplicate rows and columns
    fillEmptyCells();

    removeDuplicateCells();

    //------

    // update placement
    gridToPlacement();

    adjustToFit();
  }

  if (CQTileAreaConstants::debug_grid)
    grid_.print(std::cerr);
}

void
CQTileArea::
removeArea(CQTileWindowArea *area)
{
  areas_.erase(area->id());

  int np = placementAreas_.size();

  for (int i = 0; i < np; ++i) {
    PlacementArea &placementArea = placementAreas_[i];

    if (placementArea.area == area)
      placementArea.area = 0;
  }
}

void
CQTileArea::
insertRows(int row, int nrows)
{
  // save old rows and cols
  Grid grid = grid_;

  // calc new cell grid
  int nrows1 = grid_.nrows + nrows;
  int ncols1 = std::max(grid_.ncols, 1);

  grid_.setSize(nrows1, ncols1);

  // populate new grid with old either side of the new rows
  for (int r = 0; r < nrows1; ++r) {
    for (int c = 0; c < ncols1; ++c) {
      int &cell = grid_.cell(r, c);

      if      (r < row)
        cell = grid.cell(r, c);
      else if (r > row)
        cell = grid.cell(r - nrows, c);
      else
        cell = -1;
    }
  }

  // ensure no areas are split
  if (row > 0 && row < nrows1 - 1) {
    for (int c = 0; c < ncols1; ++c) {
      if (grid_.cell(row - 1, c) == grid_.cell(row + 1, c))
        grid_.cell(row, c) = grid_.cell(row - 1, c);
    }
  }
}

void
CQTileArea::
insertColumns(int col, int ncols)
{
  // save old rows and cols
  Grid grid = grid_;

  // calc new cell grid
  int nrows1 = std::max(grid_.nrows, 1);
  int ncols1 = grid_.ncols + ncols;

  grid_.setSize(nrows1, ncols1);

  // populate new grid with old either side of the new colums
  for (int r = 0; r < nrows1; ++r) {
    for (int c = 0; c < ncols1; ++c) {
      int &cell = grid_.cell(r, c);

      if      (c < col)
        cell = grid.cell(r, c);
      else if (c > col)
        cell = grid.cell(r, c - ncols);
      else
        cell = -1;
    }
  }

  // ensure no areas are split
  if (col > 0 && col < ncols1 - 1) {
    for (int r = 0; r < nrows1; ++r) {
      if (grid_.cell(r, col - 1) == grid_.cell(r, col + 1))
        grid_.cell(r, col) = grid_.cell(r, col - 1);
    }
  }
}

void
CQTileArea::
removeWindowArea(CQTileWindowArea *window)
{
  // reset cells for this window area to zero
  int ncells = grid_.nrows*grid_.ncols;

  for (int ci = 0; ci < ncells; ++ci)
    if (grid_.cells[ci] == window->id())
      grid_.cells[ci] = -1;

  if (isVisible()) {
    // remove empty cells and cleanup duplicate rows and columns
    fillEmptyCells();

    removeDuplicateCells();

    //------

    // update placement
    gridToPlacement();

    adjustToFit();
  }

  if (CQTileAreaConstants::debug_grid)
    grid_.print(std::cerr);
}

void
CQTileArea::
replaceWindowArea(CQTileWindowArea *oldArea, CQTileWindowArea *newArea)
{
  // reset cells for this window area to zero
  int ncells = grid_.nrows*grid_.ncols;

  for (int ci = 0; ci < ncells; ++ci) {
    if (grid_.cells[ci] == oldArea->id())
      grid_.cells[ci] = newArea->id();
  }

  int pid = getPlacementAreaIndex(oldArea->id());

  PlacementArea &placementArea = placementAreas_[pid];

  placementArea.area = newArea;

  updatePlacementGeometry(placementArea);
}

void
CQTileArea::
fillEmptyCells()
{
  // refill empty cells
  // ensure no L shapes
  for (int r = 0; r < grid_.nrows; ++r) {
    for (int c = 0; c < grid_.ncols; ++c) {
      if (grid_.cell(r, c) >= 0) continue;

      // (r, c) is zero to pick a surrounding non-zero cell to use
      int lCell = (c > 0               ? grid_.cell(r    , c - 1) : -1);
      int rCell = (c < grid_.ncols - 1 ? grid_.cell(r    , c + 1) : -1);
      int tCell = (r > 0               ? grid_.cell(r - 1, c    ) : -1);
      int bCell = (r < grid_.nrows - 1 ? grid_.cell(r + 1, c    ) : -1);

      int tlCell = (c >               0 && r > 0               ? grid_.cell(r - 1, c - 1) : -1);
      int trCell = (c < grid_.ncols - 1 && r > 0               ? grid_.cell(r - 1, c + 1) : -1);
      int blCell = (c > 0               && r < grid_.nrows - 1 ? grid_.cell(r + 1, c - 1) : -1);
      int brCell = (c < grid_.ncols - 1 && r < grid_.nrows - 1 ? grid_.cell(r + 1, c + 1) : -1);

      if      (lCell >= 0 && lCell != blCell && lCell != tlCell) grid_.cell(r, c) = lCell;
      else if (rCell >= 0 && rCell != brCell && rCell != trCell) grid_.cell(r, c) = rCell;
      else if (tCell >= 0 && tCell != trCell && tCell != tlCell) grid_.cell(r, c) = tCell;
      else if (bCell >= 0 && bCell != brCell && bCell != blCell) grid_.cell(r, c) = bCell;
    }
  }
}

void
CQTileArea::
removeDuplicateCells()
{
  removeDuplicateRows();
  removeDuplicateCols();
}

void
CQTileArea::
removeDuplicateRows()
{
  std::set<int> rows;

  for (int r = 1; r < grid_.nrows; ++r) {
    bool match = true;

    for (int c = 0; c < grid_.ncols; ++c) {
      if (grid_.cell(r - 1, c) != grid_.cell(r, c)) {
        match = false;
        break;
      }
    }

    if (match)
      rows.insert(r);
  }

  if (rows.empty())
    return;

  Grid grid(grid_.nrows - rows.size(), grid_.ncols);

  for (int r = 0, r1 = 0; r < grid_.nrows; ++r) {
    if (rows.find(r) != rows.end()) continue;

    for (int c = 0; c < grid_.ncols; ++c)
      grid.cell(r1, c) = grid_.cell(r, c);

    ++r1;
  }

  grid_ = grid;
}

void
CQTileArea::
removeDuplicateCols()
{
  std::set<int> cols;

  for (int c = 1; c < grid_.ncols; ++c) {
    bool match = true;

    for (int r = 0; r < grid_.nrows; ++r) {
      if (grid_.cell(r, c - 1) != grid_.cell(r, c)) {
        match = false;
        break;
      }
    }

    if (match)
      cols.insert(c);
  }

  if (cols.empty())
    return;

  Grid grid(grid_.nrows, grid_.ncols - cols.size());

  for (int c = 0, c1 = 0; c < grid_.ncols; ++c) {
    if (cols.find(c) != cols.end()) continue;

    for (int r = 0; r < grid_.nrows; ++r)
      grid.cell(r, c1) = grid_.cell(r, c);

    ++c1;
  }

  grid_ = grid;
}

void
CQTileArea::
gridToPlacement()
{
  // TODO: use existing placement to drive new placement ????

  placementAreas_.clear();

  Grid grid = grid_;

  int ncells = grid_.nrows*grid_.ncols;

  for (int ci = 0; ci < ncells; ++ci) {
    if (grid.cells[ci] < 0)
      continue;

    int id = grid.cells[ci];

    // get extent (nrow,ncols) of area with specified id
    int r1 = ci / grid_.ncols;
    int c1 = ci % grid_.ncols;

    int r2 = r1;
    int c2 = c1;

    while (r2 + 1 < grid_.nrows && grid.cell(r2 + 1, c1) == id)
      ++r2;

    while (c2 + 1 < grid_.ncols && grid.cell(r1, c2 + 1) == id)
      ++c2;

    int nr = r2 - r1 + 1;
    int nc = c2 - c1 + 1;

    // clear cells with this id
    for (int r = r1; r <= r2; ++r) {
      for (int c = c1; c <= c2; ++c) {
        grid.cell(r, c) = -1;
      }
    }

    // get area
    CQTileWindowArea *area = 0;

    if (id) {
      area = areas_[id];
      assert(area);
    }

    // create placement area for this area
    PlacementArea placementArea;

    placementArea.place(r1, c1, nr, nc, area);

    placementAreas_.push_back(placementArea);
  }

  addSplitters();
}

void
CQTileArea::
addSplitters()
{
  hsplitters_.clear();
  vsplitters_.clear();

  int np = placementAreas_.size();

  for (int i = 0; i < np; ++i) {
    PlacementArea &placementArea = placementAreas_[i];

    int row1 = placementArea.row1();
    int row2 = placementArea.row2();
    int col1 = placementArea.col1();
    int col2 = placementArea.col2();

    // add splitter for each edge
    if (row1 > 0          ) addHSplitter(i, row1, true ); // top
    if (row2 < grid_.nrows) addHSplitter(i, row2, false); // bottom
    if (col1 > 0          ) addVSplitter(i, col1, true ); // left
    if (col2 < grid_.ncols) addVSplitter(i, col2, false); // right
  }

  //------

  // ensure no intersect of horizontal splitters with vertical ones
  for (RowHSplitterArray::iterator p = hsplitters_.begin(); p != hsplitters_.end(); ++p) {
    int row = (*p).first;

    HSplitterArray &hsplitters = (*p).second;

    int ns = hsplitters.size();

    for (int i = 0; i < ns; ++i) {
      HSplitter &hsplitter = hsplitters[i];

      intersectVSplitter(row, hsplitter);
    }
  }

  //------

  updatePlacementGeometries();
}

void
CQTileArea::
updatePlacementGeometries()
{
  int np = placementAreas_.size();

  for (int i = 0; i < np; ++i) {
    PlacementArea &placementArea = placementAreas_[i];

    updatePlacementGeometry(placementArea);
  }
}

void
CQTileArea::
updatePlacementGeometry(PlacementArea &placementArea)
{
  CQTileWindowArea *area = placementArea.area;

  if (area) {
    area->setParent(this);

    area->move  (placementArea.x1 (), placementArea.y1  ());
    area->resize(placementArea.width, placementArea.height);

    area->show();
  }
}

void
CQTileArea::
addHSplitter(int i, int row, bool top)
{
  PlacementArea &placementArea = placementAreas_[i];

  int ss = CQTileAreaConstants::splitter_size;

  // adjust size for splitter
  if (top) { placementArea.y      += ss/2; placementArea.height -= ss/2; }
  else     { placementArea.height -= ss/2; }

  // get column range
  int col1 = placementArea.col1();
  int col2 = placementArea.col2();

  // check overlap with existing splitters for this row
  int is;

  if (getHSplitter(row, col1, col2, is)) {
    HSplitter &splitter = hsplitters_[row][is];

    // handle exact overlap
    if (col1 == splitter.col1 && col2 == splitter.col2) {
      if (top)
        splitter.bareas.push_back(i);
      else
        splitter.tareas.push_back(i);

      return;
    }

    // handle join
    if (CQTileAreaConstants::combine_splitters) {
      splitter.col1 = std::min(splitter.col1, col1);
      splitter.col2 = std::max(splitter.col2, col2);

      if (top)
        splitter.bareas.push_back(i);
      else
        splitter.tareas.push_back(i);

      return;
    }
  }

  HSplitter splitter;

  splitter.col1 = col1;
  splitter.col2 = col2;

  if (top)
    splitter.bareas.push_back(i);
  else
    splitter.tareas.push_back(i);

  hsplitters_[row].push_back(splitter);
}

void
CQTileArea::
addVSplitter(int i, int col, bool left)
{
  PlacementArea &placementArea = placementAreas_[i];

  int ss = CQTileAreaConstants::splitter_size;

  // adjust size for splitter
  if (left) { placementArea.x     += ss/2; placementArea.width -= ss/2; }
  else      { placementArea.width -= ss/2; }

  // get row range
  int row1 = placementArea.row1();
  int row2 = placementArea.row2();

  // check overlap with existing splitters for this column
  int is;

  if (getVSplitter(col, row1, row2, is)) {
    VSplitter &splitter = vsplitters_[col][is];

    // handle exact overlap
    if (row1 == splitter.row1 && row2 == splitter.row2) {
      if (left)
        splitter.rareas.push_back(i);
      else
        splitter.lareas.push_back(i);

      return;
    }

    // handle join
    if (CQTileAreaConstants::combine_splitters) {
      splitter.row1 = std::min(splitter.row1, row1);
      splitter.row2 = std::max(splitter.row2, row2);

      if (left)
        splitter.rareas.push_back(i);
      else
        splitter.lareas.push_back(i);

      return;
    }
  }

  VSplitter splitter;

  splitter.row1 = row1;
  splitter.row2 = row2;

  if (left)
    splitter.rareas.push_back(i);
  else
    splitter.lareas.push_back(i);

  vsplitters_[col].push_back(splitter);
}

void
CQTileArea::
intersectVSplitter(int row, const HSplitter &hsplitter)
{
  for (ColVSplitterArray::iterator p = vsplitters_.begin(); p != vsplitters_.end(); ++p) {
    std::vector<VSplitter> newVSplitters;

    int col = (*p).first;

    if (col <= hsplitter.col1 || col >= hsplitter.col2) continue;

    VSplitterArray &vsplitters = (*p).second;

    int ns = vsplitters.size();

    for (int i = 0; i < ns; ++i) {
      VSplitter &vsplitter = vsplitters[i];

      if (row <= vsplitter.row1 || row >= vsplitter.row2) continue;

      // split
      std::vector<int> lareas = vsplitter.lareas;
      std::vector<int> rareas = vsplitter.rareas;

      vsplitter.lareas.clear();
      vsplitter.rareas.clear();

      VSplitter vsplitter1 = vsplitter;

      vsplitter .row2 = row;
      vsplitter1.row1 = row;

      for (uint j = 0; j < lareas.size(); ++j) {
        const PlacementArea &placementArea = placementAreas_[lareas[j]];

        if (placementArea.row1() < row)
          vsplitter .lareas.push_back(lareas[j]);
        else
          vsplitter1.lareas.push_back(lareas[j]);
      }

      for (uint j = 0; j < rareas.size(); ++j) {
        const PlacementArea &placementArea = placementAreas_[rareas[j]];

        if (placementArea.row1() < row)
          vsplitter .rareas.push_back(rareas[j]);
        else
          vsplitter1.rareas.push_back(rareas[j]);
      }

      newVSplitters.push_back(vsplitter1);
    }

    for (uint i = 0; i < newVSplitters.size(); ++i)
      vsplitters_[col].push_back(newVSplitters[i]);
  }
}

void
CQTileArea::
adjustToFit()
{
  std::vector<bool> widthSized;
  std::vector<bool> heightSized;

  int np = placementAreas_.size();

  widthSized .resize(np);
  heightSized.resize(np);

  for (int i = 0; i < np; ++i) {
    widthSized [i] = false;
    heightSized[i] = false;
  }

  //------

  int b = 2;
  int w = width () - 2*b;
  int h = height() - 2*b;

  int ss = CQTileAreaConstants::splitter_size;

  // adjust areas to fit width
  for (int r = 0; r < grid_.nrows; ++r) {
    std::vector<int> pids;

    // get list of placement area ids on this row
    int c = 0;

    while (c < grid_.ncols) {
      int cell = grid_.cell(r, c);

      ++c;

      while (c < grid_.ncols && grid_.cell(r, c) == cell)
        ++c;

      int pid = getPlacementAreaIndex(cell);

      if (pid >= 0)
        pids.push_back(pid);
    }

    // calc remaining space on row (width - placement area widths)
    int w1 = w;
    int ns = 0;

    for (uint i = 0; i < pids.size(); ++i) {
      PlacementArea &placementArea = placementAreas_[pids[i]];

      w1 -= placementArea.width;

      // only resize those not already sized
      if (! widthSized[pids[i]])
        ++ns;
    }

    // account for spacers
    w1 -= (pids.size() - 1)*ss;

    // resize placement areas
    int x = b;

    for (uint i = 0; i < pids.size(); ++i) {
      PlacementArea &placementArea = placementAreas_[pids[i]];

      // adjust width (if not done already)
      if (! widthSized[pids[i]]) {
        int dw = w1/ns;

        if (CQTileAreaConstants::rand_resize) {
          if ((w1 % ns) > 0 && rand() > RAND_MAX/2)
            ++dw;
        }

        --ns;

        placementArea.width += dw;

        w1 -= dw;

        widthSized[pids[i]] = true;
      }

      // place at x and update x
      placementArea.x = x;

      x = placementArea.x2() + ss;

      updatePlacementGeometry(placementArea);
    }
  }

  // adjust areas to fit height
  for (int c = 0; c < grid_.ncols; ++c) {
    std::vector<int> pids;

    // get list of placement area ids on this column
    int r = 0;

    while (r < grid_.nrows) {
      int cell = grid_.cell(r, c);

      ++r;

      while (r < grid_.nrows && grid_.cell(r, c) == cell)
        ++r;

      int pid = getPlacementAreaIndex(cell);

      if (pid >= 0)
        pids.push_back(pid);
    }

    // calc remaining space on column (width - placement area widths)
    int h1 = h;
    int ns = 0;

    for (uint i = 0; i < pids.size(); ++i) {
      PlacementArea &placementArea = placementAreas_[pids[i]];

      h1 -= placementArea.height;

      // only resize those not already sized
      if (! heightSized[pids[i]])
        ++ns;
    }

    // account for spacers
    h1 -= (pids.size() - 1)*ss;

    // resize placement areas
    int y = b;

    for (uint i = 0; i < pids.size(); ++i) {
      PlacementArea &placementArea = placementAreas_[pids[i]];

      // adjust height (if not done already)
      if (! heightSized[pids[i]]) {
        int dh = h1/ns;

        if (CQTileAreaConstants::rand_resize) {
          if ((h1 % ns) > 0 && rand() > RAND_MAX/2)
            ++dh;
        }

        --ns;

        placementArea.height += dh;

        h1 -= dh;

        heightSized[pids[i]] = true;
      }

      // place at y and update y
      placementArea.y = y;

      y = placementArea.y2() + ss;

      updatePlacementGeometry(placementArea);
    }
  }
}

CQTileWindow *
CQTileArea::
currentWindow() const
{
  if (! currentArea()) {
    if (! areas_.empty())
      return (*areas_.begin()).second->currentWindow();
    else
      return 0;
  }

  if (currentArea())
    return currentArea()->currentWindow();
  else
    return 0;
}

void
CQTileArea::
setCurrentWindow(CQTileWindow *window)
{
  for (WindowAreas::const_iterator p = areas_.begin(); p != areas_.end(); ++p) {
    CQTileWindowArea *area = (*p).second;

    area->setCurrentWindow(window);
  }
}

void
CQTileArea::
setCurrentArea(CQTileWindowArea *area)
{
  currentArea_ = area;

  for (WindowAreas::const_iterator p = areas_.begin(); p != areas_.end(); ++p) {
    CQTileWindowArea *area = (*p).second;

    area->update();
  }
}

int
CQTileArea::
getPlacementAreaIndex(int id) const
{
  int np = placementAreas_.size();

  for (int i = 0; i < np; ++i) {
    const PlacementArea &placementArea = placementAreas_[i];

    if      (placementArea.area && placementArea.area->id() == id)
      return i;
    else if (! placementArea.area && ! id)
      return i;
  }

  return -1;
}

CQTileWindowArea *
CQTileArea::
getAreaAt(int row, int col) const
{
  int cell = grid_.cell(row, col);

  int ind = getPlacementAreaIndex(cell);

  if (ind >= 0)
    return placementAreas_[ind].area;
  else
    return 0;
}

bool
CQTileArea::
getHSplitter(int row, int col1, int col2, int &is)
{
  is = -1;

  RowHSplitterArray::iterator p = hsplitters_.find(row);

  if (p == hsplitters_.end())
    return false;

  HSplitterArray &splitters = (*p).second;

  int ns = splitters.size();

  for (int i = 0; i < ns; ++i) {
    HSplitter &splitter1 = splitters[i];

    if (col1 > splitter1.col2 || col2 < splitter1.col1) continue;

    is = i;

    if (col1 == splitter1.col1 && col2 == splitter1.col2)
      return true;
  }

  return (is >= 0);
}

bool
CQTileArea::
getVSplitter(int col, int row1, int row2, int &is)
{
  is = -1;

  ColVSplitterArray::iterator p = vsplitters_.find(col);

  if (p == vsplitters_.end())
    return false;

  VSplitterArray &splitters = (*p).second;

  int ns = splitters.size();

  for (int i = 0; i < ns; ++i) {
    VSplitter &splitter1 = splitters[i];

    if (row1 > splitter1.row2 || row2 < splitter1.row1) continue;

    is = i;

    if (row1 == splitter1.row1 && row2 == splitter1.row2)
      return true;
  }

  return (is >= 0);
}

QRect
CQTileArea::
getHSplitterRect(const HSplitter &splitter) const
{
  int ss = CQTileAreaConstants::splitter_size;

  int x1 = INT_MAX, x2 = INT_MIN, yt = INT_MIN, yb = INT_MAX;

  int nl = splitter.tareas.size();

  for (int i = 0; i < nl; ++i) {
    int area = splitter.tareas[i];

    const PlacementArea &placementArea = placementAreas_[area];

    yt = std::max(yt, placementArea.y2());

    x1 = std::min(x1, placementArea.x1());
    x2 = std::max(x2, placementArea.x2());
  }

  int nb = splitter.bareas.size();

  for (int i = 0; i < nb; ++i) {
    int area = splitter.bareas[i];

    const PlacementArea &placementArea = placementAreas_[area];

    yb = std::min(yb, placementArea.y1());

    x1 = std::min(x1, placementArea.x1());
    x2 = std::max(x2, placementArea.x2());
  }

  if (yt == INT_MIN) yt = yb - ss;
  if (yb == INT_MAX) yb = yt + ss;

  return QRect(x1, (yt + yb)/2 - ss/2, x2 - x1 + 1, ss);
}

QRect
CQTileArea::
getVSplitterRect(const VSplitter &splitter) const
{
  int ss = CQTileAreaConstants::splitter_size;

  int y1 = INT_MAX, y2 = INT_MIN, xl = INT_MIN, xr = INT_MAX;

  int nl = splitter.lareas.size();

  for (int i = 0; i < nl; ++i) {
    int area = splitter.lareas[i];

    const PlacementArea &placementArea = placementAreas_[area];

    xl = std::max(xl, placementArea.x2());

    y1 = std::min(y1, placementArea.y1());
    y2 = std::max(y2, placementArea.y2());
  }

  int nr = splitter.rareas.size();

  for (int i = 0; i < nr; ++i) {
    int area = splitter.rareas[i];

    const PlacementArea &placementArea = placementAreas_[area];

    xr = std::min(xr, placementArea.x1());

    y1 = std::min(y1, placementArea.y1());
    y2 = std::max(y2, placementArea.y2());
  }

  if (xl == INT_MIN) xl = xr - ss;
  if (xr == INT_MAX) xr = xl + ss;

  return QRect((xl + xr)/2 - ss/2, y1, ss, y2 - y1 + 1);
}

void
CQTileArea::
maximizeWindows()
{
  if (maximized_) {
    restoreState(maximizeState_);

    for (WindowAreas::const_iterator p = areas_.begin(); p != areas_.end(); ++p) {
      CQTileWindowArea *area = (*p).second;

      area->setMaximized(false);
    }

    maximized_ = false;
  }
  else {
    if (areas_.size() == 1)
      return;

    saveState(maximizeState_, false);

    maximized_ = true;

    // save all windows
    std::vector<CQTileWindow *> windows;

    for (WindowAreas::const_iterator p = areas_.begin(); p != areas_.end(); ++p) {
      CQTileWindowArea *area = (*p).second;

      const CQTileWindowArea::Windows &areaWindows = area->getWindows();

      std::copy(areaWindows.begin(), areaWindows.end(), std::back_inserter(windows));
    }

    // reparent so not deleted
    for (std::vector<CQTileWindow *>::iterator p = windows.begin(); p != windows.end(); ++p)
      (*p)->setParent(this);

    // delete old areas (TODO: save one ?)
    for (WindowAreas::const_iterator p = areas_.begin(); p != areas_.end(); ++p)
      delete (*p).second;

    // reset
    areas_.clear();
    grid_ .reset();

    setCurrentArea(0);

    // create new area
    CQTileWindowArea *windowArea = addArea();

    // add windows to new area (as tabs)
    for (std::vector<CQTileWindow *>::iterator p = windows.begin(); p != windows.end(); ++p) {
      CQTileWindow *window = *p;

      windowArea->addWindow(window);
    }

    addWindowArea(windowArea, 0, 0, 1, 1);

    windowArea->setMaximized(true);
  }
}

void
CQTileArea::
tileWindows()
{
  // save all windows
  std::vector<CQTileWindow *> windows;

  for (WindowAreas::const_iterator p = areas_.begin(); p != areas_.end(); ++p) {
    CQTileWindowArea *area = (*p).second;

    const CQTileWindowArea::Windows &areaWindows = area->getWindows();

    std::copy(areaWindows.begin(), areaWindows.end(), std::back_inserter(windows));
  }

  // reparent so not deleted
  for (std::vector<CQTileWindow *>::iterator p = windows.begin(); p != windows.end(); ++p)
    (*p)->setParent(this);

  // delete old areas
  for (WindowAreas::const_iterator p = areas_.begin(); p != areas_.end(); ++p)
    delete (*p).second;

  // reset
  areas_.clear();
  grid_ .reset();

  setCurrentArea(0);

  // determine grid size
  int nrows = int(sqrt(windows.size()) + 0.5);
  int ncols = windows.size() / nrows;

  if (windows.size() % nrows) ++nrows;

  grid_.setSize(nrows, ncols);

  // create new areas (one per window)
  int r = 0, c = 0;

  for (std::vector<CQTileWindow *>::iterator p = windows.begin(); p != windows.end(); ++p) {
    CQTileWindowArea *windowArea = addArea();

    CQTileWindow *window = *p;

    windowArea->addWindow(window);

    grid_.cell(r, c) = windowArea->id();

    ++c;

    if (c >= ncols) {
      ++r;

      c = 0;
    }
  }

  if (isVisible()) {
    // remove empty cells and cleanup duplicate rows and columns
    fillEmptyCells();

    removeDuplicateCells();

    //------

    // update placement
    gridToPlacement();

    adjustToFit();
  }

  if (CQTileAreaConstants::debug_grid)
    grid_.print(std::cerr);
}

void
CQTileArea::
showEvent(QShowEvent *)
{
  fillEmptyCells();

  removeDuplicateCells();

  //----

  gridToPlacement();

  adjustToFit();
}

void
CQTileArea::
resizeEvent(QResizeEvent *)
{
  adjustToFit();
}

void
CQTileArea::
paintEvent(QPaintEvent *)
{
  QStylePainter ps(this);

  //ps.fillRect(rect(), palette().color(QPalette::Window));
  //ps.fillRect(rect(), QBrush(QColor(200,250,200)));

  for (RowHSplitterArray::iterator p = hsplitters_.begin(); p != hsplitters_.end(); ++p) {
    HSplitterArray &splitters = (*p).second;

    int ns = splitters.size();

    for (int i = 0; i < ns; ++i) {
      HSplitter &splitter = splitters[i];

      splitter.rect = getHSplitterRect(splitter);

      QStyleOption opt;

      opt.initFrom(this);

      opt.rect  = splitter.rect;
      opt.state = QStyle::State_None;

      if ((*p).first == mouseState_.pressHSplitter.first && i == mouseState_.pressHSplitter.second)
        opt.state |= QStyle::State_Sunken;
      if ((*p).first == mouseState_.mouseHSplitter.first && i == mouseState_.mouseHSplitter.second)
        opt.state |= QStyle::State_MouseOver;

      ps.drawControl(QStyle::CE_Splitter, opt);
    }
  }

  for (ColVSplitterArray::iterator p = vsplitters_.begin(); p != vsplitters_.end(); ++p) {
    VSplitterArray &splitters = (*p).second;

    int ns = splitters.size();

    for (int i = 0; i < ns; ++i) {
      VSplitter &splitter = splitters[i];

      splitter.rect = getVSplitterRect(splitter);

      QStyleOption opt;

      opt.initFrom(this);

      opt.rect  = splitter.rect;
      opt.state = QStyle::State_Horizontal;

      if ((*p).first == mouseState_.pressVSplitter.first && i == mouseState_.pressVSplitter.second)
        opt.state |= QStyle::State_Sunken;
      if ((*p).first == mouseState_.mouseVSplitter.first && i == mouseState_.mouseVSplitter.second)
        opt.state |= QStyle::State_MouseOver;

      ps.drawControl(QStyle::CE_Splitter, opt);
    }
  }
}

void
CQTileArea::
updateRubberBand()
{
  QPoint tl = mapToGlobal(QPoint(0, 0));

  if (! CQTileAreaConstants::animated_preview) {
    if (highlight_.side != NO_SIDE) {
      int ss = CQTileAreaConstants::splitter_size;

      QRect rect;

      if      (highlight_.ind != -1) {
        const PlacementArea &area = placementAreas_[highlight_.ind];

        if      (highlight_.side == LEFT_SIDE)
          rect = QRect(area.x1() - ss/2, area.y1(), ss/2, area.y2() - area.y1());
        else if (highlight_.side == RIGHT_SIDE)
          rect = QRect(area.x2(), area.y1(), ss/2, area.y2() - area.y1());
        else if (highlight_.side == TOP_SIDE)
          rect = QRect(area.x1(), area.y1() - ss/2, area.x2() - area.x1(), ss/2);
        else if (highlight_.side == BOTTOM_SIDE)
          rect = QRect(area.x1(), area.y2(), area.x2() - area.x1(), ss/2);
        else if (highlight_.side == MIDDLE_SIDE)
          rect = QRect(area.x1(), area.y1(), area.x2() - area.x1(),  area.y2() - area.y1());
      }
      else
        rect = QRect(0, 0, width(), height());

      rubberBand_->setGeometry(rect.adjusted(tl.x(), tl.y(), tl.x(), tl.y()));

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
      PlacementArea &placementArea = placementAreas_[pid];

      rubberBand_->setGeometry(placementArea.rect().adjusted(tl.x(), tl.y(), tl.x(), tl.y()));

      rubberBand_->show();
    }
    else
      hideRubberBand();
  }
}

void
CQTileArea::
hideRubberBand()
{
  rubberBand_->hide();
}

void
CQTileArea::
mousePressEvent(QMouseEvent *e)
{
  mouseState_.pressed        = true;
  mouseState_.pressHSplitter = getHSplitterAtPos(e->pos());
  mouseState_.pressVSplitter = getVSplitterAtPos(e->pos());
  mouseState_.pressPos       = e->globalPos();

  update();
}

void
CQTileArea::
mouseMoveEvent(QMouseEvent *e)
{
  if (mouseState_.pressed) {
    int dx = e->globalPos().x() - mouseState_.pressPos.x();
    int dy = e->globalPos().y() - mouseState_.pressPos.y();

    if (dy && mouseState_.pressHSplitter.first >= 0) {
      HSplitter &splitter =
        hsplitters_[mouseState_.pressHSplitter.first][mouseState_.pressHSplitter.second];

      int nt = splitter.tareas.size();
      int nb = splitter.bareas.size();

      // limit dy
      for (int i = 0; i < nt; ++i) {
        int area = splitter.tareas[i];

        PlacementArea &placementArea = placementAreas_[area];

        if (dy < 0 && placementArea.height + dy <= CQTileAreaConstants::min_size)
          dy = CQTileAreaConstants::min_size - placementArea.height;
      }

      for (int i = 0; i < nb; ++i) {
        int area = splitter.bareas[i];

        PlacementArea &placementArea = placementAreas_[area];

        if (dy > 0 && placementArea.height - dy <= CQTileAreaConstants::min_size)
          dy = placementArea.height - CQTileAreaConstants::min_size;
      }

      // apply dy to placement areas
      for (int i = 0; i < nt; ++i) {
        int area = splitter.tareas[i];

        PlacementArea &placementArea = placementAreas_[area];

        placementArea.height += dy;

        updatePlacementGeometry(placementArea);
      }

      for (int i = 0; i < nb; ++i) {
        int area = splitter.bareas[i];

        PlacementArea &placementArea = placementAreas_[area];

        placementArea.y      += dy;
        placementArea.height -= dy;

        updatePlacementGeometry(placementArea);
      }
    }

    if (dx && mouseState_.pressVSplitter.first >= 0) {
      VSplitter &splitter =
        vsplitters_[mouseState_.pressVSplitter.first][mouseState_.pressVSplitter.second];

      int nl = splitter.lareas.size();
      int nr = splitter.rareas.size();

      // limit dx
      for (int i = 0; i < nl; ++i) {
        int area = splitter.lareas[i];

        PlacementArea &placementArea = placementAreas_[area];

        if (dx < 0 && placementArea.width + dx <= CQTileAreaConstants::min_size)
          dx = CQTileAreaConstants::min_size - placementArea.width;
      }

      for (int i = 0; i < nr; ++i) {
        int area = splitter.rareas[i];

        PlacementArea &placementArea = placementAreas_[area];

        if (dx > 0 && placementArea.width - dx <= CQTileAreaConstants::min_size)
          dx = placementArea.width - CQTileAreaConstants::min_size;
      }

      // apply dx to placement areas
      for (int i = 0; i < nl; ++i) {
        int area = splitter.lareas[i];

        PlacementArea &placementArea = placementAreas_[area];

        placementArea.width += dx;

        updatePlacementGeometry(placementArea);
      }

      for (int i = 0; i < nr; ++i) {
        int area = splitter.rareas[i];

        PlacementArea &placementArea = placementAreas_[area];

        placementArea.x     += dx;
        placementArea.width -= dx;

        updatePlacementGeometry(placementArea);
      }
    }

    mouseState_.pressPos = e->globalPos();

    update();
  }
  else {
    SplitterInd mouseHSplitter = getHSplitterAtPos(e->pos());
    SplitterInd mouseVSplitter = getVSplitterAtPos(e->pos());

    if (mouseHSplitter != mouseState_.mouseHSplitter ||
        mouseVSplitter != mouseState_.mouseVSplitter) {
      mouseState_.mouseHSplitter = mouseHSplitter;
      mouseState_.mouseVSplitter = mouseVSplitter;

      if      (mouseHSplitter.first != -1)
        setCursor(Qt::SplitVCursor);
      else if (mouseVSplitter.first != -1)
        setCursor(Qt::SplitHCursor);
      else
        setCursor(Qt::ArrowCursor);

      update();
    }
  }
}

void
CQTileArea::
mouseReleaseEvent(QMouseEvent *)
{
  mouseState_.reset();

  update();
}

CQTileArea::SplitterInd
CQTileArea::
getHSplitterAtPos(const QPoint &pos) const
{
  for (RowHSplitterArray::const_iterator p = hsplitters_.begin(); p != hsplitters_.end(); ++p) {
    const HSplitterArray &splitters = (*p).second;

    int ns = splitters.size();

    for (int i = 0; i < ns; ++i) {
      const HSplitter &splitter = splitters[i];

      if (splitter.rect.contains(pos))
        return SplitterInd((*p).first, i);
    }
  }

  return SplitterInd(-1,-1);
}

CQTileArea::SplitterInd
CQTileArea::
getVSplitterAtPos(const QPoint &pos) const
{
  for (ColVSplitterArray::const_iterator p = vsplitters_.begin(); p != vsplitters_.end(); ++p) {
    const VSplitterArray &splitters = (*p).second;

    int ns = splitters.size();

    for (int i = 0; i < ns; ++i) {
      const VSplitter &splitter = splitters[i];

      if (splitter.rect.contains(pos))
        return SplitterInd((*p).first, i);
    }
  }

  return SplitterInd(-1,-1);
}

void
CQTileArea::
setHighlight(const QPoint &pos)
{
  highlight_ = Highlight(-1, NO_SIDE);

  QPoint tl = mapToGlobal(QPoint(          0,            0));
  QPoint br = mapToGlobal(QPoint(width() - 1, height() - 1));

  if (pos.x() < tl.x() || pos.x() > br.x() || pos.y() < tl.y() || pos.y() > br.y()) {
    update();
    return;
  }

  int  minD    = INT_MAX;
  int  minI    = -1     ;
  Side minSide = LEFT_SIDE;

  int np = placementAreas_.size();

  for (int i = 0; i < np; ++i) {
    const PlacementArea &placementArea = placementAreas_[i];

    int dx1 = abs(tl.x() + placementArea.x1() - pos.x());
    int dy1 = abs(tl.y() + placementArea.y1() - pos.y());
    int dx2 = abs(tl.x() + placementArea.x2() - pos.x());
    int dy2 = abs(tl.y() + placementArea.y2() - pos.y());

    int  d;
    Side side = NO_SIDE;

    if      (pos.x() < tl.x() + placementArea.x1()) {
      if      (pos.y() < tl.y() + placementArea.y1()) {
        d    = sqrt(dx1*dx1 + dy1*dy1);
        side = (dx1 < dy1 ? LEFT_SIDE : TOP_SIDE);
      }
      else if (pos.y() > tl.y() + placementArea.y2()) {
        d    = sqrt(dx1*dx1 + dy2*dy2);
        side = (dx1 < dy1 ? LEFT_SIDE : BOTTOM_SIDE);
      }
      else {
        d    = dx1;
        side = LEFT_SIDE;
      }
    }
    else if (pos.x() > tl.x() + placementArea.x2()) {
      if      (pos.y() < tl.y() + placementArea.y1()) {
        d    = sqrt(dx2*dx2 + dy1*dy1);
        side = (dx1 < dy1 ? RIGHT_SIDE : TOP_SIDE);
      }
      else if (pos.y() > tl.y() + placementArea.y2()) {
        d    = sqrt(dx2*dx2 + dy2*dy2);
        side = (dx1 < dy1 ? RIGHT_SIDE : BOTTOM_SIDE);
      }
      else {
        d    = dx2;
        side = RIGHT_SIDE;
      }
    }
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
        else if (dx2 == d) side = BOTTOM_SIDE;
      }
    }

    if (d < minD) { minD = d; minI = i; minSide = side; }

    int dxm = abs(tl.x() + placementArea.xm() - pos.x());
    int dym = abs(tl.y() + placementArea.ym() - pos.y());

    d = sqrt(dxm*dxm + dym*dym);

    if (d < minD) { minD = d; minI = i; minSide = MIDDLE_SIDE; }
  }

  highlight_ = Highlight(minI, minSide);

  if (! CQTileAreaConstants::animated_preview)
    updateRubberBand();
}

void
CQTileArea::
clearHighlight()
{
  highlight_.ind  = -1;
  highlight_.side = NO_SIDE;

  if (! CQTileAreaConstants::animated_preview)
    hideRubberBand();
}

bool
CQTileArea::
getHighlightPos(Side &side, int &row1, int &col1, int &row2, int &col2)
{
  if (highlight_.side == NO_SIDE) return false;

  if (highlight_.ind != -1) {
    side = highlight_.side;

    const PlacementArea &area = placementAreas_[highlight_.ind];

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

void
CQTileArea::
saveState(bool transient)
{
  saveState(saveState_, transient);
}

void
CQTileArea::
saveState(PlacementState &state, bool transient)
{
  state.transient_      = transient;
  state.grid_           = grid_;
  state.placementAreas_ = placementAreas_;
  state.hsplitters_     = hsplitters_;
  state.vsplitters_     = vsplitters_;

  if (! transient) {
    for (uint i = 0; i < placementAreas_.size(); ++i) {
      CQTileWindowArea *area = placementAreas_[i].area;

      const CQTileWindowArea::Windows &areaWindows = area->getWindows();

      state.placementAreas_[i].windows = areaWindows;
    }
  }
}

void
CQTileArea::
restoreState()
{
  restoreState(saveState_);
}

void
CQTileArea::
restoreState(const PlacementState &state)
{
  grid_           = state.grid_;
  placementAreas_ = state.placementAreas_;
  hsplitters_     = state.hsplitters_;
  vsplitters_     = state.vsplitters_;

  if (! state.transient_) {
    for (uint i = 0; i < placementAreas_.size(); ++i) {
      PlacementArea &area = placementAreas_[i];

      // reparent windows so not deleted
      for (Windows::iterator p = area.windows.begin(); p != area.windows.end(); ++p)
        (*p)->setParent(this);
    }

    // save old areas and clear areas
    WindowAreas areas = areas_;

    areas_.clear();

    setCurrentArea(0);

    // create new areas for placement areas
    for (uint i = 0; i < placementAreas_.size(); ++i) {
      PlacementArea &area = placementAreas_[i];

      CQTileWindowArea *newArea = addArea();

      // reparent windows so not deleted
      for (Windows::iterator p = area.windows.begin(); p != area.windows.end(); ++p)
        newArea->addWindow(*p);

      // replace old id in grid with new id
      grid_.replace(placementAreas_[i].area->id(), newArea->id());

      // update placement area id
      placementAreas_[i].area = newArea;
    }

    /// delete old areas
    for (WindowAreas::const_iterator p = areas.begin(); p != areas.end(); ++p)
      delete (*p).second;

    if (CQTileAreaConstants::debug_grid)
      grid_.print(std::cerr);
  }

  updatePlacementGeometries();
}

//-------

int CQTileWindowArea::lastId_;

CQTileWindowArea::
CQTileWindowArea(CQTileArea *area) :
 area_(area), detached_(false), floating_(false), maximized_(false)
{
  setCursor(Qt::ArrowCursor);

  id_ = ++lastId_;

  setFrameStyle(QFrame::Panel | QFrame::Raised);
  setLineWidth(1);

  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setMargin(0); layout->setSpacing(0);

  title_  = new CQTileWindowTitle(this);
  stack_  = new QStackedWidget;
  tabBar_ = new QTabBar;

  tabBar_->setMovable(true);

  layout->addWidget(title_);
  layout->addWidget(stack_);
  layout->addWidget(tabBar_);

  connect(tabBar_, SIGNAL(currentChanged(int)), this, SLOT(tabChangedSlot(int)));
}

CQTileWindow *
CQTileWindowArea::
addWidget(QWidget *w)
{
  CQTileWindow *window = new CQTileWindow;

  window->setWidget(w);

  addWindow(window);

  return window;
}

void
CQTileWindowArea::
addWindow(CQTileWindow *window)
{
  stack_->addWidget(window);

  windows_.push_back(window);

  int ind = tabBar_->addTab(window->widget()->windowTitle());

  tabBar_->setTabData(ind, ind);

  tabBar_->setVisible(tabBar_->count() > 1);
}

QString
CQTileWindowArea::
getTitle() const
{
  CQTileWindow *window = currentWindow();

  return (window ? window->widget()->windowTitle() : "");
}

void
CQTileWindowArea::
detach(const QPoint &pos)
{
  // remove window area from grid and display as floating window
  assert(! detached_ && ! floating_);

  //----

  // save state
  //area_->saveState();

  //-----

  QPoint lpos = mapFromGlobal(pos);

  // remove window from window area
  if (CQTileAreaConstants::detach_tab && tabBar_->count() > 1) {
    // create new window area for non-current tabs
    CQTileWindowArea *windowArea = area_->addArea();

    //----

    // get current window and add all other windows to new area
    CQTileWindow *currentWindow = this->currentWindow();

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
    setParent(0, Qt::FramelessWindowHint);

    move(pos - lpos);

    show();

    area_->replaceWindowArea(this, windowArea);

    detached_ = true;
  }
  // detach whole window area
  else {
    setParent(0, Qt::FramelessWindowHint);

    move(pos - lpos);

    show();

    area_->removeWindowArea(this);

    detached_ = true;
  }
}

void
CQTileWindowArea::
attach(bool preview)
{
  CQTileArea::Side side;
  int              row1, col1, row2, col2;

  bool rc = area_->getHighlightPos(side, row1, col1, row2, col2);
  assert(rc);

  if (! preview) {
    setParent(area_);

    show();
  }

  CQTileWindowArea *attachArea = (! preview ? this : 0);

  if      (side == CQTileArea::TOP_SIDE || side == CQTileArea::BOTTOM_SIDE) {
    area_->insertRows(row1, 1);

    area_->addWindowArea(attachArea, row1, col1, 1, col2 - col1);
  }
  else if (side == CQTileArea::LEFT_SIDE || side == CQTileArea::RIGHT_SIDE) {
    area_->insertColumns(col1, 1);

    area_->addWindowArea(attachArea, row1, col1, row2 - row1, 1);
  }
  else {
    if (! preview) {
      CQTileWindowArea *area = area_->getAreaAt(row1, col1);

      int ind = tabBar_->currentIndex() + area->tabBar_->count();

      for (Windows::iterator p = windows_.begin(); p != windows_.end(); ++p)
        area->addWindow(*p);

      area->tabBar_->setCurrentIndex(ind);

      area_->removeArea(this);

      deleteLater();
    }
  }

  if (! preview) {
    detached_ = false;
    floating_ = false;
  }
}

void
CQTileWindowArea::
reattach()
{
  setParent(area_);

  show();

  //area_->restoreState();
}

void
CQTileWindowArea::
setFloating()
{
  // set as standaline window

  setWindowFlags(Qt::Window);

  if (! floating_) {
    show();

    detached_ = false;
    floating_ = true;
  }
}

void
CQTileWindowArea::
setMaximized(bool maximized)
{
  maximized_ = maximized;

  title_->setMaximized(maximized_);
}

CQTileWindow *
CQTileWindowArea::
currentWindow() const
{
  QWidget *w = stack_->widget(stack_->currentIndex());

  return qobject_cast<CQTileWindow *>(w);
}

void
CQTileWindowArea::
setCurrentWindow(CQTileWindow *window)
{
  int i = 0;

  for (Windows::iterator p = windows_.begin(); p != windows_.end(); ++p) {
    if (*p == window) {
      stack_->setCurrentIndex(stack_->indexOf(window));

      tabBar_->setCurrentIndex(i);

      return;
    }

    ++i;
  }
}

void
CQTileWindowArea::
tabChangedSlot(int tabNum)
{
  int ind = tabBar_->tabData(tabNum).toInt();

  stack_->setCurrentIndex(ind);

  title_->update();
}

void
CQTileWindowArea::
minimizeSlot()
{
  CQTileArea *area = this->area();

  area->tileWindows();
}

void
CQTileWindowArea::
maximizeSlot()
{
  CQTileArea *area = this->area();

  CQTileWindow *window = currentWindow();

  area->maximizeWindows();

  area->setCurrentWindow(window);
}

void
CQTileWindowArea::
closeSlot()
{
}

void
CQTileWindowArea::
paintEvent(QPaintEvent *e)
{
  //QPainter p(this);
  //p.fillRect(rect(), QBrush(QColor(250,200,200)));

  QFrame::paintEvent(e);
}

//-------

CQTileWindowTitle::
CQTileWindowTitle(CQTileWindowArea *area) :
 area_(area), maximized_(false)
{
  minimizeButton_ = addButton(QPixmap(minimize_data));
  maximizeButton_ = addButton(QPixmap(maximize_data));
  closeButton_    = addButton(QPixmap(close_data   ));

  connect(minimizeButton_, SIGNAL(clicked()), area, SLOT(minimizeSlot()));
  connect(maximizeButton_, SIGNAL(clicked()), area, SLOT(maximizeSlot()));
  connect(closeButton_   , SIGNAL(clicked()), area, SLOT(closeSlot()));

  setAttribute(Qt::WA_Hover);
}

void
CQTileWindowTitle::
setMaximized(bool maximized)
{
  maximized_ = maximized;

  maximizeButton_->setIcon(maximized_ ? QPixmap(restore_data) : QPixmap(maximize_data));
}

QColor
CQTileWindowTitle::
backgroundColor() const
{
  bool current = (area_ == area_->area()->currentArea());

  return (! current ? QColor(160,160,160) : QColor(120,120,160));
}

QColor
CQTileWindowTitle::
barColor() const
{
  bool current = (area_ == area_->area()->currentArea());

  return (! current ? QColor(120,120,120) : QColor(80,80,80));
}

void
CQTileWindowTitle::
mousePressEvent(QMouseEvent *e)
{
  mouseState_.pressed  = true;
  mouseState_.pressPos = e->globalPos();

  if (CQTileAreaConstants::animated_preview)
    area_->area()->saveState();

  area_->area()->setCurrentArea(area_);
}

void
CQTileWindowTitle::
mouseMoveEvent(QMouseEvent *e)
{
  if (! mouseState_.pressed || mouseState_.escapePress) return;

  if (! area_->isDetached() && ! area_->isFloating()) {
    area_->detach(e->globalPos());

    if (CQTileAreaConstants::animated_preview)
      area_->area()->saveState();
  }

  int dx = e->globalPos().x() - mouseState_.pressPos.x();
  int dy = e->globalPos().y() - mouseState_.pressPos.y();

  area_->move(area_->pos() + QPoint(dx, dy));

  mouseState_.pressPos = e->globalPos();

  if (CQTileAreaConstants::animated_preview)
    area_->area()->restoreState();

  area_->area()->setHighlight(mouseState_.pressPos);

  if (CQTileAreaConstants::animated_preview) {
    CQTileArea::Side side;
    int              row1, col1, row2, col2;

    if (area_->area()->getHighlightPos(side, row1, col1, row2, col2))
      area_->attach(true);

    area_->area()->updateRubberBand();
  }

  area_->area()->update();
}

void
CQTileWindowTitle::
mouseReleaseEvent(QMouseEvent *)
{
  if (CQTileAreaConstants::animated_preview) {
    area_->area()->restoreState();

    area_->area()->hideRubberBand();
  }

  if (! mouseState_.escapePress) {
    CQTileArea::Side side;
    int              row1, col1, row2, col2;

    if (area_->area()->getHighlightPos(side, row1, col1, row2, col2))
      area_->attach(false);
  //else
  //  area_->setFloating();

    area_->area()->clearHighlight();
  }

  mouseState_.reset();
}

void
CQTileWindowTitle::
keyPressEvent(QKeyEvent *e)
{
  if (e->key() == Qt::Key_Escape) {
    if (mouseState_.pressed && area_->isDetached()) {
      area_->reattach();

      mouseState_.escapePress = true;

      area_->area()->clearHighlight();
    }
  }
}

bool
CQTileWindowTitle::
event(QEvent *e)
{
  switch (e->type()) {
    case QEvent::HoverEnter:
      setCursor(Qt::SizeAllCursor);
      break;
    case QEvent::HoverLeave:
      unsetCursor();
      break;
    default:
      break;
  }

  return QWidget::event(e);
}

//-------

CQTileWindow::
CQTileWindow() :
 w_(0)
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setMargin(0); layout->setSpacing(0);
}

void
CQTileWindow::
setWidget(QWidget *w)
{
  w_ = w;

  layout()->addWidget(w);
}
