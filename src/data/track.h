#pragma once

#include <QFileDialog>
#include <QList>
#include <QString>

struct track {
    track(QString track_file, QString artist, int volume, int speed, QString location, QList<int> map_id, int skill = 0) : track_file(track_file), artist(artist), volume(volume), speed(speed), location(location), map_id(map_id), skill(skill) {}
    QString track_file;
    QString artist;
    int volume;
    int speed;
    QString location;
    QList<int> map_id;
    int skill = 0;
};

QList<QList<track>> load_track_list(QWidget *parent);
