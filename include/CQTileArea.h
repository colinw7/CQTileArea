#ifndef CQTileArea_H
#define CQTileArea_H

#include <CTileGrid.h>

#include <QFrame>
#include <QTabBar>

#include <map>
#include <set>

class CQTileWindowArea;
class CQTileWindow;
class CQTileWindowTabBar;
class CQTileWindowTitle;
class CQTileStackedWidget;

class QMenu;
class QGridLayout;
class QRubberBand;

// class to tile a set of windows in a Qt Main Window
// tile is set out in a grid with splitters (resize bars) separating each
// set of grid cells
class CQTileArea : public QWidget {
  Q_OBJECT

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
  typedef std::vector<CQTileWindow *> Windows;

  //! structure to store physical placement area of area
  struct PlacementArea {
    int               row, col;      //! row, column
    int               nrows, ncols;  //! number of rows/columns
    CQTileWindowArea *area;          //! associated area (can be NULL)
    Windows           windows;       //! associated area windows (used for saving)
    int               x, y;          //! position
    int               width, height; //! size

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

    void place(int r, int c, int nr, int nc, CQTileWindowArea *a) {
      row    = r;
      col    = c;
      nrows  = nr;
      ncols  = nc;
      area   = a;
      x      = col*100;
      y      = row*100;
      width  = 100*nc;
      height = 100*nr;
    }

    QRect rect() const { return QRect(x, y, width, height); }
  };

  typedef std::set<int> AreaSet;

  //! structure to store horizontal splitter geometry
  struct HSplitter {
    int     col1, col2; //! col extend of splitter (needed ?)
    AreaSet tareas;     //! areas above splitter
    AreaSet bareas;     //! areas below splitter
    QRect   rect;       //! bounding box or splitter (calc on draw)
  };

  //! structure to store vertical splitter geometry
  struct VSplitter {
    int     row1, row2; //! row extend of splitter (needed ?)
    AreaSet lareas;     //! areas left of splitter
    AreaSet rareas;     //! areas right of splitter
    QRect   rect;       //! bounding box or splitter (calc on draw)
  };

  //! current highlight
  struct Highlight {
    int  ind;  //! highlight cell ind
    Side side; //! highlight side

    Highlight(int ind1=-1, Side side1=NO_SIDE) : ind(ind1), side(side1) { }
  };

  typedef std::map<int,CQTileWindowArea *> WindowAreas;
  typedef std::vector<PlacementArea>       PlacementAreas;
  typedef std::vector<HSplitter>           HSplitterArray;
  typedef std::map<int,HSplitterArray>     RowHSplitterArray;
  typedef std::vector<VSplitter>           VSplitterArray;
  typedef std::map<int,VSplitterArray>     ColVSplitterArray;
  typedef std::pair<int,int>               SplitterInd;

  //! structure for (saved) placement state
  struct PlacementState {
    bool              valid_;          //! is valid
    bool              transient_;      //! is transient state
    CTileGrid         grid_;           //! saved grid
    PlacementAreas    placementAreas_; //! saved placement areas
    RowHSplitterArray hsplitters_;     //! saved hsplitters
    ColVSplitterArray vsplitters_;     //! saved vsplitter

    PlacementState() {
      valid_     = false;
      transient_ = true;
    }
  };

 public:
  //! create tile area
  CQTileArea();

  //! get/set splitter size
  int  splitterSize() const { return splitterSize_; }
  void setSplitterSize(int size) { splitterSize_ = size; }

  //! get/set title active color
  QColor titleActiveColor   () const { return titleActiveColor_; }
  void   setTitleActiveColor(const QColor &color);

  //! get/set title intactive color
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

  //! get all windows
  Windows getAllWindows() const;

  //! set layout to specified grid (for testing)
  void setGrid(int nrows, int ncols, const std::vector<int> &cells);

 private:
  friend class CQTileWindowArea;
  friend class CQTileWindowTabBar;
  friend class CQTileWindowTitle;

