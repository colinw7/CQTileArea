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
  CTileGrid(int nrows=0, int ncols=0) :
   nrows_(0), ncols_(0) {
    setSize(nrows, ncols);
  }

  int nrows() const { return nrows_; }
  int ncols() const { return ncols_; }

  int &cell(int r, int c) {
    return cells_[r*ncols_ + c];
  }

  int cell(int r, int c) const {
    return cells_[r*ncols_ + c];
  }

  void reset() {
    nrows_ = 0;
    ncols_ = 0;

    cells_.clear();
  }

  void setSize(int nrows, int ncols) {
    nrows_ = nrows;
    ncols_ = ncols;

    cells_.resize(nrows_*ncols_);
  }

  void replace(int oldId, int newId) {
    for (int i = 0; i < nrows_*ncols_; ++i) {
      if (cells_[i] == oldId)
        cells_[i] = newId;
    }
  }

  void fill(int r1, int c1, int r2, int c2, int id) {
    for (int r = r1; r <= r2; ++r)
      for (int c = c1; c <= c2; ++c)
        cell(r, c) = id;
  }

  void insertRows   (int row, int nrows);
  void insertColumns(int col, int ncols);

  bool isValid() const;

  bool fillEmptyCells();

  bool fillLeft  (int c, int r1, int r2);
  bool fillRight (int c, int r1, int r2);
  bool fillTop   (int c, int r1, int r2);
  bool fillBottom(int c, int r1, int r2);

  bool testFillEmpty(int r, int c, int id);

  void removeDuplicateRows();
  void removeDuplicateCols();

  bool getRegion(int id, int r1, int c1, int *nr, int *nc);

  void print(std::ostream &os);

 private:
  typedef std::vector<int> Cells;

  Cells cells_;
  int   nrows_;
  int   ncols_;
};

#endif
