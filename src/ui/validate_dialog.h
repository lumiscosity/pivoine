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

    void validate_record_player();

private:
    QString project;
    Ui::ValidateDialog *ui;
    void push(QString string);
};
