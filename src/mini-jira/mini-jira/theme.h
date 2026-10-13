#ifndef THEME_H
#define THEME_H

#include "raylib.h"

// Structure defining the design system color palette
typedef struct {
    Color bg_main;          // Main window background color
    Color bg_sidebar;       // Sidebar and top header panel background
    Color bg_column;        // Kanban column panel background
    Color bg_card;          // Default task card background
    Color bg_card_hover;    // Task card background on hover

    Color accent;           // Primary brand/action color
    Color accent_hover;     // Primary action hover state
    Color border;           // Panel and card border stroke color
    Color danger;           // Warning and delete action color

    Color text_bright;      // High-contrast text (titles and headers)
    Color text_normal;      // Standard text (descriptions and body)
    Color text_muted;       // Low-contrast text (labels, IDs, secondary details)

    Color priority_low;     // Low priority badge color
    Color priority_medium;  // Medium priority badge color
    Color priority_high;    // High priority badge color
    Color priority_urgent;  // Urgent priority badge color

    Color status_todo;        // "To Do" status color
    Color status_in_progress; // "In Progress" status color
    Color status_in_review;   // "In Review" status color
    Color status_done;        // "Done" status color
} Theme;

// Global theme instance accessible across UI rendering modules
extern Theme theme;

// Initializes theme color palette with default values
void Theme_Init(void);

#endif // THEME_H
