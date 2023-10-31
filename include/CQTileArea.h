#ifndef CQTileArea_H
#define CQTileArea_H

#include <CTileGrid.h>

#include <QWidget>
#include <QPointer>

#include <map>
#include <set>

class CQTileWindowArea;
class CQTileWindow;
class CQTileWindowTabBar;
class CQTileWindowTitle;
class CQTileStackedWidget;
class CQTileAreaSplitter;
class CQTileAreaMenuIcon;
class CQTileAreaMenuControls;

class CQWidgetResizer;
class CQRubberBand;

class QMainWindow;
class QMenu;
class QGridLayout;

//! class to tile a set of windows in a Qt Main Window
//! tile is set out in a grid with splitters (resize bars) separating each
//! set of grid cells
class CQTileArea : public QWidget {
  Q_OBJECT

  Q_PROPERTY(int    border             READ border             WRITE setBorder            )
  Q_PROPERTY(int    splitterSize       READ splitterSize       WRITE setSplitterSize      )
  Q_PROPERTY(bool   animateDrag        READ animateDrag        WRITE setAnimateDrag       )
  Q_PROPERTY(QColor titleActiveColor   READ titleActiveColor   WRITE setTitleActiveColor  )
  Q_PROPERTY(QColor titleInactiveColor READ titleInactiveColor WRITE setTitleInactiveColor)

 public:
  //! side enum
  enum Side {
    NO_SIDE,
    LEFT_SIDE,
    RIGHT_SIDE,
    TOP_SIDE,
    BOTTOM_SIDE,
    MIDDLE_SIDE
  };

 private:
  using Windows = std::vector<CQTileWindow *>;

  //! structure to store physical placement area of area
  struct PlacementArea {
    int     row     { 0 };  //!< row
    int     col     { 0 };  //!< column
    int     nrows   { 1 };  //!< number of rows
    int     ncols   { 1 };  //!< number of columns
    int     areaId  { -1 }; //!< associated area (can be invalid (<= 0))
    Windows windows;        //!< associated area windows (used for saving)
    int     x       { 0 };  //!< x
    int     y       { 0 };  //!< y
    int     width   { 1 };  //!< width
    int     height  { 1 };  //!< height

    int row1() const { return row        ; }
    int col1() const { return col        ; }
    int row2() const { return row + nrows; }
    int col2() const { return col + ncols; }

    int x1() const { return x         ; }
    int y1() const { return y         ; }
    int x2() const { return x + width ; }
    int y2() const { return y + height; }

    int xm() const { return x + width /2; }
    int ym() const { return y + height/2; }

    void place(int r, int c, int nr, int nc, int id) {
      row    = r;
      col    = c;
      nrows  = nr;
      ncols  = nc;
      areaId = id;
      x      = col*100;
      y      = row*100;
      width  = 100*nc;
      height = 100*nr;
    }

    QRect rect() const { return QRect(x, y, width, height); }
  };

  using AreaSet = std::set<int>;

  //! structure to store horizontal splitter geometry
  struct HSplitter {
    int      col1 { -1 };       //!< min col extend of splitter (needed ?)
    int      col2 { -1 };       //!< max col extend of splitter (needed ?)
    AreaSet  tareas;            //!< areas above splitter
    AreaSet  bareas;            //!< areas below splitter
    int      splitterId { -1 }; //!< splitter widget id

    HSplitter() { }
  };

  //! structure to store vertical splitter geometry
  struct VSplitter {
    int      row1 { -1 };       //!< min row extend of splitter (needed ?)
    int      row2 { -1 };       //!< max row extend of splitter (needed ?)
    AreaSet  lareas;            //!< areas left of splitter
    AreaSet  rareas;            //!< areas right of splitter
    int      splitterId { -1 }; //!< splitter widget id

    VSplitter() { }
  };

  //! current highlight
  struct Highlight {
    int  ind  { -1 };      //!< highlight cell ind
    Side side { NO_SIDE }; //!< highlight side

    Highlight() { }

    Highlight(int ind, Side side) :
     ind(ind), side(side) {
    }
  };

  using WindowAreas       = std::map<int, CQTileWindowArea *>;
  using PlacementAreas    = std::vector<PlacementArea>;
  using HSplitterArray    = std::vector<HSplitter>;
  using RowHSplitterArray = std::map<int, HSplitterArray>;
  using VSplitterArray    = std::vector<VSplitter>;
  using ColVSplitterArray = std::map<int, VSplitterArray>;
  using SplitterInd       = std::pair<int, int>;

 public:
  //! structure for (saved) placement state
  struct PlacementState {
    bool              valid_     { false }; //!< is valid
    bool              transient_ { true };  //!< is transient state
    CTileGrid         grid_;                //!< saved grid
    PlacementAreas    placementAreas_;      //!< saved placement areas
    RowHSplitterArray hsplitters_;          //!< saved hsplitters
    ColVSplitterArray vsplitters_;          //!< saved vsplitter

