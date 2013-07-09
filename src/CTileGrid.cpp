#include <CTileGrid.h>
#include <set>

// add new rows after specified row
void
CTileGrid::
insertRows(int row, int nrows)
{
  // save old rows and cols
  CTileGrid grid = *this;

  // calc new cell grid
  int nrows1 = nrows_ + nrows;
  int ncols1 = std::max(ncols_, 1);

  setSize(nrows1, ncols1);

  clear();

  // populate new grid with old either side of the new rows
  for (int r = 0; r < nrows1; ++r) {
    for (int c = 0; c < ncols1; ++c) {
      int &pc = cell(r, c);

      if      (r < row)
        pc = grid.cell(r, c);
      else if (r > row)
        pc = grid.cell(r - nrows, c);
    }
  }

  // ensure no areas are split
  if (row > 0 && row < nrows1 - 1) {
    for (int c = 0; c < ncols1; ++c) {
      if (cell(row - 1, c) == cell(row + 1, c))
        cell(row, c) = cell(row - 1, c);
    }
  }
}

// add new columns after specified column
void
CTileGrid::
insertColumns(int col, int ncols)
{
  // save old rows and cols
  CTileGrid grid = *this;

  // calc new cell grid
  int nrows1 = std::max(nrows_, 1);
  int ncols1 = ncols_ + ncols;

  setSize(nrows1, ncols1);

  clear();

  // populate new grid with old either side of the new columns
  for (int r = 0; r < nrows1; ++r) {
    for (int c = 0; c < ncols1; ++c) {
      int &pc = cell(r, c);

      if      (c < col)
        pc = grid.cell(r, c);
      else if (c > col)
        pc = grid.cell(r, c - ncols);
    }
  }

  // ensure no areas are split
  if (col > 0 && col < ncols1 - 1) {
    for (int r = 0; r < nrows1; ++r) {
      if (cell(r, col - 1) == cell(r, col + 1))
        cell(r, col) = cell(r, col - 1);
    }
  }
}

// expand occupied cells to fill empty ones
bool
CTileGrid::
fillEmptyCells()
{
  // TODO: remove empty rows/cols

  // save original grid
  CTileGrid grid = *this;

  int ncells = nrows()*ncols();

  bool failed = false;

  while (! failed) {
    // find empty cell
    int i = 0;

    for ( ; i < ncells; ++i)
      if (cells_[i] < 0)
        break;

    if (i >= ncells)
      break;

    int r = i / ncols();
    int c = i % ncols();

    int id = cell(r, c);

    // get empty cell region
    int nr, nc;

    getRegion(id, r, c, &nr, &nc);

    int r2 = r + nr - 1;
    int c2 = c + nc - 1;

    // fill based on major direction
    if (nr > nc) {
      if (r  > 0          && fillTop   (r  - 1, c, c2)) { /* print(std::cerr); */ continue; }
      if (r2 < nrows_ - 1 && fillBottom(r2 + 1, c, c2)) { /* print(std::cerr); */ continue; }
      if (c  > 0          && fillLeft  (c  - 1, r, r2)) { /* print(std::cerr); */ continue; }
      if (c2 < ncols_ - 1 && fillRight (c2 + 1, r, r2)) { /* print(std::cerr); */ continue; }
    }
    else {
      if (c  > 0          && fillLeft  (c  - 1, r, r2)) { /* print(std::cerr); */ continue; }
      if (c2 < ncols_ - 1 && fillRight (c2 + 1, r, r2)) { /* print(std::cerr); */ continue; }
      if (r  > 0          && fillTop   (r  - 1, c, c2)) { /* print(std::cerr); */ continue; }
      if (r2 < nrows_ - 1 && fillBottom(r2 + 1, c, c2)) { /* print(std::cerr); */ continue; }
    }

    // no change so fail
    failed = true;
  }

  // if failed restore original grid
  if (failed) {
    print(std::cerr);

    *this = grid;
  }

  return failed;
}

// fill columns to left of column and row range
bool
CTileGrid::
fillLeft(int c, int r1, int r2)
{
  if (r1 > 0          && cell(r1 - 1, c) == cell(r1, c)) return false;
  if (r2 < nrows_ - 1 && cell(r2 + 1, c) == cell(r2, c)) return false;

  for (int r = r1; r <= r2; ++r)
    cell(r, c + 1) = cell(r, c);

  return true;
}

