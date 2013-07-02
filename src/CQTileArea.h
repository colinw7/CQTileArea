#ifndef CQTileArea_H
#define CQTileArea_H

#include <CTileGrid.h>
#include <QFrame>
#include <map>

class QStackedWidget;
class QTabBar;
class QGridLayout;
class QRubberBand;
class CQTileWindowArea;
class CQTileWindow;
class CQTileWindowTitle;

// class to tile a set of windows in a Qt Main Window
// tile is set out in a grid with splitters (resize bars) separating each
// set of grid cells
class CQTileArea : public QWidget {
  Q_OBJECT

 private:
  enum Side {
    NO_SIDE,
    LEFT_SIDE,
    RIGHT_SIDE,
    TOP_SIDE,
    BOTTOM_SIDE,
    MIDDLE_SIDE
  };

  typedef std::vector<CQTileWindow *> Windows;

  struct PlacementArea {
    int               row;
    int               col;
    int               nrows;
    int               ncols;
    CQTileWindowArea *area;
    Windows           windows;
    int               x, y;
    int               width, height;

    int row1() const { return row        ; }
    int col1() const { return col        ; }
    int row2() const { return row + nrows; }
    int col2() const { return col + ncols; }

    int x1() const { return x             ; }
    int y1() const { return y             ; }
    int x2() const { return x + width  - 1; }
    int y2() const { return y + height - 1; }

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

  struct HSplitter {
    int              col1, col2; // col extend of splitter (needed ?)
    std::vector<int> tareas;     // areas above splitter
    std::vector<int> bareas;     // areas below splitter
    QRect            rect;       // bounding box or splitter (calc on draw)
  };

  struct VSplitter {
    int              row1, row2; // row extend of splitter (needed ?)
    std::vector<int> lareas;     // areas left of splitter
    std::vector<int> rareas;     // areas right of splitter
    QRect            rect;       // bounding box or splitter (calc on draw)
  };

  struct Highlight {
    int  ind;
    Side side;

    Highlight(int ind1=-1, Side side1=NO_SIDE) : ind(ind1), side(side1) { }
  };

  typedef std::map<int,CQTileWindowArea *> WindowAreas;
  typedef std::vector<PlacementArea>       PlacementAreas;
  typedef std::vector<HSplitter>           HSplitterArray;
  typedef std::map<int,HSplitterArray>     RowHSplitterArray;
  typedef std::vector<VSplitter>           VSplitterArray;
  typedef std::map<int,VSplitterArray>     ColVSplitterArray;
  typedef std::pair<int,int>               SplitterInd;

  struct PlacementState {
    bool              transient_;
    CTileGrid         grid_;
    PlacementAreas    placementAreas_;
    RowHSplitterArray hsplitters_;
    ColVSplitterArray vsplitters_;
  };

 public:
  CQTileArea();

  CQTileWindow *addWindow(QWidget *w);
  CQTileWindow *addWindow(QWidget *w, int row, int col, int nrows=1, int ncols=1);

  CQTileWindow *currentWindow() const;

  void setCurrentWindow(CQTileWindow *window);

 private:
  friend class CQTileWindowArea;
  friend class CQTileWindowTitle;

  CQTileWindowArea *addArea();

  void addWindowArea(CQTileWindowArea *windowArea, int row, int col, int nrows=1, int ncols=1);

  CQTileWindowArea *currentArea() const { return currentArea_; }

  void setCurrentArea(CQTileWindowArea *area);

  void removeArea(CQTileWindowArea *area);

  void insertRows   (int row, int rows);
  void insertColumns(int col, int cols);

  void removeWindowArea(CQTileWindowArea *window);
  void replaceWindowArea(CQTileWindowArea *oldWindow, CQTileWindowArea *newWindow);

  void fillEmptyCells();

  void removeDuplicateCells();

  void gridToPlacement();

  void addSplitters();

  void updatePlacementGeometries();
  void updatePlacementGeometry(PlacementArea &placementArea);

  void intersectVSplitter(int row, const HSplitter &hsplitter);

  void addHSplitter(int i, int row, bool top);
  void addVSplitter(int i, int col, bool left);

  void adjustToFit();
  void adjustSplitterSides();

  int getPlacementAreaIndex(int id) const;

  CQTileWindowArea *getAreaAt(int row, int col) const;

  bool getHSplitter(int row, int col1, int col2, int &is);
  bool getVSplitter(int col, int row1, int row2, int &is);

  QRect getHSplitterRect(const HSplitter &splitter) const;
  QRect getVSplitterRect(const VSplitter &splitter) const;

  void showEvent(QShowEvent *);

  void resizeEvent(QResizeEvent *);

  void paintEvent(QPaintEvent *);

