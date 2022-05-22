#ifndef CQTileAreaMenuControls_H
#define CQTileAreaMenuControls_H

#include <QFrame>

class CQTileArea;

class QToolButton;

class CQTileAreaMenuControls : public QFrame {
  Q_OBJECT

 public:
  CQTileAreaMenuControls(CQTileArea *area);

  void updateState();

 private:
  QToolButton *createButton(const char **data, const QString &tip);

 private:
  CQTileArea  *area_          { nullptr }; //!< parent area
  QToolButton *detachButton_  { nullptr }; //!< detach button
  QToolButton *restoreButton_ { nullptr }; //!< restore button
  QToolButton *closeButton_   { nullptr }; //!< close button
};

#endif
