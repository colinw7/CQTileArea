#ifndef CQRubberBand_H
#define CQRubberBand_H

#include <QRubberBand>

class CQRubberBandStyle;

class CQRubberBand : public QRubberBand {
  Q_OBJECT

 public:
  CQRubberBand(QWidget *p=0);

 private:
  CQRubberBandStyle *style_;
};

#endif
