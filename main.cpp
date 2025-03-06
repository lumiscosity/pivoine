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

void gentree(int item, int max, QList<track> tracks) {
    if (max > item) {
        return;
    }
}

void branchstart(int type, int id, std::vector<lcf::rpg::EventCommand> &v) {
    lcf::rpg::EventCommand c;
    c.code = int(lcf::rpg::EventCommand::Code::ConditionalBranch);
    c.parameters = {
        5,
        type == 0 ? 2 : 3,
        4,
        id
    };
    v.push_back(c);

    lcf::rpg::EventCommand mul9;
    mul9.indent = 1;
    mul9.code = int(lcf::rpg::EventCommand::Code::ControlVars);
    mul9.parameters = {
        0, 9, 9, 3, 0, 0, 0
    };
    v.push_back(mul9);

    lcf::rpg::EventCommand branchelse;
    branchelse.code = int(lcf::rpg::EventCommand::Code::ElseBranch);
    v.push_back(branchelse);

    lcf::rpg::EventCommand mul8;
    mul8.indent = 1;
    mul8.code = int(lcf::rpg::EventCommand::Code::ControlVars);
    mul8.parameters = {
        0, 8, 8, 3, 0, 0, 0
    };
    v.push_back(mul8);

    lcf::rpg::EventCommand branchend;
    branchend.code = int(lcf::rpg::EventCommand::Code::EndBranch);
    v.push_back(branchend);
}

int main(int argc, char *argv[])
{
    // load the record player data as a csv
    QList<QList<track>> v;
    QFile f("data.tsv");
    if (f.open(QFile::ReadOnly | QFile::Text)){
        QTextStream in(&f);
        QString s;
        while (in.readLineInto(&s)) {
            QStringList split = s.split("   ");
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
    // construct the holding event
    lcf::rpg::Event e;
    e.ID = 18;
    e.y = 7;
    e.name = lcf::DBString("unlock check");

    lcf::rpg::EventCommand mul9;
    mul9.code = int(lcf::rpg::EventCommand::Code::ControlVars);
    mul9.parameters = {
        0, 9, 9, 3, 0, 0, 0
    };

    int counter = 0;
    for (QList<track> i : v) {
        counter++;
        lcf::rpg::EventPage p;
        p.ID = counter;
        for (track j : i) {
            if (j.map_id != 0 || j.skill != 0) {
                branchstart(int(j.skill != 0), j.skill != 0 ? j.skill : j.map_id, p.event_commands);
            } else {
                p.event_commands.push_back(mul9);
            }
        }
        e.pages.push_back(p);
    }

    // replace event id 18
    counter = 0;
    if (QFile::exists("Map0007.lmu")) {
        std::unique_ptr<lcf::rpg::Map> map = lcf::LMU_Reader::Load("Map0007.lmu", "UTF-8");
        for (auto i : map->events) {
            if (i.ID == 18) {
                map->events[counter] = e;
                break;
            }
            counter++;
        }
        lcf::LMU_Reader::Save("Map0007.lmu", *map, lcf::EngineVersion::e2k3, "UTF-8");
    } else {
        return 1;
    }

    return 0;
}
