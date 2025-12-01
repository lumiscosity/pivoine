#include "track.h"

#include <QMessageBox>

QList<QList<track>> load_track_list(QWidget *parent) {
    QList<QList<track>> track_list;
    QFile f(QFileDialog::getOpenFileName(parent, "Select the record player data", "", "Tab separated values (*.tsv)"));
    if (f.open(QFile::ReadOnly | QFile::Text)){
        QTextStream in(&f);
        QString s;
        QStringList header = in.readLine().split("\t");
        if (
            header[0] != "slot id" ||
            header[1] != "track file" ||
            header[2] != "Artist" ||
            header[3] != "volume (%)" ||
            header[4] != "speed (%)" ||
            header[5] != "Location" ||
            header[6] != "Map id" ||
            header[7] != "Unlock skill"
            ) {
            QMessageBox::critical(parent, "Error", "Invalid header detected! This might not be a record player data .tsv file.");
            return QList<QList<track>>();
        }

        while (in.readLineInto(&s)) {
            QStringList split = s.split("\t");
            if (split.size() >= 7) {
                bool has_id = split[0].isEmpty();
                QStringList maps_raw = split[6].split("/");
                QList<int> maps = QList<int>();
                for (QString i : maps_raw) {
                    maps.append(i.toInt());
                }
                if (has_id) {
                    track_list.last().append(track(split[1].trimmed(), split[2], split[3].toInt(), split[4].toInt(), split[5], maps, split[7].toInt()));
                } else {
                    QList<track> temp;
                    temp.append(track(split[1].trimmed(), split[2], split[3].toInt(), split[4].toInt(), split[5], maps, split[7].toInt()));
                    track_list.append(temp);
                }
            }
        }
    } else {
        return QList<QList<track>>();
    }

    return track_list;
}
