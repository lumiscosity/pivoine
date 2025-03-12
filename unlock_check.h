#pragma once

#include "lcf/lmu/reader.h"
#include "track.h"

#include <lcf/rpg/event.h>
#include <lcf/rpg/map.h>

#include <qfile.h>

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

int unlock_check(QList<QList<track>>& track_list) {
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
    for (QList<track> i : track_list) {
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
