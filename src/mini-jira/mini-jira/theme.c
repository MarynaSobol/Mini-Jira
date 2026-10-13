#include "theme.h"

// Global theme instance definition
Theme theme;

// Initializes theme color palette with Slate/Indigo dark mode values
void Theme_Init(void) {
    // Background and panel layout colors
    theme.bg_main = (Color){ 15, 23, 42, 255 };    // Main window background (#0F172A)
    theme.bg_sidebar = (Color){ 30, 41, 59, 255 };    // Header and sidebar background (#1E293B)
    theme.bg_column = (Color){ 24, 33, 47, 255 };    // Kanban column background (#18212F)
    theme.bg_card = (Color){ 38, 51, 71, 255 };    // Default card background (#263347)
    theme.bg_card_hover = (Color){ 51, 65, 85, 255 };    // Card background on hover (#334155)

    // Accent, border, and interaction colors
    theme.accent = (Color){ 99, 102, 241, 255 };  // Primary indigo accent (#6366F1)
    theme.accent_hover = (Color){ 129, 140, 248, 255 }; // Light indigo hover state (#818CF8)
    theme.border = (Color){ 51, 65, 85, 255 };    // UI border lines (#334155)
    theme.danger = (Color){ 239, 68, 68, 255 };   // Warning and deletion red (#EF4444)

    // Typography hierarchy text colors
    theme.text_bright = (Color){ 248, 250, 252, 255 }; // High-contrast headers (#F8FAFC)
    theme.text_normal = (Color){ 203, 213, 225, 255 }; // Primary body text (#CBD5E1)
    theme.text_muted = (Color){ 148, 163, 184, 255 }; // Secondary captions and IDs (#94A3B8)

    // Priority badge indicator colors
    theme.priority_low = (Color){ 100, 116, 139, 255 }; // Low priority (Slate)
    theme.priority_medium = (Color){ 59, 130, 246, 255 };  // Medium priority (Blue)
    theme.priority_high = (Color){ 249, 115, 22, 255 };  // High priority (Orange)
    theme.priority_urgent = (Color){ 239, 68, 68, 255 };   // Urgent priority (Red)

    // Column status indicator colors
    theme.status_todo = (Color){ 100, 116, 139, 255 }; // To Do status
    theme.status_in_progress = (Color){ 99, 102, 241, 255 }; // In Progress status
    theme.status_in_review = (Color){ 245, 158, 11, 255 }; // In Review status
    theme.status_done = (Color){ 34, 197, 94, 255 };  // Done status
}