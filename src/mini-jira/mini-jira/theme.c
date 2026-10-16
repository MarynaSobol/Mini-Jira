#include "theme.h"
#include <stdio.h>

// Global theme instance shared across the whole application
Theme theme;

// Initializes all theme colors and loads the UI font.
// Must be called once after the window has been created (raylib requires
// an active OpenGL context to load fonts/textures).
void Theme_Init(void) {
    // ---- Background colors ----
    // Advanced Cyber-Slate & Glassmorphism Color Palette
    theme.bg_main = (Color){ 13, 18, 30, 255 };    // Rich deep canvas (#0D121E)
    theme.bg_sidebar = (Color){ 22, 29, 46, 255 };    // Elevated panel (#161D2E)
    theme.bg_column = (Color){ 18, 24, 38, 255 };    // Column background (#121826)
    theme.bg_card = (Color){ 28, 38, 59, 255 };    // Card surface (#1C263B)
    theme.bg_card_hover = (Color){ 36, 49, 75, 255 };    // Interactive hover card (#24314B)

    // ---- Accent, border and alert colors ----
    theme.accent = (Color){ 99, 102, 241, 255 };  // Vibrant Indigo (#6366F1)
    theme.accent_hover = (Color){ 129, 140, 248, 255 }; // Light Indigo (#818CF8)
    theme.border = (Color){ 45, 60, 92, 255 };    // Crisp structural border (#2D3C5C)
    theme.danger = (Color){ 244, 63, 94, 255 };   // Rose Red (#F43F5E)

    // ---- Text colors ----
    theme.text_bright = (Color){ 255, 255, 255, 255 }; // Pure white headers
    theme.text_normal = (Color){ 218, 225, 238, 255 }; // Crisp body text
    theme.text_muted = (Color){ 139, 155, 182, 255 }; // Secondary muted labels

    // ---- Task priority colors ----
    theme.priority_low = (Color){ 148, 163, 184, 255 }; // Slate
    theme.priority_medium = (Color){ 59, 130, 246, 255 };  // Blue
    theme.priority_high = (Color){ 249, 115, 22, 255 };  // Orange
    theme.priority_urgent = (Color){ 244, 63, 94, 255 };   // Rose

    // ---- Task status colors ----
    theme.status_todo = (Color){ 148, 163, 184, 255 };        // Slate (not started)
    theme.status_in_progress = (Color){ 99, 102, 241, 255 };  // Indigo (being worked on)
    theme.status_in_review = (Color){ 245, 158, 11, 255 };    // Amber (awaiting review)
    theme.status_done = (Color){ 34, 197, 94, 255 };          // Green (completed)

    // ---- Font loading ----
    // Try loading a professional TrueType font. If not found, falls back safely to default.
    // Place any font file (e.g. Inter-Regular.ttf or Roboto-Regular.ttf) in the project
    // folder as "font.ttf", or change the path below.
    if (FileExists("font.ttf")) {
        // Load at size 20, starting from codepoint 0, with 250 glyphs
        theme.custom_font = LoadFontEx("font.ttf", 20, 0, 250);
        // Bilinear filtering keeps scaled text smooth
        SetTextureFilter(theme.custom_font.texture, TEXTURE_FILTER_BILINEAR);
    }
    else {
        // Fallback: use raylib's built-in default font
        theme.custom_font = GetFontDefault();
    }
}

// Releases resources allocated by Theme_Init.
// Must be called before closing the window.
void Theme_Unload(void) {
    // Only unload the font if it is a custom one; the default font is
    // owned by raylib and must not be freed manually.
    if (theme.custom_font.texture.id != GetFontDefault().texture.id) {
        UnloadFont(theme.custom_font);
    }
}