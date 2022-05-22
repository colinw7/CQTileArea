#include <CQTileAreaMenuControls.h>
#include <CQTileArea.h>

#include <QHBoxLayout>
#include <QToolButton>

#include <images/detach.xpm>
#include <images/restore.xpm>
#include <images/close.xpm>

CQTileAreaMenuControls::
CQTileAreaMenuControls(CQTileArea *area) :
 QFrame(area), area_(area)
{
  setObjectName("menuControls");

  auto *layout = new QHBoxLayout(this);
  layout->setMargin(0); layout->setSpacing(1);

  detachButton_  = createButton(detach_data , "Detach" );
  restoreButton_ = createButton(restore_data, "Restore");
  closeButton_   = createButton(close_data  , "Close"  );

  layout->addWidget(detachButton_ );
  layout->addWidget(restoreButton_);
  layout->addWidget(closeButton_  );

  connect(detachButton_ , SIGNAL(clicked()), area_, SLOT(detachSlot()));
  connect(restoreButton_, SIGNAL(clicked()), area_, SLOT(restoreSlot()));
  connect(closeButton_  , SIGNAL(clicked()), area_, SLOT(closeSlot()));
}

void
CQTileAreaMenuControls::
updateState()
{
  restoreButton_->setEnabled(area_->isRestoreStateValid());
}

QToolButton *
CQTileAreaMenuControls::
createButton(const char **data, const QString &tip)
{
  QFontMetrics fm(font());

  int s = std::min(fm.height() + 2, 15);

  auto *button = new QToolButton;

  button->setObjectName(tip);

  button->setFixedSize(s, s);

  button->setIcon(QPixmap(data));
  button->setAutoRaise(true);
  button->setToolTip(tip);

  return button;
}
