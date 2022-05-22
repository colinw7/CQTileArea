#ifndef CQTileWindowArea_H
#define CQTileWindowArea_H

#include <CQTileArea.h>

#include <QFrame>

class CQTileWindow;

//! class for each window area
//! a window area can contain one or more tile windows
//! multiple windows in a tile area produces a tab widget (customizable)
class CQTileWindowArea : public QFrame {
  Q_OBJECT

 public:
  using Windows = std::vector<CQTileWindow *>;

 public:
  //! create window area
  CQTileWindowArea(CQTileArea *area);

  //! destroy window area
 ~CQTileWindowArea();

  //! get parent tile area
  CQTileArea *area() const { return area_; }

  //! get id
  int id() const { return id_; }

  //! get child windows
  const Windows &getWindows() const { return windows_; }

  //! add widget to area
  CQTileWindow *addWidget(QWidget *w);

  //! is currently detached
  bool isDetached () const { return detached_; }
  //! is currently floating
  bool isFloating () const { return floating_; }
  //! is currently docked
  bool isDocked   () const { return ! isDetached() && ! isFloating(); }
  //! is currently maximized
  bool isMaximized() const;

  //! get title
  QString getTitle() const;

  //! get icon
  QIcon getIcon() const;

  //! get current window
  CQTileWindow *currentWindow() const;
  //! get current window
  void setCurrentWindow(CQTileWindow *window);

  //! has window
  bool hasWindow(CQTileWindow *window) const;

  //! size hint
  QSize sizeHint() const override;
  //! size minimum hint
  QSize minimumSizeHint() const override;

 private:
  //! set detached
  void setDetached(bool detached);
  //! set floating
  void setFloating(bool floating);

  //! get position of detached area
  int getDetachPos(int w, int h) const;

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
  void updateContextMenu(QMenu *menu) const;

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
  CQWidgetResizer     *resizer_;    //! detached resizer
  Windows              windows_;    //! child window
  bool                 detached_;   //! detached flag
  bool                 floating_;   //! floating flag
  AttachData           attachData_; //! attach data
};

#endif
