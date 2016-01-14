#ifndef EPN_DIALOG_H
#define EPN_DIALOG_H

#include <QDialog>
#include <QSystemTrayIcon>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSettings>
#include <QTimer>
#include <QResource>
#include <QJsonArray>
#include "popup.h"
#include "version.h"
#include "filedownloader.h"

namespace Ui {
class EPN_Dialog;
}

class EPN_Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit EPN_Dialog(QWidget *parent = 0);
    ~EPN_Dialog();

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

public slots:
    void getUpdate(void);

private slots:
    void replyFinished(QNetworkReply*);
    void saveSettings(void);
    void cancel(void);
    void quit(void);
    void iconCheckForDoubleClick(QSystemTrayIcon::ActivationReason);

private:
    int compareVersions(QString, QString);

private:
    Ui::EPN_Dialog *ui;
    QNetworkAccessManager *networkManager;
    Popup *popup;
    int timeout;
    QUrl url;
    QSettings *settings;
    QString username, version;
    QTimer *timer;
    QAction *openAction, *exitAction;
    QResource resource;
    bool dontshowagain;
    int lowPriorityMsg;
    QSystemTrayIcon *trayIcon;
    QList<FileDownloader*> downloadList;
    QJsonArray filelist;
};

#endif // EPN_DIALOG_H