    PlacementState() {
      reset();
    }

    void reset() {
      valid_     = false;
      transient_ = true;
    }
  };

 public:
  //! create tile area
  CQTileArea(QMainWindow *window);

  //! destroy tile area
 ~CQTileArea();

  //! get/set border
  int  border() const { return border_; }
  void setBorder(int border) { border_ = border; }

  //! get/set splitter size
  int  splitterSize() const { return splitterSize_; }
  void setSplitterSize(int size) { splitterSize_ = size; }

  //! get/set drag animation enabled
  bool animateDrag() const { return animateDrag_; }
  void setAnimateDrag(bool animate) { animateDrag_ = animate; }

  //! get/set title active color
  QColor titleActiveColor   () const { return titleActiveColor_; }
  void   setTitleActiveColor(const QColor &color);

  //! get/set title inactive color
  QColor titleInactiveColor   () const { return titleInactiveColor_; }
  void   setTitleInactiveColor(const QColor &color);

  //! add window for widget
  CQTileWindow *addWindow(QWidget *w);
  //! add window for widget at specified grid location
  CQTileWindow *addWindow(QWidget *w, int row, int col, int nrows=1, int ncols=1);

  //! remove window
  void removeWindow(CQTileWindow *window);

  //! get current window
  CQTileWindow *currentWindow() const;

  //! set current window
  void setCurrentWindow(CQTileWindow *window);

  //! is full screen display
  bool isFullScreen() const;

  //! get number of windows
  int getNumWindows() const;

  //! get all windows
  Windows getAllWindows() const;

  //! get placement area for area
  PlacementArea &getPlacementAreaForArea(CQTileWindowArea *area);

  //! emit current window changed signal
  void emitCurrentWindowChanged();

  //! set layout to specified grid (for testing)
  void setGrid(int nrows, int ncols, const std::vector<int> &cells);

 private:
  friend class CQTileWindowArea;
  friend class CQTileWindowTabBar;
  friend class CQTileWindowTitle;
  friend class CQTileAreaSplitter;
  friend class CQTileAreaMenuIcon;
  friend class CQTileAreaMenuControls;

  //! add new area
  CQTileWindowArea *addArea();

  //! window window area at specified grid position
  void addWindowArea(CQTileWindowArea *windowArea, int row, int col, int nrows=1, int ncols=1);

  //! calc best location for new window area
  void calcBestWindowArea(int &row, int &col, int &nrows, int &ncols);

  //! get current area
  CQTileWindowArea *currentArea() const { return currentArea_; }

  //! set current area
  void setCurrentArea(CQTileWindowArea *area);

  //! remove area
  void removeArea(CQTileWindowArea *area);

  //! add specified number of rows at specified row number
  void insertRows   (int row, int rows);
  //! add specified number of columns at specified column number
  void insertColumns(int col, int cols);

  //! detach window area
  void detachWindowArea(CQTileWindowArea *window);
  //! replace window area
  void replaceWindowArea(CQTileWindowArea *oldWindow, CQTileWindowArea *newWindow);

  //! update current window
  void updateCurrentWindow();

  //! update placement from grid
  void updatePlacement(bool useExisting=true);

  //! fill empty cells with surrounding non-empty cells
  void fillEmptyCells();

  //! remove duplicate rows/columns
  void removeDuplicateCells();

  //! update placement from grid
  void gridToPlacement(bool useExisting=true);

  //! add splitters between cells
  void addSplitters();

  //! combine splitters
  void combineTouchingSplitters();

  //! update all placement geometries
  void updatePlacementGeometries();
  //! update specified placement geometry
  void updatePlacementGeometry(PlacementArea &placementArea);

  //! check for intersect or vertical and horizontal splitters
  void intersectVSplitter(int row, const HSplitter &hsplitter);

  //! add horizontal splitter at specified row (above or below specified placement)
  void addHSplitter(int i, int row, bool top);
  //! add vertical splitter at specified column (left or right of specified placement)
  void addVSplitter(int i, int col, bool left);

  //! adjust sizes of cells to fit geometry
  void adjustToFit();

  //! adjust placements which must have same width to match splitters
  void adjustSameWidthPlacements (PlacementArea &area);
  //! adjust placements which must have same height to match splitters
  void adjustSameHeightPlacements(PlacementArea &area);

  //! update titles
  void updateTitles();

  //! update menu bar widgets
  void updateMenuBar();

  //! get placement area index from id
  int getPlacementAreaIndex(int id) const;
  //! get placement area index from id from list of placements
  int getPlacementAreaIndex(const PlacementAreas &area, int id) const;

