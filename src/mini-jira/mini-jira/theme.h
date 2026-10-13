#ifndef THEME_H
#define THEME_H

#include "raylib.h"

// Structure defining the color palette used throughout the application interface
typedef struct {
    Color bg_main;          // Main background color of the application window
    Color bg_sidebar;       // Background color for sidebar panels or containers
    Color bg_column;        // Background color for Kanban board columns
    Color bg_card;          // Default background color for task cards
    Color bg_card_hover;    // Background color for task cards when hovered by the mouse
    Color accent;           // Primary accent color for buttons and highlights
    Color accent_hover;     // Accent color when hovered
    Color border;           // Color used for lines, outlines, and separators
    Color danger;           // Warning or destructive action color (e.g., delete buttons)
    Color text_bright;      // High-contrast color for headers and important text
    Color text_normal;      // Standard color for general body text
    Color text_muted;       // Muted or faded color for secondary and placeholder text

    // Color indicators for different task priority levels
    Color priority_low;
    Color priority_medium;
    Color priority_high;
    Color priority_urgent;

    // Color indicators for different workflow statuses
    Color status_todo;
    Color status_in_progress;
    Color status_in_review;
    Color status_done;
} Theme;

// Global theme instance accessible across modules
extern Theme theme;

// Initializes the theme color palette values
void Theme_Init(void);

#endif