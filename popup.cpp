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
    //setStyleSheet(QString("QDialog {background-color: yellow}"));
    ui->title->setStyleSheet(QString("QLabel {background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 #E0E0E0, stop: 1 #000000);}"));
    //ui->description->setStyleSheet(QString("QLabel {background-color: rgba(0,0,0,25%);}"));
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
