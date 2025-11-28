#include "track.h"

QList<QList<track>> load_track_list(QWidget *parent) {
    QList<QList<track>> track_list;
    QFile f(QFileDialog::getOpenFileName(parent, "Select the record player data", "", "Tab separated values (*.tsv)"));
    if (f.open(QFile::ReadOnly | QFile::Text)){
        QTextStream in(&f);
        QString s;
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
        track_list.pop_front();
    } else {
        return QList<QList<track>>();
    }

    return track_list;
}
