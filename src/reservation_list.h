#pragma once

#include "../third_party/dbstring.h"
#include "lcf/ldb/reader.h"
#include "lcf/lmt/reader.h"
#include <QFile>
#include <QFileDialog>
#include <QWidget>
#include <lcf/rpg/treemap.h>
#include <lcf/rpg/database.h>

int run_reservation_list(std::string project, QWidget *parent) {
    QFile f(QFileDialog::getSaveFileName(parent, "Reservation list file destination", "names.txt", "names.txt (*.txt)"));
    if (f.open(QFile::WriteOnly | QFile::Text)){
        QTextStream out(&f);
        std::unique_ptr<lcf::rpg::TreeMap> maptree = lcf::LMT_Reader::Load(project + "/RPG_RT.lmt", "UTF-8");
        std::unique_ptr<lcf::rpg::Database> database = lcf::LDB_Reader::Load(project + "/RPG_RT.ldb", "UTF-8");

        out << "MAPS,"+QString::number(maptree->maps.size()-1)+"\n";
        for (lcf::rpg::MapInfo i : maptree->maps) {
            // ignore root map
            if (i.ID != 0) {
                QString name = ToQString(i.name);
                // remove leading ID
                if (name.size() <= 4) {
                    out << "\n";
                } else {
                    name = name.slice(4).trimmed();
                    if (name.startsWith("-")) {
                        name = name.slice(1).trimmed();
                    }
                    out << name+"\n";
                }
            }
        }

        out << "SWITCHES,"+QString::number(database->switches.size())+"\n";
        for (lcf::rpg::Switch i : database->switches) {
            out << ToQString(i.name).trimmed()+"\n";
        }

        out << "VARIABLES,"+QString::number(database->variables.size())+"\n";
        for (lcf::rpg::Variable i : database->variables) {
            out << ToQString(i.name).trimmed()+"\n";
        }

        // Saving the database without Maniacs clears the stringvar chunk
        // Commenting this out until this is resolved

        // out << "STRING VARIABLES,"+QString::number(database->maniac_string_variables.size())+"\n";
        // for (lcf::rpg::StringVariable i : database->maniac_string_variables) {
        //     out << ToQString(i.name).trimmed()+"\n";
        // }

        out << "TILESETS,"+QString::number(database->chipsets.size())+"\n";
        for (lcf::rpg::Chipset i : database->chipsets) {
            out << ToQString(i.name).trimmed()+"\n";
        }

        out << "BATTLE ANIMATIONS,"+QString::number(database->animations.size())+"\n";
        for (lcf::rpg::Animation i : database->animations) {
            out << ToQString(i.name).trimmed()+"\n";
        }

        out << "TERRAINS,"+QString::number(database->terrains.size())+"\n";
        for (lcf::rpg::Terrain i : database->terrains) {
            out << ToQString(i.name).trimmed()+"\n";
        }

        f.close();
    } else {
        return 1;
    }
    return 0;
}
