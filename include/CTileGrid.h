#ifndef CTileGrid_H
#define CTileGrid_H

#include <vector>
#include <iostream>

// grid with rows x cols cells where each cells has an associated area
// number to represent which window area is contained in it.
// Multiple cells may have same window area to support multiple cell spanning
// TODO: add unique x/y values and reconfigure grid to match (i.e. new x/y values
// are added removed)
// TODO: store real offsets of edges for smooth resize
class CTileGrid {
 public:
  //! create grid
  CTileGrid(int nrows=0, int ncols=0) :
   nrows_(0), ncols_(0) {
    setSize(nrows, ncols);
  }

  //! get number of rows/cols
  int nrows() const { return nrows_; }
  int ncols() const { return ncols_; }

  //! get editable cell at row/column
  int &cell(int r, int c) {
    return cells_[r*ncols_ + c];
  }

  //! get cell at row/column
  int cell(int r, int c) const {
    return cells_[r*ncols_ + c];
  }

  //! get cell at index
  int cell(int ind) const {
    return cells_[ind];
  }

  //! reset to empty
  void reset() {
    nrows_ = 0;
    ncols_ = 0;

    cells_.clear();
  }

  //! set size
  void setSize(int nrows, int ncols) {
    nrows_ = nrows;
    ncols_ = ncols;

    cells_.resize(nrows_*ncols_);
  }

  //! is single cell
  bool isSingleCell() const { return cells_.size() == 1; }

  //! get old index with new index
  void replace(int oldId, int newId) {
    for (int i = 0; i < nrows_*ncols_; ++i) {
      if (cells_[i] == oldId)
        cells_[i] = newId;
    }
  }

  //! clear to index
  void clear(int ind=-1) {
    for (int i = 0; i < nrows_*ncols_; ++i)
      cells_[i] = ind;
  }

  //! fill range with index
  void fill(int r1, int c1, int r2, int c2, int id) {
    for (int r = r1; r <= r2; ++r)
      for (int c = c1; c <= c2; ++c)
        cell(r, c) = id;
  }

  //! insert specified number of rows at row
  void insertRows   (int row, int nrows);
  //! insert specified number of columns at column
  void insertColumns(int col, int ncols);

  //! is valid
  bool isValid() const;

  //! expand occupied cells to fill empty ones
  bool fillEmptyCells();

  //! fill columns to left of column and row range
  bool fillLeft  (int c, int r1, int r2);
  //! fill columns to right of column and row range
  bool fillRight (int c, int r1, int r2);
  //! fill columns above row and column range
  bool fillTop   (int c, int r1, int r2);
  //! fill columns below row and column range
  bool fillBottom(int c, int r1, int r2);

  //! remove duplicate rows
  void removeDuplicateRows();
  //! remove duplicate columns
  void removeDuplicateCols();

  //! get region for specified id
  bool getRegion(int id, int r1, int c1, int *nr, int *nc);

  //! print grid
  void print(std::ostream &os);

 private:
  typedef std::vector<int> Cells;

  Cells cells_; //! grid cells
  int   nrows_; //! number of rows
  int   ncols_; //! number of columns
};

#endif
