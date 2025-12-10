#pragma once

#include "../third_party/dbstring.h"
#include "lcf/lmu/reader.h"
#include "snack.h"
#include <string>
#include <QDialog>
#include <QFileDialog>

lcf::rpg::EventCommand gen_giant_vm_pic(snack s) {
    lcf::rpg::EventCommand c;
    c.code = int(lcf::rpg::EventCommand::Code::ShowPicture);
    c.indent = 0;
    c.string = ToDBString(s.filename);
    c.parameters = {
        2, 1, 3, 4, 1, 100, 0, 1, 100, 100, 100, 100, 0,
        536870912, 0, 5830152, 0, 1, 0, 0, 0, 0,
        s.sheet_w, s.sheet_h, 0, s.sheet_pos, 0, 2, 0, 113
    };

    return c;
}

std::vector<lcf::rpg::EventCommand> gen_giant_vm_check(int id) {
    std::vector<lcf::rpg::EventCommand> v({});

    lcf::rpg::EventCommand branch_header;
    branch_header.code = int(lcf::rpg::EventCommand::Code::ConditionalBranch);
    branch_header.indent = 0;
    branch_header.parameters = {
        5, 4, 4, id, 0, 0
    };
    v.push_back(branch_header);

    lcf::rpg::EventCommand switch_set;
    switch_set.code = int(lcf::rpg::EventCommand::Code::ControlSwitches);
    switch_set.indent = 1;
    switch_set.parameters = {
        0, 5, 5, 0
    };
    v.push_back(switch_set);

    lcf::rpg::EventCommand branch_footer;
    branch_footer.code = int(lcf::rpg::EventCommand::Code::EndBranch);
    branch_footer.indent = 0;
    v.push_back(branch_footer);

    return v;
}

int run_giant_vm(std::string project, QWidget *parent) {
    // load the record player data as a tsv
    QList<snack> snack_list = load_snack_list(parent);
    if (snack_list.isEmpty()) {
        return 1;
    }

    int snack_count = snack_list.size();

    int counter = 0;
    std::unique_ptr<lcf::rpg::Map> map = lcf::LMU_Reader::Load(project + "/Map0008.lmu", "UTF-8");

    // generate unlock checks
    lcf::rpg::Event unlock_event;
    unlock_event.ID = 30;
    unlock_event.name = "check unlocked items";
    unlock_event.x = 1;
    unlock_event.y = 0;

    counter = 1;
    for (snack j : snack_list) {
        lcf::rpg::EventPage p;
        p.ID = counter;
        p.event_commands = gen_giant_vm_check(counter);
        unlock_event.pages.push_back(p);
        counter++;
    }

    // generate picture show event
    lcf::rpg::Event pic_event;
    pic_event.ID = 31;
    pic_event.name = "VM Item Pic Table";
    pic_event.x = 3;
    pic_event.y = 25;

    counter = 1;
    for (snack j : snack_list) {
        lcf::rpg::EventPage p;
        p.ID = counter;
        p.event_commands.push_back(gen_giant_vm_pic(j));
        pic_event.pages.push_back(p);
        counter++;
    }

    // replace events
    counter = 0;
    for (auto i : map->events) {
        if (i.ID == 30) {
            map->events[counter] = unlock_event;
            break;
        }
        counter++;
    }
    counter = 0;
    for (auto i : map->events) {
        if (i.ID == 31) {
            map->events[counter] = pic_event;
            break;
        }
        counter++;
    }

    // bump max count
    counter = 0;
    for (auto i : map->events) {
        if (i.ID == 27) {
            for (lcf::rpg::EventCommand &j : map->events[counter].pages[0].event_commands) {
                if (j.code == int(lcf::rpg::EventCommand::Code::ControlVars)
                    && j.parameters[1] == 20) {
                    j.parameters[5] = snack_count - 1;
                    break;
                }
            }
            break;
        }
        counter++;
    }

    lcf::LMU_Reader::Save(project + "/Map0008.lmu", *map, lcf::EngineVersion::e2k3, "UTF-8");

    return 0;
}
