#pragma once

#include <QSet>

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
