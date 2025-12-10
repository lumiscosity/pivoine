#pragma once

#include "lcf/rpg/eventcommand.h"
#include "../third_party/dbstring.h"
#include <QList>
#include <QString>
#include <QWidget>
#include <lcf/dbarray.h>

class Condition {
public:
    inline Condition(const QString &str, lcf::DBArray<int32_t> params) : str(str), params(params) {};
    lcf::rpg::EventCommand command(int32_t depth) {
        lcf::rpg::EventCommand c;
        c.code = int(lcf::rpg::EventCommand::Code::ConditionalBranch);
        c.indent = depth;
        c.string = ToDBString(this->str);
        c.parameters = params;
        return c;
    };

    static Condition from_map(int32_t id) {
        return Condition("", { 5, 2, 4, id, 0, 1 });
    }

    static Condition from_s(int32_t id) {
        return Condition("", { 0, id, 0, 0, 0, 0 });
    }

    static Condition from_rp(int32_t id) {
        return Condition("", { 5, 3, 4, id, 0, 1 });
    }
private:
    QString str;
    lcf::DBArray<int32_t> params;
};

QList<Condition> gen_condition_list(int map, int s, int rp, QString raw);

struct cover {
    cover(QString name, QString author, QList<Condition> cond)
        : name(std::move(name)), author(std::move(author)), cond(std::move(cond)) {}

    QString name;
    QString author;
    QList<Condition> cond;
};

QList<cover> load_cover_list(QWidget *parent);

struct theme {
    theme(QString name, QString author, QString file, QString tiling,
          QList<Condition> cond)
        : name(std::move(name)), author(std::move(author)), file(std::move(file)),
        tiling(std::move(tiling)), cond(std::move(cond)) {}

    QString name;
    QString author;
    QString file;
    QString tiling;
    QList<Condition> cond;
};

QList<theme> load_theme_list(QWidget *parent);
