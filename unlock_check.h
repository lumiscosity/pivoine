#pragma once

#include "lcf/lmu/reader.h"
#include "track.h"

#include <lcf/rpg/event.h>
#include <lcf/rpg/map.h>

#include <qfile.h>

void gen_unlock_check_header(int type, int id, std::vector<lcf::rpg::EventCommand> &v) {
    // conditional branch header
    lcf::rpg::EventCommand c;
    c.indent = 0;
    c.code = int(lcf::rpg::EventCommand::Code::ConditionalBranch);
    c.parameters = {
        5,
        type == 0 ? 2 : 3,
        4,
        id,
        0,
        1
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
    branchelse.indent = 0;
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
    branchend.indent = 0;
    branchend.code = int(lcf::rpg::EventCommand::Code::EndBranch);
    v.push_back(branchend);
}

void gen_play_header(std::vector<lcf::rpg::EventCommand> &v) {
    lcf::rpg::EventCommand recalc_id;
    recalc_id.indent = 0;
    recalc_id.code = int(lcf::rpg::EventCommand::Code::CallEvent);
    recalc_id.parameters = { 1, 13, 1 };
    v.push_back(recalc_id);

    lcf::rpg::EventCommand reset_branch;
    reset_branch.indent = 0;
    reset_branch.code = int(lcf::rpg::EventCommand::Code::ConditionalBranch);
    reset_branch.parameters = { 1, 1, 1, 4, 5, 0 };
    v.push_back(reset_branch);

    lcf::rpg::EventCommand reset_setter;
    reset_setter.indent = 1;
    reset_setter.code = int(lcf::rpg::EventCommand::Code::ControlVars);
    reset_setter.parameters = { 0, 3, 3, 0, 0, -1, 0 };
    v.push_back(reset_setter);

    lcf::rpg::EventCommand reset_endbranch;
    reset_endbranch.indent = 0;
    reset_endbranch.code = int(lcf::rpg::EventCommand::Code::EndBranch);
    v.push_back(reset_endbranch);

    lcf::rpg::EventCommand label1;
    label1.indent = 0;
    label1.code = int(lcf::rpg::EventCommand::Code::Label);
    label1.parameters = { 1 };
    v.push_back(label1);

    lcf::rpg::EventCommand add1;
    add1.indent = 0;
    add1.code = int(lcf::rpg::EventCommand::Code::ControlVars);
    add1.parameters = { 0, 3, 3, 1, 0, 1, 0 };
    v.push_back(add1);
}

void gen_play_footer(std::vector<lcf::rpg::EventCommand> &v) {
    lcf::rpg::EventCommand label3;
    label3.indent = 0;
    label3.code = int(lcf::rpg::EventCommand::Code::Label);
    label3.parameters = { 3 };
    v.push_back(label3);

    lcf::rpg::EventCommand set19to1;
    set19to1.indent = 0;
    set19to1.code = int(lcf::rpg::EventCommand::Code::ControlVars);
    set19to1.parameters = { 0, 19, 19, 0, 1, 1, 0 };
    v.push_back(set19to1);

    lcf::rpg::EventCommand mult19;
    mult19.indent = 0;
    mult19.code = int(lcf::rpg::EventCommand::Code::ControlVars);
    mult19.parameters = { 0, 19, 19, 3, 0, 100, 0 };
    v.push_back(mult19);

    lcf::rpg::EventCommand addto18;
    addto18.indent = 0;
    addto18.code = int(lcf::rpg::EventCommand::Code::ControlVars);
    addto18.parameters = { 0, 18, 18, 1, 1, 19, 0 };
    v.push_back(addto18);

    lcf::rpg::EventCommand bgupdate;
    bgupdate.indent = 0;
    bgupdate.code = int(lcf::rpg::EventCommand::Code::CallEvent);
    bgupdate.parameters = { 1, 16, 1 };
    v.push_back(bgupdate);

    lcf::rpg::EventCommand label2;
    label2.indent = 0;
    label2.code = int(lcf::rpg::EventCommand::Code::Label);
    label2.parameters = { 2 };
    v.push_back(label2);
}

int record_player(QList<QList<track>>& track_list) {
    // construct the holding events
    lcf::rpg::Event unlock_check_event;
    unlock_check_event.ID = 18;
    unlock_check_event.y = 7;
    unlock_check_event.name = lcf::DBString("unlock check");

    lcf::rpg::Event play_event;
    play_event.ID = 6;
    unlock_check_event.y = 2;
    unlock_check_event.name = lcf::DBString("play");

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
                gen_unlock_check_header(int(j.skill != 0), j.skill != 0 ? j.skill : j.map_id, p.event_commands);
            } else {
                p.event_commands.push_back(mul9);
            }
        }
        unlock_check_event.pages.push_back(p);
    }

    // replace event id 18
    counter = 0;
    if (QFile::exists("Map0007.lmu")) {
        std::unique_ptr<lcf::rpg::Map> map = lcf::LMU_Reader::Load("Map0007.lmu", "UTF-8");
        for (auto i : map->events) {
            if (i.ID == 18) {
                map->events[counter] = unlock_check_event;
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
