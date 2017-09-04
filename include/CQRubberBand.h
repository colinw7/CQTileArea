#ifndef CQRubberBand_H
#define CQRubberBand_H

#include <QRubberBand>

class CQRubberBandStyle;

class CQRubberBand : public QRubberBand {
  Q_OBJECT

 public:
  CQRubberBand(QWidget *p=0);
  CQRubberBand(QRubberBand::Shape shape, QWidget *p=0);

 ~CQRubberBand();

  const QColor &color() const;
  void setColor(const QColor &c);

 private:
  CQRubberBandStyle *style_ { nullptr };
};

#endif
