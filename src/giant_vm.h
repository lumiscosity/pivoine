#pragma once

#include "lcf/lmu/reader.h"
#include <string>
#include <QDialog>
#include <QFileDialog>

struct snack {
    QString filename;
    int sheet_x;
    int sheet_y;
    int sheet_offset;
};

int run_giant_vm(std::string project, QWidget *parent) {
    // load the record player data as a tsv
    QList<snack> snack_list;
    QFile f(QFileDialog::getOpenFileName(parent, "Select the giant VM data", "", "Tab separated values (*.tsv)"));
    if (f.open(QFile::ReadOnly | QFile::Text)){
        QTextStream in(&f);
        QString s;
        while (in.readLineInto(&s)) {
            QStringList split = s.split("\t");
            if (split.size() >= 3) {
                QStringList sheet_raw = split[2].split(",");
                QList<int> sheet;
                if (sheet_raw.size() >= 3) {
                    sheet = {sheet_raw[0].toInt(), sheet_raw[1].toInt(), sheet_raw[2].toInt()};
                } else {
                    sheet = {-1, -1, -1};
                }
                snack_list.append(snack{split[1], sheet[0], sheet[1], sheet[2]});
            }
        }
        snack_list.pop_front();
    } else {
        return 1;
    }

    int snack_count = snack_list.size();

    int counter = 0;
    std::unique_ptr<lcf::rpg::Map> map = lcf::LMU_Reader::Load(project + "/Map0008.lmu", "UTF-8");

    // TODO: Generate pointer map for var id to skill id for unlock checks
    // does maniacs just have a command for this? let's just use that if it does

    // TODO: Generate picture show event

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
