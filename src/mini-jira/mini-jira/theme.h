#ifndef THEME_H
#define THEME_H

#include "raylib.h"

typedef struct {
    Color bg_dark;
    Color bg_panel;
    Color bg_card;
    Color bg_card_hover;
    Color text_primary;
    Color text_secondary;
    Color accent_blue;
    Color status_todo;
    Color status_in_progress;
    Color status_in_review;
    Color status_done;
    Color priority_low;
    Color priority_medium;
    Color priority_high;
    Color priority_urgent;
    Color border;
} Theme;

extern Theme theme;

void Theme_Init(void);

#endif