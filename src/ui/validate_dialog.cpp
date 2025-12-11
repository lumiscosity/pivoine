#include "../../third_party/dbstring.h"
#include "validate_dialog.h"
#include "ui_validate_dialog.h"

#include <QMessageBox>

ValidateDialog::ValidateDialog(QString project, QWidget *parent)
    : QDialog(parent)
    , project(project), ui(new Ui::ValidateDialog)
{
    ui->setupUi(this);
}

ValidateDialog::~ValidateDialog()
{
    delete ui;
}

void ValidateDialog::push(QString string) {
    ui->plainTextEdit->appendPlainText(string);
}
