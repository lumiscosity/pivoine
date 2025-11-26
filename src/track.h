/*
 * This file is part of Pivoine.
 *
 * Pivoine is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Pivoine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Pivoine. If not, see <http://www.gnu.org/licenses/>.
 */

#include <QFileDialog>
#include <QList>
#include <qstring.h>

#pragma once

struct track {
    track(QString track_file, QString artist, int volume, int speed, QString location, QList<int> map_id, int skill = 0) : track_file(track_file), artist(artist), volume(volume), speed(speed), location(location), map_id(map_id), skill(skill) {}
    QString track_file;
    QString artist;
    int volume;
    int speed;
    QString location;
    QList<int> map_id;
    int skill = 0;
};

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
