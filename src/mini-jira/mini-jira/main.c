#include "raylib.h"
#include "theme.h"
#include "data.h"
#include "ui.h"

int main(void) {
    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "Mini Jira Dashboard - Day 2");
    SetTargetFPS(60);

    // Инициализация всех модулей
    Theme_Init();
    Data_Init();
    UI_Init();

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(theme.bg_main);

        // Отрисовка интерфейса Kanban
        UI_DrawHeader();
        UI_DrawKanbanBoard();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}