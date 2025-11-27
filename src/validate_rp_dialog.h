#pragma once

#include <QDialog>

namespace Ui {
class ValidateRPDialog;
}

class ValidateRPDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ValidateRPDialog(QString project, QWidget *parent = nullptr);
    ~ValidateRPDialog();

    void validate();

private:
    QString project;
    Ui::ValidateRPDialog *ui;
    void push(QString string);
};
