#pragma once

#include <QDialog>

namespace Ui {
class ValidateDialog;
}

class ValidateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ValidateDialog(QString project, QWidget *parent = nullptr);
    ~ValidateDialog();

    void push(QString string);

    QString project;

private:
    Ui::ValidateDialog *ui;
};
