#include "ui.h"
#include "theme.h"
#include "data.h"
#include "raylib.h"
#include <stdio.h>

static const char* status_titles[STATUS_COUNT] = {
    "TO DO",
    "IN PROGRESS",
    "IN REVIEW",
    "DONE"
};

void UI_Init(void) {
    // Резерв под инициализацию графических элементов
}

void UI_DrawHeader(void) {
    int screenWidth = GetScreenWidth();

    // Верхняя панель
    DrawRectangle(0, 0, screenWidth, 65, theme.bg_sidebar);
    DrawRectangle(0, 64, screenWidth, 1, theme.border);
    DrawText("MINI JIRA DASHBOARD", 25, 20, 22, theme.text_bright);

    // KPI Счётчики в шапке
    DrawText(TextFormat("Total SP: %d", g_data.total_sp), screenWidth - 420, 24, 15, theme.text_normal);
    DrawText(TextFormat("Done: %d", g_data.done_sp), screenWidth - 280, 24, 15, theme.status_done);
    DrawText(TextFormat("In Progress: %d", g_data.in_progress_sp), screenWidth - 170, 24, 15, theme.status_in_progress);
}

static Color GetPriorityColor(TaskPriority priority) {
    switch (priority) {
    case PRIORITY_LOW:    return theme.priority_low;
    case PRIORITY_MEDIUM: return theme.priority_medium;
    case PRIORITY_HIGH:   return theme.priority_high;
    case PRIORITY_URGENT: return theme.priority_urgent;
    default:              return theme.text_muted;
    }
}

void UI_DrawKanbanBoard(void) {
    Vector2 mousePos = GetMousePosition();

    int startX = 20;
    int startY = 85;
    int colWidth = (GetScreenWidth() - 100) / 4;
    int colGap = 20;
    int colHeight = GetScreenHeight() - startY - 20;

    for (int col = 0; col < STATUS_COUNT; col++) {
        int x = startX + col * (colWidth + colGap);
        Rectangle colRect = { (float)x, (float)startY, (float)colWidth, (float)colHeight };

        // 1. Отрисовка фона колонки
        DrawRectangleRounded(colRect, 0.02f, 4, theme.bg_column);
        DrawRectangleRoundedLines(colRect, 0.02f, 4, theme.border);
        DrawText(status_titles[col], x + 15, startY + 15, 14, theme.text_muted);

        // 2. Карточки внутри колонки
        int cardY = startY + 45;
        for (int i = 0; i < g_data.task_count; i++) {
            Task* t = &g_data.tasks[i];
            if (!t->active || t->status != col) continue;

            Rectangle cardRect = { (float)x + 10, (float)cardY, (float)colWidth - 20, 110.0f };
            bool cardHover = CheckCollisionPointRec(mousePos, cardRect);

            // Отрисовка тела карточки с эффектом Hover
            DrawRectangleRounded(cardRect, 0.04f, 4, cardHover ? theme.bg_card_hover : theme.bg_card);
            DrawRectangleRoundedLines(cardRect, 0.04f, 4, theme.border);

            // Метка приоритета (полоска слева)
            Rectangle prioBar = { cardRect.x, cardRect.y + 10, 4, cardRect.height - 20 };
            DrawRectangleRounded(prioBar, 0.5f, 2, GetPriorityColor(t->priority));

            // Кнопка удаления задачи (x)
            Rectangle delBtn = { cardRect.x + cardRect.width - 25, cardRect.y + 8, 18, 18 };
            bool hoverDel = CheckCollisionPointRec(mousePos, delBtn);
            DrawText("x", (int)delBtn.x + 4, (int)delBtn.y - 2, 16, hoverDel ? theme.danger : theme.text_muted);

            if (hoverDel && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                Data_DeleteTask(t->id);
                break;
            }

            // Кнопка сдвига задачи в следующий статус (>)
            if (t->status < STATUS_DONE) {
                Rectangle moveBtn = { cardRect.x + cardRect.width - 25, cardRect.y + cardRect.height - 25, 20, 20 };
                bool hoverMove = CheckCollisionPointRec(mousePos, moveBtn);
                DrawText(">", (int)moveBtn.x + 5, (int)moveBtn.y + 1, 16, hoverMove ? theme.accent : theme.text_muted);

                if (hoverMove && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    Data_MoveTask(t->id, t->status + 1);
                    break;
                }
            }

            // Заголовок и данные задачи
            DrawText(TextFormat("MJ-%d", t->id), (int)cardRect.x + 12, (int)cardRect.y + 10, 12, theme.text_muted);
            DrawText(t->title, (int)cardRect.x + 12, (int)cardRect.y + 28, 14, theme.text_bright);
            DrawText(t->description, (int)cardRect.x + 12, (int)cardRect.y + 50, 12, theme.text_normal);
            DrawText(TextFormat("%s  |  %d SP", t->assignee, t->story_points), (int)cardRect.x + 12, (int)cardRect.y + 82, 12, theme.text_muted);

            cardY += 122; // Смещение вниз для следующей карточки
        }
    }
}