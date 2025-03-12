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

#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <lcf/lmu/reader.h>
#include "track.h"
#include "unlock_check.h"

int main(int argc, char *argv[])
{
    // load the record player data as a csv
    QList<QList<track>> v;
    QFile f("data.tsv");
    if (f.open(QFile::ReadOnly | QFile::Text)){
        QTextStream in(&f);
        QString s;
        while (in.readLineInto(&s)) {
            QStringList split = s.split("\t");
            if (split.size() >= 7) {
                bool has_id = split[0].isEmpty();
                if (has_id) {
                    v.last().append(track(split[1], split[2], split[3].toInt(), split[4].toInt(), split[5], split[6].toInt(), split[7].toInt()));
                } else {
                    QList<track> temp;
                    temp.append(track(split[1], split[2], split[3].toInt(), split[4].toInt(), split[5], split[6].toInt(), split[7].toInt()));
                    v.append(temp);
                }
            }
        }
        v.pop_front();
    } else {
        return 1;
    }

    unlock_check(v);

    return 0;
}