  //! add new area
  CQTileWindowArea *addArea();

  //! window window area at specified grid position
  void addWindowArea(CQTileWindowArea *windowArea, int row, int col, int nrows=1, int ncols=1);

  //! calc best location for new window area
  void calcBestWindowArea(int &row, int &col, int &nrows, int &ncols);

  //! get current area
  CQTileWindowArea *currentArea() const { return currentArea_; }

  //! get/set drag animation enabled
  bool animateDrag() const { return animateDrag_; }
  void setAnimateDrag(bool animate) { animateDrag_ = animate; }

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
  void updatePlacement();

  //! fill empty cells with surrounding non-empty cells
  void fillEmptyCells();

  //! remove duplicate rows/columns
  void removeDuplicateCells();

  //! update placement from grid
  void gridToPlacement();

  //! add splitters between cells
  void addSplitters();

  //! combine splitters
  void combineTouchingSplitters();

  //! update all placement geometries
  void updatePlacementGeometries();
  //! update specfied placement geometry
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
  void adjustSameWidthPlacements (const PlacementArea &area);
  //! adjust placements which must have same height to match splitters
  void adjustSameHeightPlacements(const PlacementArea &area);

  //! update titles
  void updateTitles();

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

  //! is maximized
  bool isMaximized() const;

  //! get window area for windoe
  CQTileWindowArea *getWindowArea(CQTileWindow *window) const;

  //! emit current window changed signal
  void emitCurrentWindowChanged();

  //! handle show event
  void showEvent(QShowEvent *);

  //! handle resize event
  void resizeEvent(QResizeEvent *);

  //! handle paint event
  void paintEvent(QPaintEvent *);

  //! mouse mouse events
  void mousePressEvent  (QMouseEvent *e);
  void mouseMoveEvent   (QMouseEvent *e);
  void mouseReleaseEvent(QMouseEvent *e);

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

  //! get size hint
  QSize sizeHint() const;
  //! get minimum size hint
  QSize minimumSizeHint() const;

 public slots:
  //! maximize windows
  void maximizeWindows();
  //! restore windows
  void restoreWindows();
  //! tile windows
  void tileWindows();

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
  //! structure for mouse state
  struct MouseState {
    bool        pressed;        //! mouse pressed
    QPoint      pressPos;       //! mouse press position
    SplitterInd pressHSplitter; //! pressed horizontal splitter
    SplitterInd pressVSplitter; //! pressed vertical splitter
    SplitterInd mouseHSplitter; //! mouse over horizontal splitter
    SplitterInd mouseVSplitter; //! mouse over vertical splitter
    bool        dragAll;        //! drag all tabs

    MouseState() :
     mouseHSplitter(-1,-1), mouseVSplitter(-1,-1), dragAll(false) {
      reset();
    }

    void reset() {
      pressed        = false;
      pressHSplitter = SplitterInd(-1,-1);
      pressVSplitter = SplitterInd(-1,-1);
      dragAll        = false;
    }
  };

  CTileGrid          grid_;               //! layout grid
  bool               animateDrag_;        //! animate drag
  QColor             titleActiveColor_;   //! title active color
  QColor             titleInactiveColor_; //! title inactive color
  WindowAreas        areas_;              //! window areas
  PlacementAreas     placementAreas_;     //! placed areas
  RowHSplitterArray  hsplitters_;         //! horizontal splitters
  ColVSplitterArray  vsplitters_;         //! vertical splitters
  MouseState         mouseState_;         //! mouse state
  Highlight          highlight_;          //! current highlight (for drag)
  PlacementState     maximizeState_;      //! saved state to restore from maximized
  int                splitterSize_;       //! splitter size
  QRubberBand       *rubberBand_;         //! rubber band (for drag)
  CQTileWindowArea  *currentArea_;        //! current window area
  int                defWidth_;           //! default (new) area width
  int                defHeight_;          //! default (new) area height
};

