#pragma once

#include "book.h"
#include "book_data.h"
#include "lcf/lmu/reader.h"

#include <QFile>
#include <QFileDialog>
#include <QWidget>
#include <QPainter>
#include <QRgb>

#include <lcf/rpg/event.h>

QPixmap transparentify(QString filename) {
    QImage raw_source(filename);
    QImage new_source(raw_source.width(), raw_source.height(), QImage::Format_RGBA8888);
    new_source.fill(Qt::transparent);
    if (raw_source.isNull()) {
        throw std::invalid_argument(filename.toStdString());
    }
    QPixmap source(raw_source.width(), raw_source.height());
    if (raw_source.format() == QImage::Format_Indexed8) {
        for (int x = 0; x < raw_source.width(); x++) {
            for (int y = 0; y < raw_source.height(); y++) {
                if (raw_source.pixelIndex(x, y) != 0) {
                    new_source.setPixel(x, y, raw_source.color(raw_source.pixelIndex(x, y)));
                }
            }
        }
        source.convertFromImage(new_source);
    } else {
        source.convertFromImage(raw_source);
    }
    return source;
}

QPixmap generate_theme_preview(QString filename) {
    QPixmap dest(32, 32);
    dest.fill(Qt::transparent);
    QPixmap source = transparentify(filename);
    QPainter painter;
    painter.begin(&dest);

    painter.drawPixmap(QRect(0, 0, 272, 32), source.copy(QRect(0, 0, 32, 32)));
    painter.drawPixmap(QRect(0, 0, 272, 32), source.copy(QRect(32, 0, 32, 32)));

    return dest;
}

QPixmap generate_getbox(QString filename) {
    QPixmap dest(272, 32);
    dest.fill(Qt::transparent);
    QPixmap source = transparentify(filename);
    QPainter painter;
    painter.begin(&dest);

    // draw background
    painter.drawTiledPixmap(QRect(0, 0, 272, 32), source.copy(QRect(0, 0, 32, 32)));
    // draw corners
    painter.drawPixmap(QRect(0, 0, 8, 8), source.copy(QRect(32, 0, 8, 8)));  // top left
    painter.drawPixmap(QRect(264, 0, 8, 8), source.copy(QRect(56, 0, 8, 8)));  // top right
    painter.drawPixmap(QRect(0, 24, 8, 8), source.copy(QRect(32, 24, 8, 8)));  // bottom left
    painter.drawPixmap(QRect(264, 24, 8, 8), source.copy(QRect(56, 24, 8, 8)));  // bottom right
    // draw edges
    painter.drawTiledPixmap(QRect(8, 0, 256, 8), source.copy(QRect(40, 0, 16, 8))); // top
    painter.drawPixmap(QRect(0, 8, 8, 16), source.copy(QRect(32, 8, 8, 16))); // left
    painter.drawPixmap(QRect(264, 8, 8, 16), source.copy(QRect(56, 8, 8, 16))); // right
    painter.drawTiledPixmap(QRect(8, 24, 256, 8), source.copy(QRect(40, 24, 16, 8))); // bottom
    painter.end();

    return dest;
}

int run_theme(std::string project, QWidget *parent) {
    // load the theme data as a csv
    QList<theme> theme_list;
    QFile theme_f(QFileDialog::getOpenFileName(parent, "Select the theme data", "", "Tab separated values (*.tsv)"));
    if (theme_f.open(QFile::ReadOnly | QFile::Text)){
        QTextStream in(&theme_f);
        QString s;
        in.readLine();
        while (in.readLineInto(&s)) {
            QStringList split = s.split("\t");
            if (split.size() >= 7) {
                theme_list.append(theme(split[1].trimmed(), split[2].trimmed(), split[3].trimmed(), split[4].trimmed(), gen_condition_list(0, split[5].toInt(), 0, split[6])));
            }
        }
    } else {
        return 1;
    }

    lcf::rpg::EventCommand switchoff;
    switchoff.code = int(lcf::rpg::EventCommand::Code::ControlSwitches);
    switchoff.indent = 0;
    switchoff.parameters = { 0, 1, 1, 1 };

    // generate theme unlock check event
    lcf::rpg::Event theme_unlock_event;
    theme_unlock_event.ID = 25;
    theme_unlock_event.name = "GET_THEME_STATUS";
    theme_unlock_event.x = 19;
    theme_unlock_event.y = 14;
    int counter = 1;
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
        page.event_commands.push_back(setter);
        theme_equip_event.pages.push_back(page);
    }

    // replace events
    std::unique_ptr<lcf::rpg::Map> map = lcf::LMU_Reader::Load(project + "/Map0009.lmu", "UTF-8");
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
                    && j.parameters[1] == 115) {
                    j.parameters[5] = theme_count - 1;
                    break;
                }
            }
            break;
        }
        counter++;
    }
    lcf::LMU_Reader::Save(project + "/Map0009.lmu", *map, lcf::EngineVersion::e2k3, "UTF-8");

    // generate assets (getbox, book preview, book name/author)
    counter = 0;
    for (theme i : theme_list) {
        QString path = QString::fromStdString(project) + "/System/" + i.file + ".png";
        generate_getbox(path)
            .save(QString::fromStdString(project) + "/Picture/getbox_" + QString::number(counter).rightJustified(4, QChar(48)) + ".png");
        generate_theme_preview(path)
            .save(QString::fromStdString(project) + "/Picture/book/mpreview" + QString::number(counter).rightJustified(4, QChar(48)) + ".png");
        counter++;
    }

    return 0;
}
