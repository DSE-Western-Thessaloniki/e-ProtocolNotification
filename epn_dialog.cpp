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

    connect(ui->okButton, SIGNAL(released()), this, SLOT(saveSettings()));
    connect(ui->cancelButton, SIGNAL(released()), this, SLOT(cancel()));
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
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(iconCheckForDoubleClick(QSystemTrayIcon::ActivationReason)));

    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();
    
    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));
    
    popup = new Popup(this);
    settings = new QSettings("epn.ini", QSettings::IniFormat);
    username = settings->value("username", "").toString();
    ui->usernameEdit->setText(username);
    url = settings->value("url", "").toUrl();

    timer = new QTimer(this);
    timer->start(120000);
    ui->minEdit->setText(QString::number(2));
    connect(timer, SIGNAL(timeout()), this, SLOT(getUpdate()));

    dontshowagain = false;
    lowPriorityMsg = 0;

    qDebug() << "Support SSL:  " << QSslSocket::supportsSsl()
            << "\nLib Version Number: " << QSslSocket::sslLibraryVersionNumber()
            << "\nLib Version String: " << QSslSocket::sslLibraryVersionString()
            << "\nLib Build Version Number: " << QSslSocket::sslLibraryBuildVersionNumber()
            << "\nLib Build Version String: " << QSslSocket::sslLibraryBuildVersionString();
}

EPN_Dialog::~EPN_Dialog()
{
    settings->setValue("url", url.toString());
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
    QJsonObject jobj, joptobj;
    QJsonValue val;

    if (reply->error() == QNetworkReply::NoError) {
        //qDebug() << reply->readAll();
        jdoc = QJsonDocument::fromJson(reply->readAll(), &jerr);
        if (jerr.error == QJsonParseError::NoError) {
            dontshowagain = false;
            jobj = jdoc.object();

            // Έλεγχος τιμών
            val = jobj.value("message");
            if (val != QJsonValue::Undefined) { // Αν υπάρχει μήνυμα
                if (jobj.value("priority").toString() == "high") { // high priority
                    popup->setPriority(Popup::HighPriority);
                    popup->showPopup("Ενημέρωση", val.toString());
                }
                else {
                    if (jobj.value("priority").toString() == "normal") { // normal priority
                        popup->setPriority(Popup::NormalPriority);
                        if (!lowPriorityMsg) // Show low priority messages only 1/4 of the time (when value is 0)
                            popup->showPopup("Ενημέρωση", val.toString());
                    }
                    else {
                        popup->setPriority(Popup::NoPriority);
                        if (!lowPriorityMsg) // Show low priority messages only 1/4 of the time (when value is 0)
                            popup->showPopup("Ενημέρωση", val.toString());
                    }
                    lowPriorityMsg = (lowPriorityMsg + 1) % 4;
                }
            }
            val = jobj.value("options");
            if (val != QJsonValue::Undefined) { // Αν υπάρχουν ρυθμίσεις για το πρόγραμμα
                joptobj = val.toObject();
                val = joptobj.value("timeout");
                timeout = val.toInt(30*60*1000); // Default: 30min
                if (timeout > 60000) // Just in case...
                    timer->setInterval(timeout);
                ui->minEdit->setText(QString::number(timeout));
                if (joptobj.value("url") != QJsonValue::Undefined) {
                    url = joptobj.value("url").toString(); // Δες αν υπάρχει νέα ρύθμιση για το url
                    settings->setValue("url",url.toString());
                }
                val = joptobj.value("version");
                if (val != QJsonValue::Undefined) {
                    version = joptobj.value("version").toString(); // Διάβασε την τελευταία έκδοση του προγράμματος
                    if (compareVersions(QString(VERSION),version)>0) {
                        // Υπάρχει νέα έκδοση του προγράμματος. Κατέβασέ το!
                        val = joptobj.value("filelist");
                        if (val != QJsonValue::Undefined) {
                            fileDownloader.getFiles(joptobj.value("filelist").toArray().toVariantList(), QUrl(url.toString() + QString("/download/")));
                            //filelist = jobj.value("filelist").toArray();
                            //for (int i=0; i<filelist.size(); i++) {
                                //QJsonArray file = filelist[0].toArray();
                                //downloadList << new FileDownloader(QUrl(url.toString() + QString("/download/") + file[0].toString()), file[1].toString(), file[2].toInt());
                        }
                    }
                }
            }
            trayIcon->setIcon(QIcon(":/icons/epn-icon.png"));
        }
        else {
            qDebug() << "Json Error: " << jerr.errorString();
            if (!dontshowagain) {
                popup->setPriority(Popup::Error);
                popup->showPopup("Σφάλμα!","Μη έγκυρη απάντηση από το διακομιστή.");
                trayIcon->setIcon(QIcon(":/icons/epn-icon-error.png"));
            }
            dontshowagain = true;
        }
    }
    else {
        qDebug() << "Network Error: " << reply->errorString();
        if (!dontshowagain) {
            popup->setPriority(Popup::Error);
            popup->showPopup("Σφάλμα!","Πρόβλημα σύνδεσης με το διακομιστή.");
            trayIcon->setIcon(QIcon(":/icons/epn-icon-error.png"));
        }
        dontshowagain = true;
    }
    reply->deleteLater();
}

void EPN_Dialog::getUpdate()
{
    QUrl query;

    query = QUrl(url.toString() + QString("?username=") + username);
    if (username.isEmpty()) {
        show();
    }
    else
        networkManager->get(QNetworkRequest(query));
    qDebug() << "Trying request from " << query.toString();
}

void EPN_Dialog::saveSettings(void)
{
    username = ui->usernameEdit->text();
    settings->setValue("username", username);
    getUpdate();
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

void EPN_Dialog::cancel(void)
{
    ui->usernameEdit->setText(username);
    hide();
}

void EPN_Dialog::iconCheckForDoubleClick(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick)
        show();
}
