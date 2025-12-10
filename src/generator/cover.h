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
    BookDrawAssets(QImage dark, QImage light0,
                   QImage light1, QImage light2, QImage light3)
        : dark(std::move(dark)), light0(std::move(light0)),
        light1(std::move(light1)), light2(std::move(light2)), light3(std::move(light3)) {}
    QImage dark;
    QImage light0;
    QImage light1;
    QImage light2;
    QImage light3;
};

void pil_draw_image_multiply(QImage &in1, QImage &in2) {
    // Qt bug: multiply breaks with transparency, so we have to reimplement it
    for (int x = 0; x <= in1.width(); x++) {
        for (int y = 0; y <= in1.height(); y++) {
            QColor in1_color(in1.pixelColor(x,y));
            QColor in2_color(in2.pixelColor(x,y));
            QColor out_color(
                in1_color.red() * in2_color.red() / 255,
                in1_color.green() * in2_color.green() / 255,
                in1_color.blue() * in2_color.blue() / 255,
                in1_color.alpha()
            );
            in1.setPixelColor(x, y, out_color);
        }
    }
}

QImage pil_affine_transform(QTransform transform, QImage src) {
    QImage dest(320, 240, QImage::Format_ARGB32);
    dest.fill(Qt::transparent);
    QPainter painter(&dest);
    for (int x = 0; x <= 320; x++) {
        for (int y = 0; y <= 240; y++) {
            dest.setPixelColor(x, y, src.pixelColor(
                transform.m11()*x + transform.m21()*y + transform.dx(),
                transform.m12()*x + transform.m22()*y + transform.dy())
            );
        }
    }
    return dest;
}

QImage sheared(QImage image, double left, double wscale, double rise, double slope) {
    QTransform transform(1.0/wscale,             slope,            0.0,
                         0.0,                    1.0,              0.0,
                         (-left+slope)/wscale,   -left*slope+rise, 1.0);
    return pil_affine_transform(transform, image);
}

QImage gen_cover_anim(QImage cover, BookDrawAssets assets) {
    cover.convertTo(QImage::Format_ARGB32);
    QImage p(320, 720, QImage::Format_ARGB32); // 320, 240 * 3
    p.fill(Qt::transparent);
    QPainter painter(&p);
    // frame 0
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.drawImage(0, 0, sheared(cover, 45, 1, 0, 0));
    painter.setCompositionMode(QPainter::CompositionMode_Plus);
    painter.drawImage(0, 0, assets.light0);
    painter.drawImage(0, 0, assets.light0);
    // frame 1
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.drawImage(0, 240, sheared(cover, 45, 0.857798, 1, 0.19565), 0, 0, 320, 240);
    painter.setCompositionMode(QPainter::CompositionMode_Plus);
    painter.drawImage(0, 240, assets.light1);
    painter.drawImage(0, 240, assets.light1);
    // frame 2
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.drawImage(0, 480, sheared(cover, 43, 0.43119266, 5, 0.4849));
    painter.drawImage(0, 480, sheared(cover, 43, 0.40825, 5, 0.4849));
    painter.setCompositionMode(QPainter::CompositionMode_Plus);
    painter.drawImage(0, 480, assets.light2);
    painter.drawImage(0, 480, assets.light3);
    painter.end();

    pil_draw_image_multiply(p, assets.dark);
    return p;
}

inline QImage gen_cover_preview(QImage cover) {
    return cover.scaled(54, 70, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

int run_cover(std::string project, QWidget *parent, bool overwrite){
    // load the cover data as a tsv
    QList<cover> cover_list = load_cover_list(parent);
    if (cover_list.isEmpty()){
        return 1;
    }

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
    BookDrawAssets book_draw_assets(QImage(":/dark"), QImage(":/light0"), QImage(":/light1"), QImage(":/light2"), QImage(":/light3"));
    QDir covers_path = QFileDialog::getExistingDirectory(parent, "Select the directory containing the raw cover files (as XXXX.png):");
    counter = 0;
    Font font = gen_book_font();
    for (cover i : cover_list) {
        QString path = covers_path.absolutePath() + "/" + QString::number(counter).rightJustified(4, QChar(48)) + ".png";
        QImage cover_image(path);
        if (!cover_image.isNull()){
            if (cover_image.width() == 218 && cover_image.height() == 282) {
                QString anim_fname(QString::fromStdString(project) + "/Picture/book/cover/cover" + QString::number(counter).rightJustified(4, QChar(48)) + ".png");
                if (!QFile::exists(anim_fname) || overwrite) {
                    gen_cover_anim(cover_image, book_draw_assets)
                        .save(anim_fname);
                }
                QString cover_fname(QString::fromStdString(project) + "/Picture/book/cover/cpreview" + QString::number(counter).rightJustified(4, QChar(48)) + ".png");
                if (!QFile::exists(cover_fname) || overwrite) {
                    gen_cover_preview(cover_image)
                        .save(cover_fname);
                }
                QString cname_fname(QString::fromStdString(project) + "/Picture/book/cover/cname" + QString::number(counter).rightJustified(4, QChar(48)) + ".png");
                if (!QFile::exists(cname_fname) || overwrite) {
                    gen_book_name(font, counter, i.name)
                        .save(cname_fname);
                }
                QString cauthor_fname(QString::fromStdString(project) + "/Picture/book/cover/cauthor" + QString::number(counter).rightJustified(4, QChar(48)) + ".png");
                if (!QFile::exists(cauthor_fname) || overwrite) {
                    gen_book_author(font, counter, i.author)
                        .save(cauthor_fname);
                }
            } else {
                QMessageBox::warning(parent, "Warning", "The cover image at " + path + " isn't 218 by 282! Skipping.");
            }
        }
        counter++;
    }

    return 0;
}
