#include "ui.h"
#include "theme.h"
#include "data.h"
#include "raylib.h"
#include <stdio.h>
#include <string.h>

static bool show_modal = false;
static int editing_task_id = -1; // -1 indicates creation mode

static char input_title[MAX_TITLE_LEN] = "";
static char input_desc[MAX_DESC_LEN] = "";
static char input_assignee[MAX_ASSIGNEE_LEN] = "";
static int input_sp = 3;
static TaskStatus input_status = STATUS_TODO;
static TaskPriority input_priority = PRIORITY_MEDIUM;
static int active_field = 0; // 0=Title, 1=Description, 2=Assignee

static const char* status_labels[STATUS_COUNT] = { "TO DO", "IN PROGRESS", "IN REVIEW", "DONE" };
static const char* priority_labels[PRIORITY_COUNT] = { "Low", "Medium", "High", "Urgent" };

void UI_Init(void) {}

// Prepares modal input buffers for creating a new task
static void OpenModalForCreate(void) {
    editing_task_id = -1;
    input_title[0] = '\0';
    input_desc[0] = '\0';
    snprintf(input_assignee, sizeof(input_assignee), "Marina S.");
    input_priority = PRIORITY_MEDIUM;
    input_sp = Data_SuggestStoryPoints(input_priority);
    input_status = STATUS_TODO;
    active_field = 0;
    show_modal = true;
}

// Prepares modal input buffers for editing an existing task
static void OpenModalForEdit(const Task* t) {
    editing_task_id = t->id;
    snprintf(input_title, sizeof(input_title), "%s", t->title);
    snprintf(input_desc, sizeof(input_desc), "%s", t->description);
    snprintf(input_assignee, sizeof(input_assignee), "%s", t->assignee);
    input_sp = t->story_points;
    input_status = t->status;
    input_priority = t->priority;
    active_field = 0;
    show_modal = true;
}

