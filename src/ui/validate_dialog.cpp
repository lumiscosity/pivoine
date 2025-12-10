#include "../third_party/dbstring.h"
#include "validate_dialog.h"
#include "lcf/lmu/reader.h"
#include "track.h"
#include "ui_validate_dialog.h"

#include <QMessageBox>

struct call_tuple {
    int skill;
    QSet<int> map = QSet<int>();
    int trigger_map;
    int trigger_id;
    int trigger_x;
    int trigger_y;
};

void insert_or_create(QHash<int, QSet<int>> &dest, int key, int value) {
    // we have defaultdict at home
    if (dest.keys().contains(key)) {
        dest.value(key).insert(value);
    } else {
        dest.insert(key, QSet<int>({value}));
    }
}

ValidateDialog::ValidateDialog(QString project, QWidget *parent)
    : QDialog(parent)
    , project(project), ui(new Ui::ValidateDialog)
{
    ui->setupUi(this);
}

ValidateDialog::~ValidateDialog()
{
    delete ui;
}

void ValidateDialog::push(QString string) {
    ui->plainTextEdit->appendPlainText(string);
}

void ValidateDialog::validate_record_player()
{
    QList<QList<track>> tracks = load_track_list(this);
    QString check_path = this->project + "/Music/";
    QHash<int, QList<int>> tsv_skill_ids;

    int co = 0;
    for (QList<track> i : tracks) {
        int ci = 0;
        for (track j : i) {
            // check if the music file exists
            if (!(QFile::exists(check_path + j.track_file + ".wav") ||
                  QFile::exists(check_path + j.track_file + ".mp3") ||
                  QFile::exists(check_path + j.track_file + ".ogg") ||
                  QFile::exists(check_path + j.track_file + ".opus") ||
                  QFile::exists(check_path + j.track_file + ".mid") ||
                  // i'm not adding a full rtp filename list for this exception
                  j.track_file == "SERain" ||
                  j.track_file == "SESea" )) {
                push("- Track " + QString::number(co) + " variation " + QString::number(ci) +
                     " refers to the music file " + j.track_file +
                     ", which is missing!");
            }

            // check if the background file exists
            if (!(QFile::exists(this->project
                                + "/Picture/record_player/background_"
                                + QString::number(co).rightJustified(4, QChar(48))
                                + QString::number(ci).rightJustified(2, QChar(48))
                                + ".png"))) {
                push("- The background for track " + QString::number(co) +
                     " variation " + QString::number(ci) +
                     " is missing!");
            }
            // check if the description file exists
            if (!(QFile::exists(this->project
                                + "/Picture/record_player/description_"
                                + QString::number(co).rightJustified(4, QChar(48))
                                + QString::number(ci).rightJustified(2, QChar(48))
                                + ".png"))) {
                push("- The description for track " + QString::number(co) +
                     " variation " + QString::number(ci) +
                     " is missing!");
            }
            // check for missing skill ids on multi-map setups
            if (j.map_id.size() > 1 && j.skill == 0) {
                push("- Track " + QString::number(co) +
                     " variation " + QString::number(ci) +
                     " covers multiple maps, but has no skill switch!");
            }

            // store used skill ids for later
            if (j.skill != 0) {
                tsv_skill_ids.insert(j.skill, j.map_id);
            }

            ci++;
        }
        co++;
    }


    // check every map for skill ids
    QDir maps_dir(this->project);
    QHash<int, QSet<int>> maps_per_skill;
    QHash<int, QSet<int>> skills_per_map;
    QList<call_tuple> calls;

    maps_dir.setNameFilters(QStringList("*.lmu"));
    for (QFileInfo i : maps_dir.entryInfoList()) {
        std::unique_ptr<lcf::rpg::Map> map = lcf::LMU_Reader::Load(i.absoluteFilePath().toStdString(), "UTF-8");
        int map_id = i.fileName().slice(3,4).toInt();
        for (auto j : map->events) {
            // first pass: find transfer commands
            QSet<int> transfers = QSet<int>({map_id});
            for (auto k : j.pages) {
                for (lcf::rpg::EventCommand l : k.event_commands) {
                    if (l.code == int(lcf::rpg::EventCommand::Code::Teleport)) {
                        transfers.insert(l.parameters[0]);
                    }
                }
            }
            // second pass: populate maps_per_skill, skills_per_map and calls
            for (auto k : j.pages) {
                for (lcf::rpg::EventCommand l : k.event_commands) {
                    if (l.code == int(lcf::rpg::EventCommand::Code::ChangeSkills)) {
                        // ignore the book updater
                        if (!(map_id == 9 && j.ID == 37)) {
                            // only check for changes to the record player actor
                            if (l.parameters[1] == 3 && l.parameters[3] == 0) {
                                call_tuple potential{l.parameters[4], QSet<int>(), map_id, j.ID, j.x, j.y};
                                for (int m : transfers) {
                                    insert_or_create(maps_per_skill, l.parameters[4], m);
                                    insert_or_create(skills_per_map, m, l.parameters[4]);
                                    potential.map.insert(m);
                                }
                                calls.push_back(potential);
                            }
                        }
                    }
                }
            }
        }
        map.reset();
    }


    // check for skills which don't get set
    for (int i : tsv_skill_ids.keys()) {
        if (!maps_per_skill.keys().contains(i)) {
            push("- Skill " + QString::number(i) +
                 " is present on the sheet, but never set!");
        }
    }
    // check for skills which get set in the wrong map and unused skills getting set
    for (call_tuple i : calls) {
        if (tsv_skill_ids.keys().contains(i.skill)) {
            bool found = false;
            for (int j : tsv_skill_ids.value(i.skill)) {
                found = found || i.map.contains(j);
            }
            QString maps;
            for (int j : i.map) {
                maps.append(QString::number(j) + ", ");
            }
            maps.removeLast();
            maps.removeLast();
            if (!found) {
                push("- Skill " + QString::number(i.skill) +
                     " is set in map " + QString::number(i.trigger_map) +
                     " in EV" + QString::number(i.trigger_id) +
                     " at (" + QString::number(i.trigger_x) + "," + QString::number(i.trigger_y) +
                     ") when it shouldn't be!");
            }
        } else {
            push("- Map " + QString::number(i.trigger_map) +
                 " sets unused skill " + QString::number(i.skill) +
                 " in EV" + QString::number(i.trigger_id) +
                 " at (" + QString::number(i.trigger_x) + "," + QString::number(i.trigger_y) +
                 ")!");
        }
    }
}
