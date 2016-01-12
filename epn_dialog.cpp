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
    
    QNetworkAccessManager *networkManager = new QNetworkAccessManager(this);
    connect(networkManager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));
    
    
}

EPN_Dialog::~EPN_Dialog()
{
    delete ui;
}

void EPN_Dialog::on_horizontalSlider_valueChanged(int value)
{
    ui->minEdit->setText(QString::number(value));
}

void EPN_Dialog::replyFinished(QNetworkReply *reply)
{
    QJsonDocument jdoc;
    QJsonParseError jerr;
    QJsonObject jobj;
    QJsonValue val;

    jdoc.fromJson(reply->readAll(), &jerr);
    if (jerr.error == QJsonParseError::NoError) {
        jobj = jdoc.object();

        // Έλεγχος τιμών
        val = jobj.value("message");
        if (val != QJsonValue::Undefined) { // Αν υπάρχει μήνυμα
            showPopup("Message", val.toString());
        }
    }
}
