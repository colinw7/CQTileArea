#include <CQTileAreaTest.h>
#include <CQTileArea.h>

#include <QApplication>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMenu>
#include <QMenuBar>

#include <cstdio>

#include <icon.xpm>

class ButtonWindow : public QWidget {
 public:
  ButtonWindow(const QString &name) {
    setObjectName(name);

    setWindowTitle(name);
    setWindowIcon(QPixmap(icon_data));

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0); layout->setSpacing(0);

    button_ = new QPushButton(name);

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

  QAction *addAction  = new QAction("&Add" , fileMenu);
  QAction *quitAction = new QAction("&Quit", fileMenu);

  fileMenu->addAction(addAction);
  fileMenu->addAction(quitAction);

  connect(addAction , SIGNAL(triggered()), this, SLOT(addWindow()));
  connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));

  QMenu *debugMenu = menuBar()->addMenu("&Debug");

  QAction *fillAction   = new QAction("&Fill"  , debugMenu);
  QAction *dupAction    = new QAction("&Dup"   , debugMenu);
  QAction *placeAction  = new QAction("&Place" , debugMenu);
  QAction *adjustAction = new QAction("&Adjust", debugMenu);
  QAction *printAction  = new QAction("&Print" , debugMenu);

  debugMenu->addAction(fillAction);
  debugMenu->addAction(dupAction);
  debugMenu->addAction(placeAction);
  debugMenu->addAction(adjustAction);
  debugMenu->addAction(printAction);

  connect(fillAction  , SIGNAL(triggered()), area_, SLOT(fillSlot()));
  connect(dupAction   , SIGNAL(triggered()), area_, SLOT(dupSlot()));
  connect(placeAction , SIGNAL(triggered()), area_, SLOT(placeSlot()));
  connect(adjustAction, SIGNAL(triggered()), area_, SLOT(adjustSlot()));
  connect(printAction , SIGNAL(triggered()), area_, SLOT(printSlot()));

  static const char *names[] = {
    "One", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine"
  };

  for (int i = 0; i < 9; ++i)
    area_->addWindow(new ButtonWindow(names[i]), i / 3, i % 3);
}

void
CQTileAreaTest::
addWindow()
{
  static int ind;

  QString name = QString("Test%1").arg(++ind);

  area_->addWindow(new ButtonWindow(name));
}

void
CQTileAreaTest::
readPlaceFile(const QString &placeFile)
{
  std::vector<int> cells;
  int              num_rows, num_cols;
  int              row = -1, col = -1;

  FILE *fp = fopen(placeFile.toStdString().c_str(), "r");

  std::string line;

  while (readLine(fp, line)) {
    int pos = 0;

    if (row == -1) {
      if (! readInteger(line, &pos, &num_rows)) return;
      if (! readInteger(line, &pos, &num_cols)) return;

      cells.resize(num_rows*num_cols);

      row = 0;
    }
    else {
      int ii = row*num_cols;

      for (col = 0; col < num_cols; ++col) {
        if (! readInteger(line, &pos, &cells[ii + col]))
          return;
      }

      ++row;

      if (row >= num_rows)
        break;
    }
  }

  fclose(fp);

  area_->setGrid(num_rows, num_cols, cells);
}

bool
CQTileAreaTest::
readLine(FILE *fp, std::string &line)
{
  line = "";

  if (feof(fp))
    return false;

  while (! feof(fp)) {
    char c = fgetc(fp);

    if (c == '\n')
      return true;

    line += c;
  }

  return true;
}

bool
CQTileAreaTest::
readInteger(const std::string &line, int *pos, int *value)
{
  while (*pos < int(line.size()) && isspace(line[*pos]))
    ++(*pos);

  int i = *pos;

  while (*pos < int(line.size()) && isdigit(line[*pos]))
    ++(*pos);

  std::string istr = line.substr(i, *pos - i);

  if (istr == "")
    return false;

  *value = atoi(istr.c_str());

  return true;
}

//------

int
main(int argc, char **argv)
{
  QApplication app(argc, argv);

  QString placeFile;

  for (int i = 1; i < argc; ++i) {
    QString arg = argv[i];

    if (arg == "-p") {
      ++i;

      if (i < argc)
        placeFile = argv[i];
    }
  }

  CQTileAreaTest *test = new CQTileAreaTest;

  test->resize(400, 400);

  test->show();

  if (placeFile != "")
    test->readPlaceFile(placeFile);

  return app.exec();
}
