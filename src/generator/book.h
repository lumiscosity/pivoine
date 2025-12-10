#pragma once

#include "../data/book_data.h"
#include "../text.h"
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

QPixmap gen_book_name(const Font &font, int id, QString name) {
    QPixmap p(320, 24);
    p.fill(Qt::transparent);
    QPainter painter(&p);
    QPoint anchor(6, 5);
    QColor text_color = Qt::white;
    draw_text_l(font, painter, anchor, name, text_color);
    return p;
}

QPixmap gen_book_author(const Font &font, int id, QString name) {
    QPixmap p(320, 24);
    p.fill(Qt::transparent);
    QPainter painter(&p);
    QPoint anchor(314, 5);
    QColor text_color = Qt::white;
    draw_text_r(font, painter, anchor, name, text_color);
    return p;
}
