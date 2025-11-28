#include "projectwindow.h"
#include "book.h"
#include "record_player.h"
#include "reservation_list.h"
#include "ui_projectwindow.h"
#include "cover.h"
#include "theme.h"
#include "validate_rp_dialog.h"

#include <QFileDialog>
#include <QMessageBox>

ProjectWindow::ProjectWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProjectWindow)
{
    ui->setupUi(this);
}

ProjectWindow::~ProjectWindow()
{
    delete ui;
}

void ProjectWindow::on_projectPushButton_clicked()
{
    this->project = QFileDialog::getExistingDirectory(this, "Select the project folder");
    bool dirty = this->project.isEmpty();
    if (!dirty && !QFile(this->project+"/RPG_RT.ldb").exists()) {
        dirty = true;
        QMessageBox::critical(this, "Error", "This folder isn't an RPG Maker 2003 project!");
    }
    if (dirty) {
        ui->projectTextLabel->setText("...");
    } else {
        ui->projectTextLabel->setText(this->project);
    }
    ui->rpPushButton->setEnabled(!dirty);
    ui->coverPushButton->setEnabled(!dirty);
    ui->themePushButton->setEnabled(!dirty);
    ui->reservationListPushButton->setEnabled(!dirty);
}


void ProjectWindow::on_rpPushButton_clicked()
{
    int run = run_record_player(this->project.toStdString(), this, ui->overwriteCheckBox->isChecked());
    if (!run) {
        QMessageBox::information(this, "Finished", "Record player updated!");
    } else if (run != 1) {
        QMessageBox::warning(this, "Error", "Something went wrong! Error code: " + QString::number(run));
    }

}


void ProjectWindow::on_coverPushButton_clicked()
{
    int run = run_cover(this->project.toStdString(), this, ui->overwriteCheckBox->isChecked());
    if (!run) {
        QMessageBox::information(this, "Finished", "Covers updated!");
    } else if (run != 1) {
        QMessageBox::warning(this, "Error", "Something went wrong! Error code: " + QString::number(run));
    }
}


void ProjectWindow::on_themePushButton_clicked()
{
    int run = run_theme(this->project.toStdString(), this, ui->overwriteCheckBox->isChecked());
    if (!run) {
        QMessageBox::information(this, "Finished", "Themes updated!");
    } else if (run != 1) {
        QMessageBox::warning(this, "Error", "Something went wrong! Error code: " + QString::number(run));
    }
}

void ProjectWindow::on_reservationListPushButton_clicked()
{
    int run = run_reservation_list(this->project.toStdString(), this);
    if (!run) {
        QMessageBox::information(this, "Finished", "Reservation list file generated!");
    } else if (run != 1) {
        QMessageBox::warning(this, "Error", "Something went wrong! Error code: " + QString::number(run));
    }
}


void ProjectWindow::on_validateRPPushButton_clicked()
{
    ValidateRPDialog dialog(this->project, this);
    dialog.validate();
    dialog.exec();
}

