#pragma once

#include <QString>
#include <QWidget>

struct snack {
    QString filename;
    int sheet_w;
    int sheet_h;
    int sheet_pos;
};

QList<snack> load_snack_list(QWidget *parent);
