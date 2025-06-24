#pragma once

#include "../third_party/dbstring.h"
#include "lcf/lmu/reader.h"
#include "track.h"

#include <lcf/rpg/event.h>
#include <lcf/rpg/map.h>

#include <QTextStream>
#include <QFileDialog>
#include <qfile.h>

void gen_unlock_check_header(int type, int id, std::vector<lcf::rpg::EventCommand> &v) {
    // conditional branch header
    lcf::rpg::EventCommand c;
    c.indent = 0;
    c.code = int(lcf::rpg::EventCommand::Code::ConditionalBranch);
    c.parameters = {
        5,
        type == 0 ? 2 : 3,
        4,
        id,
        0,
        1
    };
    v.push_back(c);

    lcf::rpg::EventCommand mul9;
    mul9.indent = 1;
    mul9.code = int(lcf::rpg::EventCommand::Code::ControlVars);
    mul9.parameters = {
        0, 9, 9, 3, 0, 0, 0
    };
    v.push_back(mul9);

    lcf::rpg::EventCommand branchelse;
    branchelse.indent = 0;
    branchelse.code = int(lcf::rpg::EventCommand::Code::ElseBranch);
    v.push_back(branchelse);

    lcf::rpg::EventCommand mul8;
    mul8.indent = 1;
    mul8.code = int(lcf::rpg::EventCommand::Code::ControlVars);
    mul8.parameters = {
        0, 8, 8, 3, 0, 0, 0
    };
    v.push_back(mul8);

    lcf::rpg::EventCommand branchend;
    branchend.indent = 0;
    branchend.code = int(lcf::rpg::EventCommand::Code::EndBranch);
    v.push_back(branchend);
}

void gen_play_header(std::vector<lcf::rpg::EventCommand> &v) {
    lcf::rpg::EventCommand recalc_id;
    recalc_id.indent = 0;
    recalc_id.code = int(lcf::rpg::EventCommand::Code::CallEvent);
    recalc_id.parameters = { 1, 13, 1 };
    v.push_back(recalc_id);

    lcf::rpg::EventCommand reset_branch;
    reset_branch.indent = 0;
    reset_branch.code = int(lcf::rpg::EventCommand::Code::ConditionalBranch);
    reset_branch.parameters = { 1, 1, 1, 4, 5, 0 };
    v.push_back(reset_branch);

    lcf::rpg::EventCommand reset_setter;
    reset_setter.indent = 1;
    reset_setter.code = int(lcf::rpg::EventCommand::Code::ControlVars);
    reset_setter.parameters = { 0, 3, 3, 0, 0, -1, 0 };
    v.push_back(reset_setter);

    lcf::rpg::EventCommand reset_endbranch;
    reset_endbranch.indent = 0;
    reset_endbranch.code = int(lcf::rpg::EventCommand::Code::EndBranch);
    v.push_back(reset_endbranch);

    lcf::rpg::EventCommand label1;
    label1.indent = 0;
    label1.code = int(lcf::rpg::EventCommand::Code::Label);
    label1.parameters = { 1 };
    v.push_back(label1);

    lcf::rpg::EventCommand add1;
    add1.indent = 0;
    add1.code = int(lcf::rpg::EventCommand::Code::ControlVars);
    add1.parameters = { 0, 3, 3, 1, 0, 1, 0 };
    v.push_back(add1);
}

lcf::DBArray<int32_t> unlock_condition_branch_params(track track) {
    if (track.skill) {
        return { 5, 2, 4, track.skill, 0, 1 };
    } else if (track.map_id) {
        return { 5, 2, 4, track.map_id, 0, 1 };
    }
    return {};
}

lcf::rpg::EventCommand gen_play_command(int32_t depth, track track) {
    lcf::rpg::EventCommand c;
    c.indent = depth;
    c.code = int(lcf::rpg::EventCommand::Code::PlayBGM);
    c.parameters = { 0, track.volume, track.speed, 50 };
    c.string = ToDBString(track.track_file);
    return c;
}

std::vector<lcf::rpg::EventCommand> gen_play_check_branch(int32_t depth, QList<track> &tracks) {
    std::vector<lcf::rpg::EventCommand> c;
    if (tracks.size() > 0) {
        lcf::rpg::EventCommand branch_head;
        branch_head.indent = depth;
        branch_head.code = int(lcf::rpg::EventCommand::Code::ConditionalBranch);
        branch_head.parameters = unlock_condition_branch_params(tracks.back());
        c.push_back(branch_head);

        lcf::rpg::EventCommand branch_else;
        branch_else.indent = depth;
        branch_else.code = int(lcf::rpg::EventCommand::Code::ElseBranch);
        c.push_back(branch_else);

        tracks.pop_back();
        if (tracks.size() > 0) {
            for (lcf::rpg::EventCommand command : gen_play_check_branch(depth + 1, tracks)){
                c.push_back(command);
            }
        } else {
            lcf::rpg::EventCommand jump2;
            jump2.indent = depth + 1;
            jump2.code = int(lcf::rpg::EventCommand::Code::JumpToLabel);
            jump2.parameters = { 2 };
            c.push_back(jump2);
        }

        lcf::rpg::EventCommand branch_end;
        branch_end.indent = depth;
        branch_end.code = int(lcf::rpg::EventCommand::Code::EndBranch);
        c.push_back(branch_end);
    }
    return c;
}

