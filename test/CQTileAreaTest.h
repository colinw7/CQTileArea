#include <QMainWindow>

class CQTileArea;

class CQTileAreaTest : public QMainWindow {
  Q_OBJECT

 public:
  CQTileAreaTest();

 private:
  CQTileArea *area_;
};
