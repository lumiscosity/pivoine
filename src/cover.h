#pragma once

#include "book.h"
#include "book_data.h"
#include "lcf/lmu/reader.h"

#include <QFile>
#include <QFileDialog>
#include <QWidget>

#include <lcf/rpg/event.h>



int run_cover(std::string project, QWidget *parent){
    // load the cover data as a csv
    QList<cover> cover_list;
    QFile f(QFileDialog::getOpenFileName(parent, "Select the cover data", "", "Tab separated values (*.tsv)"));
    if (f.open(QFile::ReadOnly | QFile::Text)){
        QTextStream in(&f);
        QString s;
        in.readLine();
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
    // bump max counts
    counter = 0;
    for (auto i : map->events) {
        if (i.ID == 36) {
            for (lcf::rpg::EventCommand &j : map->events[counter].pages[0].event_commands) {
                if (j.code == int(lcf::rpg::EventCommand::Code::ControlVars)
                    && j.parameters[1] == 111) {
                    j.parameters[5] = cover_count - 1;
                    break;
                }
            }
            break;
        }
        counter++;
    }
    lcf::LMU_Reader::Save(project + "/Map0009.lmu", *map, lcf::EngineVersion::e2k3, "UTF-8");


    return 0;
}
