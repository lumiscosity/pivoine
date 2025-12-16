#include "book_data.h"

#include <QFileDialog>
#include <QMessageBox>

SkillCondition Condition::get_skill() {
    if (this->params.size() >= 3 && this->params[0] == 5 && this->params[2] == 4) {
        return SkillCondition {
            this->params[1],
            this->params[3]
        };
    } else {
        return SkillCondition{};
    }
}

SwitchCondition Condition::get_switch() {
    if (this->params.size() >= 3 && this->params[0] == 0 && this->params[3] == 0) {
        return SwitchCondition {
            this->params[1],
            this->params[2] == 0
        };
    } else {
        return SwitchCondition{};
    }
}

VariableCondition Condition::get_variable() {
    if (this->params.size() >= 3 && this->params[0] == 0 && this->params[3] == 0) {
        return VariableCondition {
            this->params[1],
            this->params[2] == 1,
            static_cast<VariableOperator>(this->params[4]),
            this->params[3]
        };
    } else {
        return VariableCondition{};
    }
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

QList<cover> load_cover_list(QWidget *parent) {
    QList<cover> cover_list;
    QFile f(QFileDialog::getOpenFileName(parent, "Select the cover data", "", "Tab separated values (*.tsv)"));
    if (f.open(QFile::ReadOnly | QFile::Text)){
        QTextStream in(&f);
        QString s;
        QStringList validate = in.readLine().split("\t");
        if (
            validate[0] != "ID" ||
            validate[1] != "Name" ||
            validate[2] != "Author" ||
            validate[3] != "Map" ||
            validate[4] != "Switch" ||
            validate[5] != "RP Unlock" ||
            validate[6] != "Custom" ||
            validate[7] != "Info"
            ) {
            QMessageBox::critical(parent, "Error", "Invalid header detected! This might not be a book cover data .tsv file.");
            return QList<cover>();
        }

        while (in.readLineInto(&s)) {
            QString s_copy = s;
            if (!s_copy.right(s_copy.length()-4).replace("\t", "").isEmpty()){
                QStringList split = s.split("\t");
                if (split.size() >= 7) {
                    cover_list.append(cover(split[1].trimmed(), split[2].trimmed(), gen_condition_list(split[3].toInt(), split[4].toInt(), split[5].toInt(), split[6])));
                }
            }
        }
    } else {
        return QList<cover>();
    }
    return cover_list;
}

QList<theme> load_theme_list(QWidget *parent) {
    QList<theme> theme_list;
    QFile theme_f(QFileDialog::getOpenFileName(parent, "Select the theme data", "", "Tab separated values (*.tsv)"));
    if (theme_f.open(QFile::ReadOnly | QFile::Text)){
        QTextStream in(&theme_f);
        QString s;
        QStringList validate = in.readLine().split("\t");
        if (
            validate[0] != "ID" ||
            validate[1] != "Name" ||
            validate[2] != "Author" ||
            validate[3] != "Filename" ||
            validate[4] != "BG Mode" ||
            validate[5] != "Switch" ||
            validate[6] != "Custom" ||
            validate[7] != "Info"
            ) {
            QMessageBox::critical(parent, "Error", "Invalid header detected! This might not be a menu theme data .tsv file.");
        }

        while (in.readLineInto(&s)) {
            QString s_copy = s;
            if (!s_copy.right(s_copy.length()-4).replace("\t", "").isEmpty()){
                QStringList split = s.split("\t");
                if (split.size() >= 7) {
                    theme_list.append(theme(split[1].trimmed(), split[2].trimmed(), split[3].trimmed(), split[4].trimmed(), gen_condition_list(0, split[5].toInt(), 0, split[6])));
                }
            }
        }
    } else {
        return QList<theme>();
    }
    return theme_list;
}
