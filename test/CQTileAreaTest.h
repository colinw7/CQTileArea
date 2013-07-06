#include <QMainWindow>

class CQTileArea;

class CQTileAreaTest : public QMainWindow {
  Q_OBJECT

 public:
  CQTileAreaTest();

  void readPlaceFile(const QString &placeFile);

 private:
  bool readLine(FILE *fp, std::string &line);
  bool readInteger(const std::string &line, int *pos, int *value);

 private:
  CQTileArea *area_;
};