  void mousePressEvent  (QMouseEvent *e);
  void mouseMoveEvent   (QMouseEvent *e);
  void mouseReleaseEvent(QMouseEvent *e);

  void updateRubberBand();
  void hideRubberBand();

  SplitterInd getHSplitterAtPos(const QPoint &pos) const;
  SplitterInd getVSplitterAtPos(const QPoint &pos) const;

  void setHighlight(const QPoint &pos);
  void clearHighlight();

  bool getHighlightPos(Side &side, int &row1, int &col1, int &row2, int &col2);

  void maximizeWindows();
  void tileWindows();

  void saveState(bool transient=true);
  void saveState(PlacementState &state, bool transient=true);

  void restoreState();
  void restoreState(const PlacementState &state);

 public slots:
  void printSlot();
  void fillSlot();
  void dupSlot();
  void placeSlot();
  void adjustSlot();

 private:
  struct MouseState {
    bool        pressed;
    QPoint      pressPos;
    SplitterInd pressHSplitter;
    SplitterInd pressVSplitter;
    SplitterInd mouseHSplitter;
    SplitterInd mouseVSplitter;

    MouseState() :
     mouseHSplitter(-1,-1), mouseVSplitter(-1,-1) {
      reset();
    }

    void reset() {
      pressed        = false;
      pressHSplitter = SplitterInd(-1,-1);
      pressVSplitter = SplitterInd(-1,-1);
    }
  };

  CTileGrid          grid_;
  WindowAreas        areas_;
  PlacementAreas     placementAreas_;
  RowHSplitterArray  hsplitters_;
  ColVSplitterArray  vsplitters_;
  MouseState         mouseState_;
  Highlight          highlight_;
  PlacementState     saveState_;
  bool               maximized_;
  PlacementState     maximizeState_;
  QRubberBand       *rubberBand_;
  CQTileWindowArea  *currentArea_;
};

// class for each window area
// a window area can contain one or more tile windows
// multiple windows in a tile area produces a tab widget (customizable)
class CQTileWindowArea : public QFrame {
  Q_OBJECT

 public:
  typedef std::vector<CQTileWindow *> Windows;

 public:
  CQTileWindowArea(CQTileArea *area);

  CQTileArea *area() const { return area_; }

  int id() const { return id_; }

  const Windows &getWindows() const { return windows_; }

  CQTileWindow *addWidget(QWidget *w);

  bool isDetached() const { return detached_; }
  bool isFloating() const { return floating_; }

  QString getTitle() const;

  void detach(const QPoint &pos);
  void attach(bool preview=false);
  void reattach();

  void setFloating();

  CQTileWindow *currentWindow() const;

  void setCurrentWindow(CQTileWindow *window);

 private:
  void setMaximized(bool maximized);

  void addWindow(CQTileWindow *window);

  void paintEvent(QPaintEvent *);

  QSize sizeHint() const;
  QSize minimumSizeHint() const;

 public slots:
  void minimizeSlot();
  void maximizeSlot();
  void closeSlot();

 private slots:
  void tabChangedSlot(int tabNum);

 private:
  friend class CQTileArea;

  static int lastId_;

  CQTileArea        *area_;      // parent area
  int                id_;        // window id
  CQTileWindowTitle *title_;     // title widget
  QStackedWidget    *stack_;     // widget stack
  QTabBar           *tabBar_;    // tabbar
  Windows            windows_;   // child window
  bool               detached_;  // detached flag
  bool               floating_;  // floating flag
  bool               maximized_; // maximized
};

// class to represent a tile window
// a tile window has a titlebar
class CQTileWindow : public QWidget {
  Q_OBJECT

 public:
  CQTileWindow();

  void setWidget(QWidget *w);

  QWidget *widget() const { return w_; }

 private:
  QWidget *w_;
};

//------

#include <CQTitleBar.h>

// widget for window area title bar
class CQTileWindowTitle : public CQTitleBar {
 public:
  CQTileWindowTitle(CQTileWindowArea *area);

  void setMaximized(bool maximized);

 private:
  QColor backgroundColor() const;
  QColor barColor() const;

  void mousePressEvent  (QMouseEvent *e);
  void mouseMoveEvent   (QMouseEvent *e);
  void mouseReleaseEvent(QMouseEvent *e);

  void keyPressEvent(QKeyEvent *e);

  bool event(QEvent *e);

 private:
  struct MouseState {
    bool   pressed;
    bool   escapePress;
    QPoint pressPos;

    MouseState(){
      reset();
    }

    void reset() {
      pressed     = false;
      escapePress = false;
    }
  };

  CQTileWindowArea *area_;
  MouseState        mouseState_;
  CQTitleBarButton *minimizeButton_;
  CQTitleBarButton *maximizeButton_;
  CQTitleBarButton *closeButton_;
  bool              maximized_;
};

#endif