// fill columns to right of column and row range
bool
CTileGrid::
fillRight(int c, int r1, int r2)
{
  if (r1 > 0          && cell(r1 - 1, c) == cell(r1, c)) return false;
  if (r2 < nrows_ - 1 && cell(r2 + 1, c) == cell(r2, c)) return false;

  for (int r = r1; r <= r2; ++r)
    cell(r, c - 1) = cell(r, c);

  return true;
}

// fill columns above row and column range
bool
CTileGrid::
fillTop(int r, int c1, int c2)
{
  if (c1 > 0          && cell(r, c1 - 1) == cell(r, c1)) return false;
  if (c2 < ncols_ - 1 && cell(r, c2 + 1) == cell(r, c2)) return false;

  for (int c = c1; c <= c2; ++c)
    cell(r + 1, c) = cell(r, c);

  return true;
}

// fill columns below row and column range
bool
CTileGrid::
fillBottom(int r, int c1, int c2)
{
  if (c1 > 0          && cell(r, c1 - 1) == cell(r, c1)) return false;
  if (c2 < ncols_ - 1 && cell(r, c2 + 1) == cell(r, c2)) return false;

  for (int c = c1; c <= c2; ++c)
    cell(r - 1, c) = cell(r, c);

  return true;
}

// remove duplicate rows
void
CTileGrid::
removeDuplicateRows()
{
  std::set<int> rows;

  for (int r = 1; r < nrows_; ++r) {
    bool match = true;

    for (int c = 0; c < ncols_; ++c) {
      if (cell(r - 1, c) != cell(r, c)) {
        match = false;
        break;
      }
    }

    if (match)
      rows.insert(r);
  }

  if (rows.empty())
    return;

  CTileGrid grid(nrows_ - rows.size(), ncols_);

  for (int r = 0, r1 = 0; r < nrows_; ++r) {
    if (rows.find(r) != rows.end()) continue;

    for (int c = 0; c < ncols_; ++c)
      grid.cell(r1, c) = cell(r, c);

    ++r1;
  }

  *this = grid;
}

// remove duplicate columns
void
CTileGrid::
removeDuplicateCols()
{
  std::set<int> cols;

  for (int c = 1; c < ncols_; ++c) {
    bool match = true;

    for (int r = 0; r < nrows_; ++r) {
      if (cell(r, c - 1) != cell(r, c)) {
        match = false;
        break;
      }
    }

    if (match)
      cols.insert(c);
  }

  if (cols.empty())
    return;

  CTileGrid grid(nrows_, ncols_ - cols.size());

  for (int c = 0, c1 = 0; c < ncols_; ++c) {
    if (cols.find(c) != cols.end()) continue;

    for (int r = 0; r < nrows_; ++r)
      grid.cell(r, c1) = cell(r, c);

    ++c1;
  }

  *this = grid;
}

bool
CTileGrid::
isValid() const
{
  // grid is valid if square shapes and max of one region per cell value
  CTileGrid grid1 = *this;

  std::set<int> used;

  for (int r = 0; r < grid1.nrows_; ++r) {
    for (int c = 0; c < grid1.ncols_; ++c) {
      int id = grid1.cell(r, c);
      if (id < 0) continue;

      if (used.find(id) != used.end())
        return false;

      used.insert(id);

      int nr, nc;

      bool rc = grid1.getRegion(id, r, c, &nr, &nc);

      if (! rc)
        return false;

      int r2 = r + nr - 1;
      int c2 = c + nc - 1;

      grid1.fill(r, c, r2, c2, -2);
    }
  }

  return true;
}

// get region for specified id
bool
CTileGrid::
getRegion(int id, int r1, int c1, int *nr, int *nc)
{
  // get extent (nrows,ncols) of area with specified id
  int r2 = r1;
  int c2 = c1;

  while (r2 + 1 < nrows_ && cell(r2 + 1, c1) == id)
    ++r2;

  while (c2 + 1 < ncols_ && cell(r1, c2 + 1) == id)
    ++c2;

  for (int r = r1; r <= r2; ++r) {
    for (int c = c1; c <= c2; ++c) {
      if (cell(r, c) != id)
        return false;
    }
  }

  *nr = r2 - r1 + 1;
  *nc = c2 - c1 + 1;

  return true;
}

void
CTileGrid::
print(std::ostream &os)
{
  // display new grid
  if (! isValid())
    std::cerr << "Invalid" << std::endl;

  for (int r = 0; r < nrows_; ++r) {
    for (int c = 0; c < ncols_; ++c) {
      int id = cell(r, c);

      if (id >= 0)
        os << " " << id;
      else
        os << " .";
    }

    os << std::endl;
  }

  os << std::endl;
}
