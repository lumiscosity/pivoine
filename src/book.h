#pragma once

#include "book_data.h"
#include "book_text.h"
#include <lcf/rpg/event.h>
#include <lcf/rpg/eventcommand.h>
#include <lcf/rpg/eventpage.h>
#include <QFile>
#include <QFileDialog>
#include <QRegularExpression>
#include <QPainter>


std::vector<lcf::rpg::EventCommand> gen_check(QList<Condition> &list, int32_t depth) {
    std::vector<lcf::rpg::EventCommand> v;
    lcf::rpg::EventCommand switchon;
    switchon.code = int(lcf::rpg::EventCommand::Code::ControlSwitches);
    switchon.indent = depth + 1;
    switchon.parameters = { 0, 1, 1, 0 };
    if (!list.isEmpty()){
        v.push_back(list.last().command(depth));
        list.pop_back();
    } else {
        // handling for automatic unlock
        switchon.indent = 0;
        v.push_back(switchon);
        return v;
    }
    if (list.isEmpty()) {
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
            if (!i.isEmpty()){
                QStringList temp = i.last(i.length() - 1).split(",");
                Condition next_cond(temp[0].last(temp[0].length() - 1).first(temp[0].length() - 2), {temp[1].toInt(), temp[2].toInt(), temp[3].toInt(), temp[4].toInt(), temp[5].toInt(), temp[6].toInt()});
                c.push_back(next_cond);
            }
        }
    }
    return c;
}

QPixmap gen_book_name(const BookFont &font, int id, QString name) {
    QPixmap p(320, 24);
    p.fill(Qt::transparent);
    QPainter painter(&p);
    QPoint anchor(6, 5);
    QColor text_color = Qt::white;
    draw_small_text_l(font, painter, anchor, name, text_color);
    return p;
}

QPixmap gen_book_author(const BookFont &font, int id, QString name) {
    QPixmap p(320, 24);
    p.fill(Qt::transparent);
    QPainter painter(&p);
    QPoint anchor(314, 5);
    QColor text_color = Qt::white;
    draw_small_text_r(font, painter, anchor, name, text_color);
    return p;
}
