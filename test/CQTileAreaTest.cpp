#include <CQTileAreaTest.h>
#include <CQTileArea.h>

#include <QApplication>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMenu>
#include <QMenuBar>

#include <icon.xpm>

class ButtonWindow : public QWidget {
 public:
  ButtonWindow(const std::string &name) {
    setWindowTitle(name.c_str());
    setWindowIcon(QPixmap(icon_data));

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0); layout->setSpacing(0);

    button_ = new QPushButton(name.c_str());

    layout->addWidget(button_);
  }

 private:
  QPushButton *button_;
};

CQTileAreaTest::
CQTileAreaTest() :
 QMainWindow()
{
  area_ = new CQTileArea;

  setCentralWidget(area_);

  QMenu *fileMenu = menuBar()->addMenu("&File");

  QAction *fillAction   = new QAction("&Fill"  , fileMenu);
  QAction *dupAction    = new QAction("&Dup"   , fileMenu);
  QAction *placeAction  = new QAction("&Place" , fileMenu);
  QAction *adjustAction = new QAction("&Adjust", fileMenu);
  QAction *printAction  = new QAction("&Print" , fileMenu);
  QAction *quitAction   = new QAction("&Quit"  , fileMenu);

  fileMenu->addAction(fillAction);
  fileMenu->addAction(dupAction);
  fileMenu->addAction(placeAction);
  fileMenu->addAction(adjustAction);
  fileMenu->addAction(printAction);
  fileMenu->addAction(quitAction);

  connect(fillAction  , SIGNAL(triggered()), area_, SLOT(fillSlot()));
  connect(dupAction   , SIGNAL(triggered()), area_, SLOT(dupSlot()));
  connect(placeAction , SIGNAL(triggered()), area_, SLOT(placeSlot()));
  connect(adjustAction, SIGNAL(triggered()), area_, SLOT(adjustSlot()));
  connect(printAction , SIGNAL(triggered()), area_, SLOT(printSlot()));
  connect(quitAction  , SIGNAL(triggered()), this , SLOT(close()));

  static const char *names[] = {
    "One", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine"
  };

  for (int i = 0; i < 9; ++i)
    area_->addWindow(new ButtonWindow(names[i]), i / 3, i % 3);
}

int
main(int argc, char **argv)
{
  QApplication app(argc, argv);

  CQTileAreaTest *test = new CQTileAreaTest;

  test->resize(400, 400);

  test->show();

  return app.exec();
}
