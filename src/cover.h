#pragma once

#include "book.h"
#include "book_data.h"
#include "lcf/lmu/reader.h"

#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QWidget>

#include <lcf/rpg/event.h>
#include <QLabel>

struct BookDrawAssets {
    BookDrawAssets(QPixmap dark0, QPixmap dark1, QPixmap dark2, QPixmap light0,
                   QPixmap light1, QPixmap light2)
        : dark0(std::move(dark0)), dark1(std::move(dark1)),
        dark2(std::move(dark2)), light0(std::move(light0)),
        light1(std::move(light1)), light2(std::move(light2)) {}
    QPixmap dark0;
    QPixmap dark1;
    QPixmap dark2;
    QPixmap light0;
    QPixmap light1;
    QPixmap light2;
};

QPixmap sheared(QPixmap pixmap, double left, double wscale, double rise, double slope) {
    QTransform transform(1/wscale, 0, (-left+slope)/wscale, slope, 1, -left*slope+rise, 1, 1, 1);
    //QDialog preview;
    //QVBoxLayout layout;
    //QLabel preview_test;
    //preview.setLayout(&layout);
    //preview_test.setPixmap(pixmap.transformed(transform, Qt::FastTransformation));
    //layout.addWidget(&preview_test);
    //preview.exec();
    return pixmap.transformed(transform, Qt::FastTransformation);
}

QPixmap gen_cover_anim(QPixmap cover, BookDrawAssets assets) {
    QPixmap p(320, 720); // 320, 240 * 3
    p.fill(Qt::transparent);
    QPainter painter(&p);
    // frame 0
    painter.drawPixmap(45, 0, cover);
    painter.drawPixmap(0, 0, assets.dark0);
    painter.drawPixmap(0, 0, assets.light0);
    // frame 1
    painter.drawPixmap(45, 240, sheared(cover, 45, 0.057798, 1, 0.19565));
    painter.drawPixmap(0, 240, assets.dark1);
    painter.drawPixmap(0, 240, assets.light1);
    // frame 2
    painter.drawPixmap(45, 480, sheared(cover, 43, 0.43119266, 5, 0.4849));
    painter.drawPixmap(45, 480, sheared(cover, 43, 0.40825, 1, 0.4849));
    painter.drawPixmap(0, 480, assets.dark2);
    painter.drawPixmap(0, 480, assets.light2);
    return p;
}

inline QPixmap gen_cover_preview(QPixmap cover) {
    return cover.scaled(54, 70, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

int run_cover(std::string project, QWidget *parent){
    // load the cover data as a csv
    QList<cover> cover_list;
    QFile f(QFileDialog::getOpenFileName(parent, "Select the cover data", "", "Tab separated values (*.tsv)"));
    if (f.open(QFile::ReadOnly | QFile::Text)){
        QTextStream in(&f);
        QString s;
        in.readLine();
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
        return 1;
    }

    lcf::rpg::EventCommand switchoff;
    switchoff.code = int(lcf::rpg::EventCommand::Code::ControlSwitches);
    switchoff.indent = 0;
    switchoff.parameters = { 0, 1, 1, 1 };

    // generate cover unlock check event
    lcf::rpg::Event cover_event;
    cover_event.ID = 22;
    cover_event.name = "GET_COVER_STATUS";
    cover_event.x = 18;
    cover_event.y = 14;
    int counter = 1;
    for (cover i : cover_list) {
        lcf::rpg::EventPage page;
        page.ID = counter;
        counter++;
        page.event_commands.push_back(switchoff);
        for (auto j : gen_check(i.cond, 0)) {
            page.event_commands.push_back(j);
        }
        cover_event.pages.push_back(page);
    }
    int cover_count = counter;

    // replace events
    counter = 0;
    std::unique_ptr<lcf::rpg::Map> map = lcf::LMU_Reader::Load(project + "/Map0009.lmu", "UTF-8");
    for (auto i : map->events) {
        if (i.ID == 22) {
            map->events[counter] = cover_event;
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
                    && j.parameters[1] == 111) {
                    j.parameters[5] = cover_count - 1;
                    break;
                }
            }
            break;
        }
        counter++;
    }
    lcf::LMU_Reader::Save(project + "/Map0009.lmu", *map, lcf::EngineVersion::e2k3, "UTF-8");



    // generate assets (full cover anim, book preview, book name/author)
    BookDrawAssets book_draw_assets(QPixmap(":/dark0"), QPixmap(":/dark1"), QPixmap(":/dark2"), QPixmap(":/light0"), QPixmap(":/light1"), QPixmap(":/light2"));
    QDir covers_path = QFileDialog::getExistingDirectory(parent, "Select the directory containing the raw cover files:");
    counter = 0;
    Font font = gen_text_qhash();
    for (cover i : cover_list) {
        QString path = covers_path.absolutePath() + "/" + QString::number(counter).rightJustified(4, QChar(48)) + ".png";
        QPixmap cover_image(path);
        if (!cover_image.isNull()){
            if (cover_image.width() == 218 && cover_image.height() == 282){
                gen_cover_anim(cover_image, book_draw_assets)
                .save(QString::fromStdString(project) + "/Picture/book/cover" + QString::number(counter).rightJustified(4, QChar(48)) + ".png");
                gen_cover_preview(cover_image)
                    .save(QString::fromStdString(project) + "/Picture/book/cpreview" + QString::number(counter).rightJustified(4, QChar(48)) + ".png");
            } else {
                QMessageBox::warning(parent, "Warning", "The cover image at " + path + " isn't 218 by 282! Skipping.");
            }
        }
        gen_book_name(font, counter, i.name)
            .save(QString::fromStdString(project) + "/Picture/book/cname" + QString::number(counter).rightJustified(4, QChar(48)) + ".png");
        gen_book_author(font, counter, i.author)
            .save(QString::fromStdString(project) + "/Picture/book/cauthor" + QString::number(counter).rightJustified(4, QChar(48)) + ".png");
        counter++;
    }

    return 0;
}
