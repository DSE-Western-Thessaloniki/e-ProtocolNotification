#ifndef EPN_DIALOG_H
#define EPN_DIALOG_H

#include <QDialog>

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

private:
    Ui::EPN_Dialog *ui;
};

#endif // EPN_DIALOG_H
