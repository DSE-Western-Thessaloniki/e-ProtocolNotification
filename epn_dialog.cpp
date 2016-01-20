#include <QMenu>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonValue>
#include <QProcess>
#include <QDir>
#include <QDebug>

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
    //trayMenu->addSeparator();
    //exitAction = trayMenu->addAction("Έξοδος");
    connect(openAction, SIGNAL(triggered(bool)), this, SLOT(show()));
    //connect(exitAction, SIGNAL(triggered(bool)), this, SLOT(quit()));
    shortcut = new QShortcut(QKeySequence("Ctrl+Q"),this);
    connect(shortcut, SIGNAL(activated()), this, SLOT(quit()));
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

    connect(&fileDownloader, SIGNAL(hashChecked()), this, SLOT(upgradeProgram()));

    dontshowagain = false;
    lowPriorityMsg = 0;
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
                    lowPriorityMsg = (lowPriorityMsg + 1) % 6;
                }
            }
            val = jobj.value("options");
            if (val != QJsonValue::Undefined) { // Αν υπάρχουν ρυθμίσεις για το πρόγραμμα
                joptobj = val.toObject();
                val = joptobj.value("timeout");
                timeout = val.toInt(30*60*1000); // Default: 30min
                if (timeout > 60000) // Just in case...
                    timer->setInterval(timeout);
                ui->minEdit->setText(QString::number(timeout/60000));
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
    trayIcon->hide();
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

void EPN_Dialog::upgradeProgram()
{
    bool success = true;
    if (fileDownloader.error() == FileDownloader::NoError) { // Files downloaded with no errors
        QStringList files = fileDownloader.downloadedFiles();
        QMap<int,QByteArray> data = fileDownloader.downloadedData();
        for (int i=0; i<files.size(); i++) {
            QStringList path = files[i].split("/");
            if (path.size() > 1) {
                path.removeLast();
                QString tmppath=path.join('/');
                QDir dir;
                if (dir.mkpath(tmppath)==false) { // We've got trouble...
                    success = false;
                    qDebug() << "Error creating dir path " << tmppath;
                }
            }
            QFile f(files[i]+".new");
            if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                if (f.write(data[i]) == -1) {
                    success = false;
                    qDebug() << "Error writing data to " << files[i];
                }
                f.close();
            }
            else {
                qDebug() << "Failed opening " << files[i];
                success = false;
            }
        }
        if (success) { // All OK for now. Do the renaming
            for (int i=0; i<files.size(); i++) {
                if (QFile::exists(files[i])) {
                    if (!QFile::rename(files[i],files[i]+".old")) {
                        qDebug() << "Failed renaming " << files[i] << " to " << files[i] << ".old";
                        success = false;
                    }
                }
            }
            if (success) {
                for (int i=0; i<files.size(); i++) {
                    if (!QFile::rename(files[i]+".new",files[i])) {
                        qDebug() << "Failed renaming " << files[i] << ".new to " << files[i];
                        success = false;
                    }
                }
            }
            else {
                for (int i=0; i<files.size(); i++) {
                    if (!QFile::rename(files[i]+".old",files[i])) {
                        qDebug() << "Failed renaming " << files[i] << ".old to " << files[i];
                        qDebug() << "Something went horribly wrong! :-(";
                        success = false;
                    }
                }
                if (QProcess::startDetached(files[0])) // Start new version and quit this one
                    QCoreApplication::exit();
            }
        }
    }
}
