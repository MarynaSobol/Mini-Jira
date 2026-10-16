#include "raylib.h"
#include "theme.h"
#include "data.h"
#include "ui.h"

// Application entry point
int main(void) {
    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "Mini Jira Dashboard");
    SetTargetFPS(60);

    // Initialize core subsystems
    Theme_Init();
    Data_Init();
    UI_Init();

    // Main render loop
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(theme.bg_main);

        // Render UI components
        UI_DrawHeader();
        UI_DrawKanbanBoard();
        EndDrawing();
    }

    // Close window and graphics context
    CloseWindow();
    return 0;
}