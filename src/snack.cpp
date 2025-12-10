#include "snack.h"

#include <QFileDialog>
#include <QMessageBox>

QList<snack> load_snack_list(QWidget *parent)
{
    QList<snack> snack_list;
    QFile f(QFileDialog::getOpenFileName(parent, "Select the giant VM data", "", "Tab separated values (*.tsv)"));
    if (f.open(QFile::ReadOnly | QFile::Text)){
        QTextStream in(&f);
        QString s;
        QStringList validate = in.readLine().split("\t");
        if (
            validate[0] != "ID" ||
            validate[1] != "Picture" ||
            validate[2] != "Spritesheet (horizontal,vertical,offset)"
            ) {
            QMessageBox::critical(parent, "Error", "Invalid header detected! This might not be a giant VM data .tsv file.");
            return QList<snack>();
        }

        while (in.readLineInto(&s)) {
            QStringList split = s.split("\t");
            if (split.size() >= 3) {
                QStringList sheet_raw = split[2].split(",");
                QList<int> sheet;
                if (sheet_raw.size() >= 3) {
                    sheet = {sheet_raw[0].toInt(), sheet_raw[1].toInt(), sheet_raw[2].toInt()};
                } else {
                    sheet = {-1, -1, -1};
                }
                snack_list.append(snack{split[1], sheet[0], sheet[1], sheet[2]});
            }
        }
    } else {
        return QList<snack>();
    }
    return snack_list;
}