  //! get area at specified row/col
  CQTileWindowArea *getAreaAt(int row, int col) const;

  //! get horizontal splitter at specified row and column range
  bool getHSplitter(int row, int col1, int col2, int &is);
  //! get vertical splitter at specified column and row range
  bool getVSplitter(int col, int row1, int row2, int &is);

  //! get horizontal splitter rectangle
  QRect getHSplitterRect(const HSplitter &splitter) const;
  //! get vertical splitter rectangle
  QRect getVSplitterRect(const VSplitter &splitter) const;

  //! is restore state valid
  bool isRestoreStateValid() const;

  //! is maximized
  bool isMaximized() const;

  //! get window area for window
  CQTileWindowArea *getWindowArea(CQTileWindow *window) const;

  //! handle show event
  void showEvent(QShowEvent *) override;

  //! handle resize event
  void resizeEvent(QResizeEvent *) override;

  //! handle paint event
  void paintEvent(QPaintEvent *) override;

  //! move horizontal splitter
  void moveHSplitter(int row, int ind, int dy);
  //! move vertical splitter
  void moveVSplitter(int col, int ind, int dx);

  int createSplitterWidget(Qt::Orientation orient, int pos, int ind);

  CQTileAreaSplitter *getSplitterWidget(int ind) const;

  //! update rubber band rectangle
  QRect updateRubberBand();
  //! hide rubber band
  void hideRubberBand();

  //! get horizontal splitter at point
  SplitterInd getHSplitterAtPos(const QPoint &pos) const;
  //! get vertical splitter at point
  SplitterInd getVSplitterAtPos(const QPoint &pos) const;

  //! get highlight for specified point
  void setHighlight(const QPoint &pos);
  //! clear highlight
  void clearHighlight();

  //! get current highlight position and side
  bool getHighlightPos(Side &side, int &row1, int &col1, int &row2, int &col2);

  //! save state
  void saveState(PlacementState &state, bool transient=true);

  //! restore state
  void restoreState(const PlacementState &state);

  //! set default placement size (size of empty cell)
  void setDefPlacementSize(int w, int h);

  //! get area for placement area id
  CQTileWindowArea *getAreaForId(int areaId) const;

  //! get size hint
  QSize sizeHint() const override;
  //! get minimum size hint
  QSize minimumSizeHint() const override;

  //! maximize windows
  void maximizeWindows();
  //! restore windows
  void restoreWindows();
  //! tile windows
  void tileWindows();

 public Q_SLOTS:
  //! maximize all windows
  void maximizeSlot();
  //! restore windows
  void restoreSlot();
  //! tile all windows
  void tileSlot();
  //! detach current window
  void detachSlot();
  //! close current window
  void closeSlot();

  //! print grid (debug)
  void printSlot();
  //! fill grid (debug)
  void fillSlot();
  //! remove duplicates (debug)
  void dupSlot();
  //! grid to placement (debug)
  void placeSlot();
  //! adjust sizes (debug)
  void adjustSlot();

 signals:
  //! current window changed signal
  void currentWindowChanged(CQTileWindow *);

  //! window closed signal
  void windowClosed(CQTileWindow *);

 private:
  using MenuIconP       = QPointer<CQTileAreaMenuIcon> ;
  using MenuControlsP   = QPointer<CQTileAreaMenuControls>;
  using SplitterWidgets = std::map<int, CQTileAreaSplitter *>;

  QMainWindow*       window_             { nullptr }; //!< parent window
  CTileGrid          grid_;                           //!< layout grid
  bool               animateDrag_        { true};     //!< animate drag
  QColor             titleActiveColor_;               //!< title active color
  QColor             titleInactiveColor_;             //!< title inactive color
  WindowAreas        areas_;                          //!< window areas
  PlacementAreas     placementAreas_;                 //!< placed areas
  RowHSplitterArray  hsplitters_;                     //!< horizontal splitters
  ColVSplitterArray  vsplitters_;                     //!< vertical splitters
  SplitterWidgets    splitterWidgets_;                //!< splitter widgets
  Highlight          highlight_;                      //!< current highlight (for drag)
  PlacementState     restoreState_;                   //!< saved state to restore from maximized
  int                border_             { 0 };       //!< border
  int                splitterSize_       { 3 };       //!< splitter size
  CQRubberBand*      rubberBand_         { nullptr }; //!< rubber band (for drag)
  CQTileWindowArea*  currentArea_        { nullptr }; //!< current window area
  bool               hasControls_        { false };   //!< has menu controls
  MenuIconP          menuIcon_;                       //!< menu bar icon button
  MenuControlsP      menuControls_;                   //!< menu bar controls
  int                defWidth_           { -1 };      //!< default (new) area width
  int                defHeight_          { -1 };      //!< default (new) area height
};

#endif
