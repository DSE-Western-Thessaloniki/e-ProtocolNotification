#include <QJsonDocument>

#include "popup.h"
#include "ui_popup.h"

Popup::Popup(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Popup)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_ShowWithoutActivating);
    setWindowFlags(
                Qt::Window | // Add if popup doesn't show up
                Qt::FramelessWindowHint | // No window border
                Qt::WindowDoesNotAcceptFocus | // No focus
                Qt::WindowStaysOnTopHint // Always on top
                );

    connect(ui->button, SIGNAL(clicked()), this, SLOT(closePopup()));

    noPrioritySS = QString("QLabel {background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                           "stop: 0 rgb(191,210,85), stop: 0.5 rgb(142,185,42),"
                           "stop:0.51 rgb(114,170,0), stop:1 rgb(158,203,45));"
                           "border-radius: 5px;"
                           "padding: 10px;"
                           "font: bold 14px;}");
    normalPrioritySS = QString("QLabel {background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                           "stop: 0 rgb(109,179,242), stop: 0.5 rgb(84,163,238),"
                           "stop:0.51 rgb(54,144,240), stop:1 rgb(30,105,222));"
                           "border-radius: 5px;"
                           "padding: 10px;"
                           "font: bold 14px;}");
    highPrioritySS = QString("QLabel {background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                           "stop: 0 rgb(255,183,107), stop: 0.5 rgb(255,167,61),"
                           "stop:0.51 rgb(255,124,0), stop:1 rgb(255,127,4));"
                           "border-radius: 5px;"
                           "padding: 10px;"
                           "font: bold 14px;}");
    errorSS = QString("QLabel {background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                           "stop: 0 rgb(248,80,50), stop: 0.5 rgb(241,111,92),"
                           "stop:0.51 rgb(246,41,12), stop: 0.71 rgb(240,47,23), stop:1 rgb(231,56,39));"
                           "border-radius: 5px;"
                           "padding: 10px;"
                           "font: bold 14px;}");

    //setStyleSheet(QString("QDialog {background-color: yellow}"));
    ui->title->setStyleSheet(QString("QLabel {background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 #E0E0E0, stop: 1 #000000);"
                                     "font: bold 12px;}"));
    ui->description->setStyleSheet(noPrioritySS);
    connect(&timer, SIGNAL(timeout()), this, SLOT(closePopup()));
    timer.setSingleShot(true);
}

Popup::~Popup()
{
    delete ui;
}

void Popup::showPopup(QString title, QString description)
{
    ui->title->setText(QString("<b>%1</b>").arg(title));
    ui->description->setText(description);

    // resize the widget, as text label may be larger than the previous size
    setGeometry(QStyle::alignedRect(
                    Qt::RightToLeft,
                    Qt::AlignBottom,
                    size(),
                    qApp->desktop()->availableGeometry()));
    show();
    timer.start(5000);
}

void Popup::closePopup()
{
    emit closed();
    hide();
}

void Popup::mousePressEvent(QMouseEvent*)
{
    emit clicked();
}

void Popup::setPriority(int priority)
{
    if (priority == Popup::Error)
        ui->description->setStyleSheet(errorSS);
    else if (priority == Popup::HighPriority)
        ui->description->setStyleSheet(highPrioritySS);
    else if (priority == Popup::NormalPriority)
        ui->description->setStyleSheet(normalPrioritySS);
    else // Popup::NoPriority
        ui->description->setStyleSheet(noPrioritySS);
}
