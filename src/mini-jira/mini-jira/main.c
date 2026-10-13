#include "raylib.h"
#include "theme.h"
#include "data.h"
#include "ui.h"

int main(void) {
    // Screen resolution constants
    const int screenWidth = 1280;
    const int screenHeight = 720;

    // Initialize the window and set the target frame rate
    InitWindow(screenWidth, screenHeight, "Mini Jira Dashboard - Day 2");
    SetTargetFPS(60);

    // Initialize all core modules
    Theme_Init();
    Data_Init();
    UI_Init();

    // Main application loop
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(theme.bg_main);

        // Render user interface components
        UI_DrawHeader();
        UI_DrawKanbanBoard();
        EndDrawing();
    }

    // Clean up and close the window
    CloseWindow();
    return 0;
}