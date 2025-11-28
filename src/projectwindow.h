#pragma once

#include <QWidget>

namespace Ui {
class ProjectWindow;
}

class ProjectWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ProjectWindow(QWidget *parent = nullptr);
    ~ProjectWindow();

private slots:
    void on_projectPushButton_clicked();

    void on_rpPushButton_clicked();

void on_coverPushButton_clicked();

void on_themePushButton_clicked();

void on_reservationListPushButton_clicked();

void on_validateRPPushButton_clicked();

private:
    Ui::ProjectWindow *ui;
    QString project;
};
