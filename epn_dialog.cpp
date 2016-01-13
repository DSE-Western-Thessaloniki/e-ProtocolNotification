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
    trayIcon = new QSystemTrayIcon(QIcon(":/icons/epn-icon.png"));
    trayIcon->setToolTip(QString("e-Protocol Notification"));
    QMenu *trayMenu = new QMenu();

    openAction = trayMenu->addAction("Ρυθμίσεις");
    trayMenu->addSeparator();
    exitAction = trayMenu->addAction("Έξοδος");
    connect(openAction, SIGNAL(triggered(bool)), this, SLOT(show()));
    connect(exitAction, SIGNAL(triggered(bool)), this, SLOT(quit()));

    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();
    
    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));
    
    popup = new Popup(this);
    settings = new QSettings("epn.ini", QSettings::IniFormat);
    username = settings->value("username").toString();
    url = settings->value("url").toUrl();

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(getUpdate()));

    dontshowagain = false;
    lowPriorityMsg = 0;
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
        dontshowagain = false;
        jobj = jdoc.object();

        // Έλεγχος τιμών
        val = jobj.value("message");
        if (val != QJsonValue::Undefined) { // Αν υπάρχει μήνυμα
            if (jobj.value("priority").toString() == "low") { // low priority
                lowPriorityMsg = (lowPriorityMsg + 1) % 4;
                if (!lowPriorityMsg) // Show low priority messages only 1/4 of the time (when value is 0)
                    popup->showPopup("Message", val.toString());
            }
            else
                popup->showPopup("Message", val.toString());
        }
        val = jobj.value("timeout");
        timeout = val.toInt(30);
        url = jobj.value("url").toString(); // Δες αν υπάρχει νέα ρύθμιση για το url
        version = jobj.value("version").toString(); // Διάβασε την τελευταία έκδοση του προγράμματος
        if (compareVersions(QString(VERSION),version)>0) {
            // Υπάρχει νέα έκδοση του προγράμματος. Κατέβασέ το!
        }
        trayIcon->setIcon(QIcon(":/icons/epn-icon.png"));
    }
    else {
        if (!dontshowagain) {
            popup->showPopup("Σφάλμα!","Πρόβλημα σύνδεσης με το διακομιστή.");
            trayIcon->setIcon(QIcon(":/icons/epn-icon-error.png"));
        }
        dontshowagain = true;
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

void EPN_Dialog::closeEvent(QCloseEvent *event)
{
    if (trayIcon->isVisible()) {
        hide();
        event->ignore();
    }
}

int EPN_Dialog::compareVersions(QString ver1, QString ver2)
{
    // Οι εκδόσεις είναι πάντα της μορφής x.y.z
    QStringList listver1 = ver1.split(".");
    QStringList listver2 = ver2.split(".");

    if (listver1.at(0).toInt()>listver2.at(0).toInt())
        return -1;
    else if (listver1.at(0).toInt()<listver2.at(0).toInt())
        return 1;
    else {
        if (listver1.at(1).toInt()>listver2.at(1).toInt())
            return -1;
        else if (listver1.at(1).toInt()<listver2.at(1).toInt())
            return 1;
        else {
            if (listver1.at(2).toInt()>listver2.at(2).toInt())
                return -1;
            else if (listver1.at(2).toInt()<listver2.at(2).toInt())
                return 1;
            return 0;
        }
    }
}