std::vector<lcf::rpg::EventCommand> gen_play_play_branch(int32_t depth, track track) {
    std::vector<lcf::rpg::EventCommand> c;
    if (track.map_id || track.skill) {
        lcf::rpg::EventCommand branch_head;
        branch_head.indent = depth;
        branch_head.code = int(lcf::rpg::EventCommand::Code::ConditionalBranch);
        branch_head.parameters = unlock_condition_branch_params(track);
        c.push_back(branch_head);

        c.push_back(gen_play_command(depth + 1, track));

        lcf::rpg::EventCommand branch_else;
        branch_else.indent = depth;
        branch_else.code = int(lcf::rpg::EventCommand::Code::ElseBranch);
        c.push_back(branch_else);

        lcf::rpg::EventCommand jump1;
        jump1.indent = depth + 1;
        jump1.code = int(lcf::rpg::EventCommand::Code::JumpToLabel);
        jump1.parameters = { 1 };
        c.push_back(jump1);

        lcf::rpg::EventCommand branch_end;
        branch_end.indent = depth;
        branch_end.code = int(lcf::rpg::EventCommand::Code::EndBranch);
        c.push_back(branch_end);
    } else {
        c.push_back(gen_play_command(depth, track));
    }

    return c;

}

std::vector<lcf::rpg::EventCommand> gen_play_run_branch(int32_t depth, QList<track> &tracks) {
    std::vector<lcf::rpg::EventCommand> c;
    if (tracks.size() > 0) {
        lcf::rpg::EventCommand branch_head;
        branch_head.indent = depth;
        branch_head.code = int(lcf::rpg::EventCommand::Code::ConditionalBranch);
        branch_head.parameters = { 1, 3, 0, depth, 0, 1, };
        c.push_back(branch_head);

        if (depth == 0) {
            lcf::rpg::EventCommand set16;
            set16.indent = depth + 1;
            set16.code = int(lcf::rpg::EventCommand::Code::ControlVars);
            set16.parameters = { 0, 16, 16, 0, 1, 141, 0 };
            c.push_back(set16);

            lcf::rpg::EventCommand set17;
            set17.indent = depth + 1;
            set17.code = int(lcf::rpg::EventCommand::Code::ControlVars);
            set17.parameters = { 0, 17, 17, 0, 1, 142, 0 };
            c.push_back(set17);
        }


        lcf::rpg::EventCommand set18;
        set18.indent = depth + 1;
        set18.code = int(lcf::rpg::EventCommand::Code::ControlVars);
        set18.parameters = { 0, 18, 18, 0, 1, 3, 0 };
        c.push_back(set18);

        for (lcf::rpg::EventCommand command : gen_play_play_branch(depth + 1, tracks.back())){
            c.push_back(command);
        }

        lcf::rpg::EventCommand branch_else;
        branch_else.indent = depth;
        branch_else.code = int(lcf::rpg::EventCommand::Code::ElseBranch);
        c.push_back(branch_else);

        tracks.pop_back();
        if (tracks.size() > 0) {
            for (lcf::rpg::EventCommand command : gen_play_run_branch(depth + 1, tracks)){
                c.push_back(command);
            }
        }

        lcf::rpg::EventCommand branch_end;
        branch_end.indent = depth;
        branch_end.code = int(lcf::rpg::EventCommand::Code::EndBranch);
        c.push_back(branch_end);
    }
    return c;
}

void gen_play_footer(std::vector<lcf::rpg::EventCommand> &v) {
    lcf::rpg::EventCommand set19to1;
    set19to1.indent = 0;
    set19to1.code = int(lcf::rpg::EventCommand::Code::ControlVars);
    set19to1.parameters = { 0, 19, 19, 0, 1, 1, 0 };
    v.push_back(set19to1);

    lcf::rpg::EventCommand mult19;
    mult19.indent = 0;
    mult19.code = int(lcf::rpg::EventCommand::Code::ControlVars);
    mult19.parameters = { 0, 19, 19, 3, 0, 100, 0 };
    v.push_back(mult19);

    lcf::rpg::EventCommand addto18;
    addto18.indent = 0;
    addto18.code = int(lcf::rpg::EventCommand::Code::ControlVars);
    addto18.parameters = { 0, 18, 18, 1, 1, 19, 0 };
    v.push_back(addto18);

    lcf::rpg::EventCommand bgupdate;
    bgupdate.indent = 0;
    bgupdate.code = int(lcf::rpg::EventCommand::Code::CallEvent);
    bgupdate.parameters = { 1, 16, 1 };
    v.push_back(bgupdate);

    lcf::rpg::EventCommand label2;
    label2.indent = 0;
    label2.code = int(lcf::rpg::EventCommand::Code::Label);
    label2.parameters = { 2 };
    v.push_back(label2);
}