//------

// class for each window area
// a window area can contain one or more tile windows
// multiple windows in a tile area produces a tab widget (customizable)
class CQTileWindowArea : public QFrame {
  Q_OBJECT

 public:
  typedef std::vector<CQTileWindow *> Windows;

 public:
  //! create window area
  CQTileWindowArea(CQTileArea *area);

  //! get parent tile area
  CQTileArea *area() const { return area_; }

  //! get id
  int id() const { return id_; }

  //! get child windows
  const Windows &getWindows() const { return windows_; }

  //! add widget to area
  CQTileWindow *addWidget(QWidget *w);

  //! is currently detached
  bool isDetached() const { return detached_; }
  //! is currently floating
  bool isFloating() const { return floating_; }
  //! is currently docked
  bool isDocked  () const { return ! isDetached() && ! isFloating(); }

  //! get titlw
  QString getTitle() const;

  //! get icon
  QIcon getIcon() const;

  //! get current window
  CQTileWindow *currentWindow() const;
  //! get current window
  void setCurrentWindow(CQTileWindow *window);

  //! has window
  bool hasWindow(CQTileWindow *window) const;

  // size hint
  QSize sizeHint() const;
  // size minimum hint
  QSize minimumSizeHint() const;

 private:
  //! set detached
  void setDetached(bool detached);
  //! set floating
  void setFloating(bool floating);

  //! initialize animate attach
  void initAttach();
  //! cancel animate attach
  void cancelAttach();
  //! start attach animate
  void startAttachPreview();
  //! perform attach animate
  void doAttachPreview();
  //! stop attach animate
  void stopAttachPreview();

  //! detach at specified point
  void detach(const QPoint &pos, bool floating, bool dragAll);

  //! attach
  void attach(bool preview=false);
  //! attach at specified side and grid position
  void attach(CQTileArea::Side side, int row1, int col1, int row2, int col2, bool preview=false);

  //! reattach at original position
  void reattach();

  //! set floated
  void setFloated();

  //! update title
  void updateTitle();

  //! add child window
  void addWindow(CQTileWindow *window);

  //! remove child window
  bool removeWindow(CQTileWindow *window);

  //! create context menu
  QMenu *createContextMenu(QWidget *parent) const;

  //! update context menu
  void updateContextMenu(QMenu *menu);

 public slots:
  //! detach
  void detachSlot();
  //! attach
  void attachSlot();
  //! tile
  void tileSlot();
  //! maximize
  void maximizeSlot();
  //! restore
  void restoreSlot();
  //! close
  void closeSlot();

 private slots:
  //! attach (after timeout)
  void attachPreviewSlot();

  //! handle tab changed
  void tabChangedSlot(int tabNum);

 private:
  typedef CQTileArea::PlacementState PlacementState;

  friend class CQTileArea;
  friend class CQTileWindowTabBar;
  friend class CQTileWindowTitle;

  static int lastId_; //! last area index (incremented on use for unique id)

  struct AttachData {
    QTimer           *timer;      //! attach timer;
    PlacementState    initState;  //! saved (original) state
    PlacementState    state;      //! saved (detached) state
    bool              initDocked; //! initial docked state
    QRect             rect;       //! preview rect
    CQTileArea::Side  side;       //! attach side
    int               row1;       //! attach start row
    int               col1;       //! attach start column
    int               row2;       //! attach end row
    int               col2;       //! attach end column

    AttachData() :
     timer(0), initState(), state(), initDocked(true),
     rect(), side(CQTileArea::NO_SIDE), row1(0), col1(0), row2(0), col2(0) {
    }
  };

  CQTileArea          *area_;       //! parent area
  int                  id_;         //! window id
  CQTileWindowTitle   *title_;      //! title widget
  CQTileStackedWidget *stack_;      //! widget stack
  CQTileWindowTabBar  *tabBar_;     //! tabbar
  Windows              windows_;    //! child window
  bool                 detached_;   //! detached flag
  bool                 floating_;   //! floating flag
  AttachData           attachData_; //! attach data
};

