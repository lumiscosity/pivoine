#include "../third_party/dbstring.h"
#include "validate_rp_dialog.h"
#include "lcf/lmu/reader.h"
#include "track.h"
#include "ui_validate_rp_dialog.h"

#include <QMessageBox>

void insert_or_create(QHash<int, QSet<int>> &dest, int key, int value) {
    // we have defaultdict at home
    if (dest.keys().contains(key)) {
        dest.value(key).insert(value);
    } else {
        dest.insert(key, QSet<int>({value}));
    }
}

ValidateRPDialog::ValidateRPDialog(QString project, QWidget *parent)
    : QDialog(parent)
    , project(project), ui(new Ui::ValidateRPDialog)
{
    ui->setupUi(this);
}

ValidateRPDialog::~ValidateRPDialog()
{
    delete ui;
}

void ValidateRPDialog::push(QString string) {
    ui->plainTextEdit->appendPlainText(string);
}

void ValidateRPDialog::validate()
{
    QList<QList<track>> tracks = load_track_list(this);
    QString check_path = this->project + "/Music/";
    QSet<int> tsv_skill_ids;

    int co = 0;
    for (QList<track> i : tracks) {
        int ci = 0;
        for (track j : i) {
            // check if the music file exists
            if (!(QFile::exists(check_path + j.track_file + ".wav") ||
                  QFile::exists(check_path + j.track_file + ".mp3") ||
                  QFile::exists(check_path + j.track_file + ".ogg") ||
                  QFile::exists(check_path + j.track_file + ".opus") ||
                  QFile::exists(check_path + j.track_file + ".mid")
                  // i'm not adding a full rtp filename list for this exception
                  || j.track_file == "SERain")) {
                push("Track " + QString::number(co) + " variation " + QString::number(ci) +
                     " refers to the music file " + j.track_file +
                     ", which is missing!\n");
            }

            // store used skill ids for later
            if (j.skill != 0) {
                tsv_skill_ids.insert(j.skill);
            }

            ci++;
        }
        co++;
    }


    // check every map for skill ids
    QDir maps_dir(this->project);
    QHash<int, QSet<int>> maps_per_skill;
    QHash<int, QSet<int>> skills_per_map;

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
            // second pass: populate maps_per_skill and skills_per_map
            for (auto k : j.pages) {
                for (lcf::rpg::EventCommand l : k.event_commands) {
                    if (l.code == int(lcf::rpg::EventCommand::Code::ChangeSkills)) {
                        // ignore the book updater
                        if (!(map_id == 9 && j.ID == 37)) {
                            // only check for changes to the record player actor
                            if (l.parameters[1] == 3) {
                                for (int m : transfers) {
                                    insert_or_create(maps_per_skill, l.parameters[4], m);
                                    insert_or_create(skills_per_map, m, l.parameters[4]);
                                }
                            }
                        }
                    }
                }
            }
        }
        map.reset();
    }

    // check for unused skills getting set
    for (int i : maps_per_skill.keys()) {
        if (!tsv_skill_ids.contains(i)) {
            for (int j : maps_per_skill.value(i)) {
                push("Map " + QString::number(j) +
                     " sets unused skill " + QString::number(i) +
                     "!\n");
            }
        }
    }
    // check for skills which don't get set
    for (int i : tsv_skill_ids - QSet(maps_per_skill.keys().begin(), maps_per_skill.keys().end())) {
        push("Skill " + QString::number(i) +
             " is never set!\n");
    }
    // TODO: check for skills which get set in the wrong map
}
