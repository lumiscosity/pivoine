#pragma once

#include "book_data.h"
#include "lcf/lmu/reader.h"
#include <lcf/rpg/event.h>
#include <lcf/rpg/eventcommand.h>
#include <lcf/rpg/eventpage.h>
#include <QFile>
#include <QFileDialog>
#include <QRegularExpression>


std::vector<lcf::rpg::EventCommand> gen_check(QList<Condition> &list, int32_t depth) {
    std::vector<lcf::rpg::EventCommand> v;
    v.push_back(list.last().command(depth));
    if (!list.isEmpty()){
        list.pop_back();
    }
    if (list.isEmpty()) {
        lcf::rpg::EventCommand switchon;
        switchon.code = int(lcf::rpg::EventCommand::Code::ControlSwitches);
        switchon.indent = depth + 1;
        switchon.parameters = { 0, 1, 1, 0 };
        v.push_back(switchon);
    } else {
        for (auto i : gen_check(list, depth + 1)) {
            v.push_back(i);
        }
    }
    lcf::rpg::EventCommand branchend;
    branchend.code = int(lcf::rpg::EventCommand::Code::EndBranch);
    branchend.indent = depth;
    v.push_back(branchend);
    return v;
}

QList<Condition> gen_condition_list(int map, int s, int rp, QString raw) {
    QList<Condition> c;
    if (map) {
        c.append(Condition::from_map(map));
        return c;
    }
    if (s) {
        c.append(Condition::from_s(s));
        return c;
    }
    if (rp) {
        c.append(Condition::from_rp(rp));
        return c;
    }
    if (!raw.isEmpty()){
        for (QString i : raw.split("]")) {
            QStringList temp = i.last(i.length() - 1).split(",");
            Condition next_cond(temp[0].last(temp[0].length() - 1).first(temp[0].length() - 2), {temp[1].toInt(), temp[2].toInt(), temp[3].toInt(), temp[4].toInt(), temp[5].toInt(), temp[6].toInt()});
        }
    }
    return c;
}

int book(std::string project, QWidget *parent) {
    // load the cover data as a csv
    QList<cover> cover_list;
    QFile f(QFileDialog::getOpenFileName(parent, "Select the cover data", "", "Tab separated values (*.tsv)"));
    if (f.open(QFile::ReadOnly | QFile::Text)){
        QTextStream in(&f);
        QString s;
        while (in.readLineInto(&s)) {
            QStringList split = s.split("\t");
            if (split.size() >= 7) {
                cover_list.append(cover(split[1].trimmed(), split[2].trimmed(), gen_condition_list(split[3].toInt(), split[4].toInt(), split[5].toInt(), split[6])));
            }
        }
    } else {
        return 1;
    }

    lcf::rpg::EventCommand switchoff;
    switchoff.code = int(lcf::rpg::EventCommand::Code::ControlSwitches);
    switchoff.indent = 0;
    switchoff.parameters = { 0, 1, 1, 1 };

    // generate cover unlock check event
    lcf::rpg::Event cover_event;
    cover_event.ID = 22;
    cover_event.name = "GET_COVER_STATUS";
    cover_event.x = 18;
    cover_event.y = 14;
    int counter = 1;
    for (cover i : cover_list) {
        lcf::rpg::EventPage page;
        page.ID = counter;
        counter++;
        page.event_commands.push_back(switchoff);
        for (auto j : gen_check(i.cond, 0)) {
            page.event_commands.push_back(j);
        }
        cover_event.pages.push_back(page);
    }
    int cover_count = counter;

    // load the theme data as a csv
    QList<theme> theme_list;
    QFile theme_f(QFileDialog::getOpenFileName(parent, "Select the theme data", "", "Tab separated values (*.tsv)"));
    if (theme_f.open(QFile::ReadOnly | QFile::Text)){
        QTextStream in(&theme_f);
        QString s;
        while (in.readLineInto(&s)) {
            QStringList split = s.split("\t");
            if (split.size() >= 7) {
                theme_list.append(theme(split[1].trimmed(), split[2].trimmed(), split[3].trimmed(), split[4].trimmed(), gen_condition_list(0, split[5].toInt(), 0, split[6])));
            }
        }
    } else {
        return 1;
    }

    // generate theme unlock check event
    lcf::rpg::Event theme_unlock_event;
    theme_unlock_event.ID = 25;
    theme_unlock_event.name = "GET_THEME_STATUS";
    theme_unlock_event.x = 19;
    theme_unlock_event.y = 14;
    counter = 1;
    for (theme i : theme_list) {
        lcf::rpg::EventPage page;
        page.ID = counter;
        counter++;
        page.event_commands.push_back(switchoff);
        for (auto j : gen_check(i.cond, 0)) {
            page.event_commands.push_back(j);
        }
        theme_unlock_event.pages.push_back(page);
    }
    int theme_count = counter;

    // generate theme equip event
    lcf::rpg::Event theme_equip_event;
    theme_equip_event.ID = 26;
    theme_equip_event.name = "SET_MENU_THEME";
    theme_equip_event.x = 4;
    theme_equip_event.y = 14;
    counter = 1;
    for (theme i : theme_list) {
        lcf::rpg::EventPage page;
        page.ID = counter;
        counter++;
        lcf::rpg::EventCommand setter;
        setter.code = int(lcf::rpg::EventCommand::Code::ChangeSystemGraphics);
        setter.indent = 0;
        setter.string = ToDBString(i.file);
        setter.parameters = {i.tiling == "tile", 1};
        page.event_commands.push_back(switchoff);
        for (auto j : gen_check(i.cond, 0)) {
            page.event_commands.push_back(j);
        }
        theme_equip_event.pages.push_back(page);
    }

    // replace events
    counter = 0;
    std::unique_ptr<lcf::rpg::Map> map = lcf::LMU_Reader::Load(project + "/Map0009.lmu", "UTF-8");
    for (auto i : map->events) {
        if (i.ID == 22) {
            map->events[counter] = cover_event;
            break;
        }
        counter++;
    }
    counter = 0;
    for (auto i : map->events) {
        if (i.ID == 25) {
            map->events[counter] = theme_unlock_event;
            break;
        }
        counter++;
    }
    counter = 0;
    for (auto i : map->events) {
        if (i.ID == 26) {
            map->events[counter] = theme_equip_event;
            break;
        }
        counter++;
    }
    // bump max counts
    counter = 0;
    for (auto i : map->events) {
        if (i.ID == 36) {
            for (lcf::rpg::EventCommand &j : map->events[counter].pages[0].event_commands) {
                if (j.code == int(lcf::rpg::EventCommand::Code::ControlVars)
                    && j.parameters[1] == 111) {
                    j.parameters[5] = cover_count;
                }
                if (j.code == int(lcf::rpg::EventCommand::Code::ControlVars)
                    && j.parameters[1] == 115) {
                    j.parameters[5] = theme_count;
                }
            }
            break;
        }
        counter++;
    }
    lcf::LMU_Reader::Save("Map0009.lmu", *map, lcf::EngineVersion::e2k3, "UTF-8");


    return 0;
}
