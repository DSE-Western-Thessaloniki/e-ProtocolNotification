#ifndef EPN_DIALOG_H
#define EPN_DIALOG_H

#include <QDialog>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSettings>
#include <QTimer>
#include "popup.h"

namespace Ui {
class EPN_Dialog;
}

class EPN_Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit EPN_Dialog(QWidget *parent = 0);
    ~EPN_Dialog();

private slots:
    void on_horizontalSlider_valueChanged(int value);
    void replyFinished(QNetworkReply*);
    void getUpdate(void);

private:
    Ui::EPN_Dialog *ui;
    QNetworkAccessManager *networkManager;
    Popup *popup;
    int timeout;
    QUrl url;
    QSettings *settings;
    QString username;
    QTimer *timer;
};

#endif // EPN_DIALOG_H