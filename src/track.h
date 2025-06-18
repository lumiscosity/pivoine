/*
 * This file is part of Pivoine.
 *
 * Pivoine is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Pivoine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Pivoine. If not, see <http://www.gnu.org/licenses/>.
 */

#include <qstring.h>

#pragma once

struct track {
    track(QString track_file, QString artist, int volume, int speed, QString location, int map_id, int skill = 0) : track_file(track_file), artist(artist), volume(volume), speed(speed), location(location), map_id(map_id), skill(skill) {}
    QString track_file;
    QString artist;
    int volume;
    int speed;
    QString location;
    int map_id;
    int skill = 0;
};
