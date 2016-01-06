#include <QSystemTrayIcon>
#include <QMenu>

#include "epn_dialog.h"
#include "ui_epn_dialog.h"

EPN_Dialog::EPN_Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EPN_Dialog)
{
    ui->setupUi(this);
    QSystemTrayIcon *trayIcon = new QSystemTrayIcon();
    QMenu *trayMenu = new QMenu();
    QAction *openAction, *exitAction;
    QPixmap p(16,16);

    p.fill(Qt::blue);
    QIcon myicon(p);

    openAction = trayMenu->addAction("Άνοιγμα");
    trayMenu->addSeparator();
    exitAction = trayMenu->addAction("Έξοδος", this, SLOT(reject()));
    trayIcon->setIcon(myicon);

    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();
}

EPN_Dialog::~EPN_Dialog()
{
    delete ui;
}

void EPN_Dialog::on_horizontalSlider_valueChanged(int value)
{
    ui->minEdit->setText(QString::number(value));
}
