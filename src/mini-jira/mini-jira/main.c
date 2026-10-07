#include "raylib.h"
#include "theme.h"

int main(void) {
    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "Mini Jira Dashboard");
    SetTargetFPS(60);

    Theme_Init();

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(theme.bg_dark);

        DrawRectangle(0, 0, screenWidth, 60, theme.bg_panel);
        DrawText("MINI JIRA DASHBOARD", 20, 18, 24, theme.text_primary);

        DrawRectangleRounded((Rectangle) { 20, 80, 280, 150 }, 0.05f, 4, theme.bg_panel);
        DrawText("Kanban Board Ready", 40, 100, 18, theme.text_primary);
        DrawText("Status: In Progress", 40, 130, 14, theme.accent_blue);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}