//------

// class to represent a tile window
// a tile window has a titlebar
class CQTileWindow : public QWidget {
  Q_OBJECT

 public:
  //! create window
  CQTileWindow(CQTileWindowArea *area);

  //! destroy window
 ~CQTileWindow();

  //! get parent window area
  CQTileWindowArea *area() const { return area_; }

  //! set child widget
  void setWidget(QWidget *w);
  //! get child widget
  QWidget *widget() const { return w_; }

  //! get title
  QString getTitle() const;

  //! get icon
  QIcon getIcon() const;

 private:
  friend class CQTileArea;
  friend class CQTileWindowArea;

  //! is current valid (not being destroyed)
  bool isValid() const { return valid_; }

  //! set parent window area
  void setArea(CQTileWindowArea *area);

  //! handle hover events
  bool event(QEvent *e);

  //! handle close event
  void closeEvent(QCloseEvent *closeEvent);

 private slots:
  //! handle focus change
  void focusChangedSlot(QWidget *old, QWidget *now);

  //! handle child widget destroyed
  void widgetDestroyed();

 private:
  CQTileWindowArea *area_;  //! parent area
  QWidget          *w_;     //! child widget
  bool              valid_; //! is child widget valid
};

//------

class CQTileWindowTabBar : public QTabBar {
  Q_OBJECT

 public:
  //! create tabbar
  CQTileWindowTabBar(CQTileWindowArea *area);

 private:
  //! handle mouse press
  void mousePressEvent(QMouseEvent *e);

  //! display context menu
  void contextMenuEvent(QContextMenuEvent *e);

 private:
  CQTileWindowArea *area_;        //! parent area
  QMenu            *contextMenu_; //! context menu
};

//------

#include <CQTitleBar.h>

// widget for window area title bar
class CQTileWindowTitle : public CQTitleBar {
  Q_OBJECT

 public:
  //! create title
  CQTileWindowTitle(CQTileWindowArea *area);

  //! update state
  void updateState();

 private:
  //! get title
  QString title() const;
  //! get icon
  QIcon   icon() const;

  //! get background color
  QColor backgroundColor() const;
  //! get bar's color
  QColor barColor() const;

  //! handle mouse events
  void mousePressEvent  (QMouseEvent *e);
  void mouseMoveEvent   (QMouseEvent *e);
  void mouseReleaseEvent(QMouseEvent *e);

  //! handle double click (expand)
  void mouseDoubleClickEvent(QMouseEvent *e);

  //! handle key perss (escape)
  void keyPressEvent(QKeyEvent *e);

  //! display context menu
  void contextMenuEvent(QContextMenuEvent *e);

  //! handle hover
  bool event(QEvent *e);

 private slots:
  //! detach
  void detachSlot();
  //! maximize
  void maximizeSlot();

 private:
  struct MouseState {
    bool   pressed;     //! is mouse pressed
    bool   moved;       //! has move started
    bool   escapePress; //! escape pressed
    QPoint pressPos;    //! press position
    bool   dragAll;     //! drag all tabs

    MouseState(){
      reset();
    }

    void reset() {
      pressed     = false;
      moved       = false;
      escapePress = false;
      dragAll     = false;
    }
  };

  CQTileWindowArea *area_;           //! parent area
  MouseState        mouseState_;     //! current mouse state
  CQTitleBarButton *detachButton_;   //! attach/detach button
  CQTitleBarButton *maximizeButton_; //! maximize/restore button
  CQTitleBarButton *closeButton_;    //! close button
  QMenu            *contextMenu_;    //! context menu
};

//------

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
  QSize sizeHint() const;

  //! minimum size hint
  QSize minimumSizeHint() const;

 private:
  //! handle show
  void showEvent(QShowEvent *);

  //! handle resize
  void resizeEvent(QResizeEvent *);

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