int run_record_player(std::string project, QWidget *parent) {
    // load the record player data as a csv
    QList<QList<track>> track_list;
    QFile f(QFileDialog::getOpenFileName(parent, "Select the record player data", "", "Tab separated values (*.tsv)"));
    if (f.open(QFile::ReadOnly | QFile::Text)){
        QTextStream in(&f);
        QString s;
        while (in.readLineInto(&s)) {
            QStringList split = s.split("\t");
            if (split.size() >= 7) {
                bool has_id = split[0].isEmpty();
                if (has_id) {
                    track_list.last().append(track(split[1].trimmed(), split[2], split[3].toInt(), split[4].toInt(), split[5], split[6].toInt(), split[7].toInt()));
                } else {
                    QList<track> temp;
                    temp.append(track(split[1].trimmed(), split[2], split[3].toInt(), split[4].toInt(), split[5], split[6].toInt(), split[7].toInt()));
                    track_list.append(temp);
                }
            }
        }
        track_list.pop_front();
    } else {
        return 1;
    }

    // construct the holding events
    lcf::rpg::Event unlock_check_event;
    unlock_check_event.ID = 18;
    unlock_check_event.y = 7;
    unlock_check_event.name = lcf::DBString("unlock check");

    lcf::rpg::Event play_event;
    play_event.ID = 6;
    play_event.y = 2;
    play_event.name = lcf::DBString("play");

    //generate pages for both
    lcf::rpg::EventCommand mul9;
    mul9.code = int(lcf::rpg::EventCommand::Code::ControlVars);
    mul9.parameters = {
        0, 9, 9, 3, 0, 0, 0
    };

    int counter = 0;
    for (QList<track> i : track_list) {
        counter++;
        // unlock check
        lcf::rpg::EventPage p_unlock;
        p_unlock.ID = counter;
        for (track j : i) {
            if (j.map_id != 0 || j.skill != 0) {
                gen_unlock_check_header(int(j.skill != 0), j.skill != 0 ? j.skill : j.map_id, p_unlock.event_commands);
            } else {
                p_unlock.event_commands.push_back(mul9);
            }
        }
        unlock_check_event.pages.push_back(p_unlock);
        // play command
        std::reverse(i.begin(), i.end());

        lcf::rpg::EventPage p_play;
        p_play.ID = counter;

        gen_play_header(p_play.event_commands);

        // the check branch only exists if none of the grouped tracks are auto-unlocked
        bool dirty = false;
        for (auto j : i) {
            dirty = dirty || (!j.map_id && !j.skill);
        }
        auto tl_copy = i;
        if (!dirty) {
            for (auto j : gen_play_check_branch(0, tl_copy)) {
                p_play.event_commands.push_back(j);
            }
        }

        lcf::rpg::EventCommand set4;
        set4.indent = 0;
        set4.code = int(lcf::rpg::EventCommand::Code::ControlVars);
        set4.parameters = { 0, 4, 4, 0, 1, 1, 0 };
        p_play.event_commands.push_back(set4);

        lcf::rpg::EventCommand mod;
        mod.indent = 0;
        mod.code = int(lcf::rpg::EventCommand::Code::ControlVars);
        mod.parameters = { 0, 3, 3, 5, 0, static_cast<int32_t>(i.size()), 0 };
        p_play.event_commands.push_back(mod);

        tl_copy = i;
        for (auto j : gen_play_run_branch(0, tl_copy)) {
            p_play.event_commands.push_back(j);
        }
        gen_play_footer(p_play.event_commands);
        play_event.pages.push_back(p_play);
    }
    int track_count = counter;

    // replace events
    counter = 0;
    std::unique_ptr<lcf::rpg::Map> map = lcf::LMU_Reader::Load(project + "/Map0007.lmu", "UTF-8");
    for (auto i : map->events) {
        if (i.ID == 6) {
            map->events[counter] = play_event;
            break;
        }
        counter++;
    }
    counter = 0;
    for (auto i : map->events) {
        if (i.ID == 18) {
            map->events[counter] = unlock_check_event;
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
                    && j.parameters[1] == 144) {
                    j.parameters[5] = track_count - 1;
                    break;
                }
            }
            break;
        }
        counter++;
    }
    lcf::LMU_Reader::Save(project + "/Map0007.lmu", *map, lcf::EngineVersion::e2k3, "UTF-8");

    return 0;
}