// Handles live keyboard text entry into active text buffer
static void HandleTextInput(char* buffer, int max_len) {
    // 1. Read all entered characters from the Raylib input queue
    int key = GetCharPressed();
    while (key > 0) {
        // Accept printable ASCII characters only (letters, numbers, spaces)
        if ((key >= 32) && (key <= 125) && ((int)strlen(buffer) < max_len - 1)) {
            int len = (int)strlen(buffer);
            buffer[len] = (char)key;
            buffer[len + 1] = '\0';
        }
        key = GetCharPressed(); // Fetch the next character from the queue
    }

    // 2. Handle Backspace key presses (with key repeat support)
    if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) {
        int len = (int)strlen(buffer);
        if (len > 0) {
            buffer[len - 1] = '\0';
        }
    }
}
// Renders modal overlay for task creation and editing
static void DrawModal(void) {
    Vector2 mousePos = GetMousePosition();
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color) { 0, 0, 0, 200 });

    Rectangle modalRect = { GetScreenWidth() / 2.0f - 220, GetScreenHeight() / 2.0f - 250, 440, 500 };
    DrawRectangleRounded(modalRect, 0.04f, 4, theme.bg_sidebar);
    DrawRectangleRoundedLines(modalRect, 0.04f, 4, theme.border);

    const char* modalTitle = (editing_task_id == -1) ? "Create New Task" : "Edit Task";
    DrawText(modalTitle, (int)modalRect.x + 20, (int)modalRect.y + 20, 20, theme.text_bright);

    // 1. Title Input Field
    DrawText("Title:", (int)modalRect.x + 20, (int)modalRect.y + 60, 14, theme.text_normal);
    Rectangle titleBox = { modalRect.x + 20, modalRect.y + 80, 400, 35 };
    if (CheckCollisionPointRec(mousePos, titleBox) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) active_field = 0;
    DrawRectangleRounded(titleBox, 0.1f, 4, (active_field == 0) ? theme.bg_card_hover : theme.bg_card);
    DrawRectangleRoundedLines(titleBox, 0.1f, 4, (active_field == 0) ? theme.accent : theme.border);

    char titleDisplay[MAX_TITLE_LEN + 2];
    snprintf(titleDisplay, sizeof(titleDisplay), "%s%s", input_title, (active_field == 0 && ((int)(GetTime() * 2) % 2 == 0)) ? "|" : "");
    DrawText(titleDisplay, (int)titleBox.x + 10, (int)titleBox.y + 8, 14, theme.text_bright);

    // 2. Description Input Field
    DrawText("Description:", (int)modalRect.x + 20, (int)modalRect.y + 125, 14, theme.text_normal);
    Rectangle descBox = { modalRect.x + 20, modalRect.y + 145, 400, 35 };
    if (CheckCollisionPointRec(mousePos, descBox) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) active_field = 1;
    DrawRectangleRounded(descBox, 0.1f, 4, (active_field == 1) ? theme.bg_card_hover : theme.bg_card);
    DrawRectangleRoundedLines(descBox, 0.1f, 4, (active_field == 1) ? theme.accent : theme.border);

    char descDisplay[MAX_DESC_LEN + 2];
    snprintf(descDisplay, sizeof(descDisplay), "%s%s", input_desc, (active_field == 1 && ((int)(GetTime() * 2) % 2 == 0)) ? "|" : "");
    DrawText(descDisplay, (int)descBox.x + 10, (int)descBox.y + 8, 14, theme.text_bright);

    // 3. Assignee Input Field
    DrawText("Assignee:", (int)modalRect.x + 20, (int)modalRect.y + 190, 14, theme.text_normal);
    Rectangle assBox = { modalRect.x + 20, modalRect.y + 210, 240, 35 };
    if (CheckCollisionPointRec(mousePos, assBox) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) active_field = 2;
    DrawRectangleRounded(assBox, 0.1f, 4, (active_field == 2) ? theme.bg_card_hover : theme.bg_card);
    DrawRectangleRoundedLines(assBox, 0.1f, 4, (active_field == 2) ? theme.accent : theme.border);

    char assDisplay[MAX_ASSIGNEE_LEN + 2];
    snprintf(assDisplay, sizeof(assDisplay), "%s%s", input_assignee, (active_field == 2 && ((int)(GetTime() * 2) % 2 == 0)) ? "|" : "");
    DrawText(assDisplay, (int)assBox.x + 10, (int)assBox.y + 8, 14, theme.text_bright);

    // Story Points Manual Buttons (- / +)
    DrawText("Story Points:", (int)modalRect.x + 280, (int)modalRect.y + 190, 14, theme.text_normal);
    Rectangle minusBtn = { modalRect.x + 280, modalRect.y + 210, 35, 35 };
    Rectangle plusBtn = { modalRect.x + 385, modalRect.y + 210, 35, 35 };
    DrawRectangleRounded(minusBtn, 0.2f, 4, theme.bg_card);
    DrawRectangleRounded(plusBtn, 0.2f, 4, theme.bg_card);
    DrawText("-", (int)minusBtn.x + 12, (int)minusBtn.y + 8, 18, theme.text_bright);
    DrawText("+", (int)plusBtn.x + 10, (int)plusBtn.y + 8, 18, theme.text_bright);
    DrawText(TextFormat("%d", input_sp), (int)modalRect.x + 332, (int)modalRect.y + 218, 16, theme.text_bright);

    if (CheckCollisionPointRec(mousePos, minusBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && input_sp > 1) input_sp--;
    if (CheckCollisionPointRec(mousePos, plusBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && input_sp < 20) input_sp++;

    // Status Selector Buttons
    DrawText("Status:", (int)modalRect.x + 20, (int)modalRect.y + 260, 14, theme.text_normal);
    for (int s = 0; s < STATUS_COUNT; s++) {
        Rectangle btn = { modalRect.x + 20 + s * 98, modalRect.y + 280, 90, 30 };
        bool selected = (input_status == s);
        DrawRectangleRounded(btn, 0.15f, 4, selected ? theme.accent : theme.bg_card);
        DrawText(status_labels[s], (int)btn.x + 6, (int)btn.y + 8, 10, theme.text_bright);
        if (CheckCollisionPointRec(mousePos, btn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) input_status = (TaskStatus)s;
    }

    // Priority Selector Buttons with Auto SP Suggester
    DrawText("Priority:", (int)modalRect.x + 20, (int)modalRect.y + 325, 14, theme.text_normal);
    for (int p = 0; p < PRIORITY_COUNT; p++) {
        Rectangle btn = { modalRect.x + 20 + p * 98, modalRect.y + 345, 90, 30 };
        bool selected = (input_priority == p);
        DrawRectangleRounded(btn, 0.15f, 4, selected ? theme.accent : theme.bg_card);
        DrawText(priority_labels[p], (int)btn.x + 15, (int)btn.y + 8, 12, theme.text_bright);

        if (CheckCollisionPointRec(mousePos, btn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            input_priority = (TaskPriority)p;
            if (editing_task_id == -1) {
                input_sp = Data_SuggestStoryPoints(input_priority);
            }
        }
    }

    // Route keyboard inputs to active field
    if (active_field == 0) HandleTextInput(input_title, MAX_TITLE_LEN);
    else if (active_field == 1) HandleTextInput(input_desc, MAX_DESC_LEN);
    else if (active_field == 2) HandleTextInput(input_assignee, MAX_ASSIGNEE_LEN);

    // Save Button
    Rectangle saveBtn = { modalRect.x + 20, modalRect.y + 425, 190, 45 };
    DrawRectangleRounded(saveBtn, 0.2f, 4, theme.accent);
    DrawText("Save Task", (int)saveBtn.x + 55, (int)saveBtn.y + 12, 16, theme.text_bright);

    if (CheckCollisionPointRec(mousePos, saveBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (strlen(input_title) > 0) {
            if (editing_task_id == -1) {
                Data_AddTask(input_title, input_desc, input_assignee, input_status, input_priority, input_sp);
            }
            else {
                Data_UpdateTask(editing_task_id, input_title, input_desc, input_assignee, input_status, input_priority, input_sp);
            }
            show_modal = false;
        }
    }

    // Cancel Button
    Rectangle cancelBtn = { modalRect.x + 230, modalRect.y + 425, 190, 45 };
    DrawRectangleRounded(cancelBtn, 0.2f, 4, theme.bg_card);
    DrawText("Cancel", (int)cancelBtn.x + 65, (int)cancelBtn.y + 12, 16, theme.text_muted);

    if (CheckCollisionPointRec(mousePos, cancelBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        show_modal = false;
    }
}

// Renders header bar, metrics, and trigger button for task creation
void UI_DrawHeader(void) {
    Vector2 mousePos = GetMousePosition();
    int screenWidth = GetScreenWidth();

    DrawRectangle(0, 0, screenWidth, 65, theme.bg_sidebar);
    DrawRectangle(0, 64, screenWidth, 1, theme.border);
    DrawText("MINI JIRA DASHBOARD", 25, 20, 22, theme.text_bright);

    // Real-time KPI Story Point Counters (Fixed positions to prevent overlap)
    DrawText(TextFormat("Total SP: %d", g_data.total_sp), screenWidth - 520, 24, 15, theme.text_normal);
    DrawText(TextFormat("Done: %d", g_data.done_sp), screenWidth - 390, 24, 15, theme.status_done);
    DrawText(TextFormat("In Progress: %d", g_data.in_progress_sp), screenWidth - 280, 24, 15, theme.status_in_progress);

    // "+ New Task" trigger button placed securely on the right edge
    Rectangle newBtn = { (float)screenWidth - 140, 15, 120, 36 };
    bool hoverNew = CheckCollisionPointRec(mousePos, newBtn);
    DrawRectangleRounded(newBtn, 0.2f, 4, hoverNew ? theme.accent_hover : theme.accent);
    DrawText("+ New Task", (int)newBtn.x + 15, (int)newBtn.y + 9, 15, theme.text_bright);

    if (hoverNew && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !show_modal) {
        OpenModalForCreate();
    }
}

// Returns badge indicator color based on task priority level
static Color GetPriorityColor(TaskPriority priority) {
    switch (priority) {
    case PRIORITY_LOW:    return theme.priority_low;
    case PRIORITY_MEDIUM: return theme.priority_medium;
    case PRIORITY_HIGH:   return theme.priority_high;
    case PRIORITY_URGENT: return theme.priority_urgent;
    default:              return theme.text_muted;
    }
}

// Main rendering routine for the 4-column Kanban board and task cards
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

        DrawRectangleRounded(colRect, 0.02f, 4, theme.bg_column);
        DrawRectangleRoundedLines(colRect, 0.02f, 4, theme.border);
        DrawText(status_labels[col], x + 15, startY + 15, 14, theme.text_muted);

        int cardY = startY + 45;
        for (int i = 0; i < g_data.task_count; i++) {
            Task* t = &g_data.tasks[i];
            if (!t->active || t->status != col) continue;

            Rectangle cardRect = { (float)x + 10, (float)cardY, (float)colWidth - 20, 110.0f };
            bool cardHover = CheckCollisionPointRec(mousePos, cardRect) && !show_modal;

            DrawRectangleRounded(cardRect, 0.04f, 4, cardHover ? theme.bg_card_hover : theme.bg_card);
            DrawRectangleRoundedLines(cardRect, 0.04f, 4, theme.border);

            Rectangle prioBar = { cardRect.x, cardRect.y + 10, 4, cardRect.height - 20 };
            DrawRectangleRounded(prioBar, 0.5f, 2, GetPriorityColor(t->priority));

            Rectangle delBtn = { cardRect.x + cardRect.width - 25, cardRect.y + 8, 18, 18 };
            bool hoverDel = CheckCollisionPointRec(mousePos, delBtn) && !show_modal;
            DrawText("x", (int)delBtn.x + 4, (int)delBtn.y - 2, 16, hoverDel ? theme.danger : theme.text_muted);

            if (hoverDel && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                Data_DeleteTask(t->id);
                break;
            }

            if (t->status > STATUS_TODO) {
                Rectangle moveBackBtn = { cardRect.x + cardRect.width - 50, cardRect.y + cardRect.height - 25, 20, 20 };
                bool hoverMoveBack = CheckCollisionPointRec(mousePos, moveBackBtn) && !show_modal;
                DrawText("<", (int)moveBackBtn.x + 5, (int)moveBackBtn.y + 1, 16, hoverMoveBack ? theme.accent : theme.text_muted);

                if (hoverMoveBack && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    Data_MoveTask(t->id, t->status - 1);
                    break;
                }
            }

            if (t->status < STATUS_DONE) {
                Rectangle moveBtn = { cardRect.x + cardRect.width - 25, cardRect.y + cardRect.height - 25, 20, 20 };
                bool hoverMove = CheckCollisionPointRec(mousePos, moveBtn) && !show_modal;
                DrawText(">", (int)moveBtn.x + 5, (int)moveBtn.y + 1, 16, hoverMove ? theme.accent : theme.text_muted);

                if (hoverMove && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    Data_MoveTask(t->id, t->status + 1);
                    break;
                }
            }

            if (cardHover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !hoverDel) {
                OpenModalForEdit(t);
            }

            DrawText(TextFormat("MJ-%d", t->id), (int)cardRect.x + 12, (int)cardRect.y + 10, 12, theme.text_muted);
            DrawText(t->title, (int)cardRect.x + 12, (int)cardRect.y + 28, 14, theme.text_bright);
            DrawText(t->description, (int)cardRect.x + 12, (int)cardRect.y + 50, 12, theme.text_normal);
            DrawText(TextFormat("%s  |  %d SP", t->assignee, t->story_points), (int)cardRect.x + 12, (int)cardRect.y + 82, 12, theme.text_muted);

            cardY += 122;
        }
    }

    if (show_modal) {
        DrawModal();
    }
}
