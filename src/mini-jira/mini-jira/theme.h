#ifndef THEME_H
#define THEME_H

#include "raylib.h"

typedef struct {
    Color bg_main;
    Color bg_sidebar;
    Color bg_column;
    Color bg_card;
    Color bg_card_hover;
    Color accent;
    Color accent_hover;
    Color border;
    Color danger;
    Color text_bright;
    Color text_normal;
    Color text_muted;

    Color priority_low;
    Color priority_medium;
    Color priority_high;
    Color priority_urgent;

    Color status_todo;
    Color status_in_progress;
    Color status_in_review;
    Color status_done;
} Theme;

extern Theme theme;

void Theme_Init(void);

#endif