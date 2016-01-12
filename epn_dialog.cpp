#include <QSystemTrayIcon>
#include <QMenu>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonValue>

#include "epn_dialog.h"
#include "ui_epn_dialog.h"

EPN_Dialog::EPN_Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EPN_Dialog)
{
    ui->setupUi(this);

    connect(this, SIGNAL(accepted()), this, SLOT(saveSettings()));
    setWindowTitle(QString("e-protocol notification v")+VERSION);

    // Create a tray icon
    QSystemTrayIcon *trayIcon = new QSystemTrayIcon(QIcon(":/icons/epn-icon.png"));
    QMenu *trayMenu = new QMenu();

    openAction = trayMenu->addAction("Ρυθμίσεις");
    trayMenu->addSeparator();
    exitAction = trayMenu->addAction("Έξοδος");
    connect(openAction, SIGNAL(triggered(bool)), this, SLOT(show()));
    connect(exitAction, SIGNAL(triggered(bool)), this, SLOT(quit()));

    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();
    
    QNetworkAccessManager *networkManager = new QNetworkAccessManager(this);
    connect(networkManager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));
    
    popup = new Popup(this);
    settings = new QSettings("epn.ini", QSettings::IniFormat);
    username = settings->value("username").toString();
    url = settings->value("url").toUrl();

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(getUpdate()));
}

EPN_Dialog::~EPN_Dialog()
{
    settings->setValue("url", url);
    timer->stop();

    delete ui;
    delete popup;
    delete settings;
    delete timer;
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
            // TODO: Έλεγχος αν είναι σημαντικό μήνυμα, αν όχι εμφάνισε 1 στις 4
            popup->showPopup("Message", val.toString());
        }
        val = jobj.value("timeout");
        timeout = val.toInt(30);
        val = jobj.value("url");
        if (val != QJsonValue::Undefined) // Αν υπάρχει νέα ρύθμιση για το url
            url = QUrl(val.toString());
    }
}

void EPN_Dialog::getUpdate()
{
    if (username.isEmpty()) {
        show();
    }
    else
        networkManager->get(QNetworkRequest(QUrl(url)));
}

void EPN_Dialog::saveSettings(void)
{
    username = ui->usernameEdit->text();
    settings->setValue("username", username);
    hide();
}

void EPN_Dialog::quit(void) {
    QApplication::quit();
}
