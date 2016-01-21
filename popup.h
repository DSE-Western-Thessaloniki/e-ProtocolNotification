#ifndef POPUP_H
#define POPUP_H

#include <QDialog>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QTimer>

namespace Ui {
class Popup;
}

class Popup : public QDialog
{
    Q_OBJECT

public:
    explicit Popup(QWidget *parent = 0);
    ~Popup();

    void showPopup(QString title, QString description);
    void mousePressEvent(QMouseEvent*);
    void setPriority(int);
    void setTimeout(int);

public:
    enum Priority {NoPriority, NormalPriority, HighPriority, Error};

private slots:
    void closePopup();

signals:
    void closed();
    void clicked();

private:
    Ui::Popup *ui;
    QString noPrioritySS, normalPrioritySS, highPrioritySS, errorSS;
    QTimer timer;
    int timeout;
};

#endif // POPUP_H
