#include "theme.h"

Theme theme;

// Configures the Slate/Indigo dark theme color values
void Theme_Init(void) {
    theme.bg_main = (Color){ 15, 23, 42, 255 };    // Deep Slate (#0F172A)
    theme.bg_sidebar = (Color){ 30, 41, 59, 255 };    // Dark Slate (#1E293B)
    theme.bg_column = (Color){ 24, 33, 47, 255 };    // Column panel (#18212F)
    theme.bg_card = (Color){ 38, 51, 71, 255 };    // Card base (#263347)
    theme.bg_card_hover = (Color){ 51, 65, 85, 255 };    // Card highlight (#334155)

    theme.accent = (Color){ 99, 102, 241, 255 };  // Indigo (#6366F1)
    theme.accent_hover = (Color){ 129, 140, 248, 255 }; // Light Indigo (#818CF8)
    theme.border = (Color){ 51, 65, 85, 255 };    // Border tint (#334155)
    theme.danger = (Color){ 239, 68, 68, 255 };   // Red (#EF4444)

    theme.text_bright = (Color){ 248, 250, 252, 255 }; // High contrast (#F8FAFC)
    theme.text_normal = (Color){ 203, 213, 225, 255 }; // Primary body (#CBD5E1)
    theme.text_muted = (Color){ 148, 163, 184, 255 }; // Secondary (#94A3B8)

    theme.priority_low = (Color){ 100, 116, 139, 255 };
    theme.priority_medium = (Color){ 59, 130, 246, 255 };
    theme.priority_high = (Color){ 249, 115, 22, 255 };
    theme.priority_urgent = (Color){ 239, 68, 68, 255 };

    theme.status_todo = (Color){ 100, 116, 139, 255 };
    theme.status_in_progress = (Color){ 99, 102, 241, 255 };
    theme.status_in_review = (Color){ 245, 158, 11, 255 };
    theme.status_done = (Color){ 34, 197, 94, 255 };
}
