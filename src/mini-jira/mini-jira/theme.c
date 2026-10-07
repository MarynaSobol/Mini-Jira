#include "theme.h"

Theme theme;

void Theme_Init(void) {
    theme.bg_dark = (Color){ 15, 23, 42, 255 };   // #0f172a
    theme.bg_panel = (Color){ 30, 41, 59, 255 };   // #1e293b
    theme.bg_card = (Color){ 51, 65, 85, 255 };   // #334155
    theme.bg_card_hover = (Color){ 71, 85, 105, 255 };  // #475569
    theme.text_primary = (Color){ 241, 245, 249, 255 };// #f1f5f9
    theme.text_secondary = (Color){ 148, 163, 184, 255 };// #94a3b8
    theme.accent_blue = (Color){ 59, 130, 246, 255 }; // #3b82f6

    theme.status_todo = (Color){ 100, 116, 139, 255 };
    theme.status_in_progress = (Color){ 59, 130, 246, 255 };
    theme.status_in_review = (Color){ 245, 158, 11, 255 };
    theme.status_done = (Color){ 34, 197, 94, 255 };

    theme.priority_low = (Color){ 148, 163, 184, 255 };
    theme.priority_medium = (Color){ 59, 130, 246, 255 };
    theme.priority_high = (Color){ 249, 115, 22, 255 };
    theme.priority_urgent = (Color){ 239, 68, 68, 255 };

    theme.border = (Color){ 71, 85, 105, 255 };
}