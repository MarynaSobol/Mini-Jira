#include "ui.h"
#include "theme.h"
#include "data.h"
#include "raylib.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// Global state variables for modals, editing, filtering, and text inputs
static bool show_modal = false;
static int editing_task_id = -1; // -1 indicates creation mode
static int current_priority_filter = -1; // -1 = All, otherwise TaskPriority value (0-3)

static char search_query[64] = "";
static bool search_box_active = false;

static char input_title[MAX_TITLE_LEN] = "";
static char input_desc[MAX_DESC_LEN] = "";
static char input_assignee[MAX_ASSIGNEE_LEN] = "";
static int input_sp = 3;
static TaskStatus input_status = STATUS_TODO;
static TaskPriority input_priority = PRIORITY_MEDIUM;
static int active_field = 0; // 0=Title, 1=Description, 2=Assignee

static const char* status_labels[STATUS_COUNT] = { "TO DO", "IN PROGRESS", "IN REVIEW", "DONE" };
static const char* priority_labels[PRIORITY_COUNT] = { "Low", "Medium", "High", "Urgent" };

// Initialize UI resources (currently empty)
void UI_Init(void) {}

// Simple case-insensitive substring search helper function
static const char* FindSubstringIgnoreCase(const char* haystack, const char* needle) {
    if (!needle || *needle == '\0') return haystack;
    for (; *haystack; ++haystack) {
        const char* h = haystack;
        const char* n = needle;
        while (*h && *n && (tolower((unsigned char)*h) == tolower((unsigned char)*n))) {
            ++h;
            ++n;
        }
        if (*n == '\0') return haystack;
    }
    return NULL;
}

// Open modal window pre-initialized for creating a new task
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

// Open modal window populated with an existing task's details for editing
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

// Handle keyboard text input and backspace deletion for active input buffers
static void HandleTextInput(char* buffer, int max_len) {
    int key = GetCharPressed();
    while (key > 0) {
        if ((key >= 32) && (key <= 125) && ((int)strlen(buffer) < max_len - 1)) {
            int len = (int)strlen(buffer);
            buffer[len] = (char)key;
            buffer[len + 1] = '\0';
        }
        key = GetCharPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) {
        int len = (int)strlen(buffer);
        if (len > 0) buffer[len - 1] = '\0';
    }
}

// Render the task creation/editing modal dialog window
static void DrawModal(void) {
    Vector2 mousePos = GetMousePosition();
    float screenW = (float)GetScreenWidth();
    float screenH = (float)GetScreenHeight();

    // Semi-transparent background overlay
    DrawRectangle(0, 0, (int)screenW, (int)screenH, (Color) { 8, 12, 20, 220 });

    Rectangle modalRect = { screenW / 2.0f - 250, screenH / 2.0f - 280, 500, 560 };

    // Modal background container with drop shadow and border lines
    DrawRectangleRounded((Rectangle) { modalRect.x + 6, modalRect.y + 6, modalRect.width, modalRect.height }, 0.04f, 6, (Color) { 0, 0, 0, 140 });
    DrawRectangleRounded(modalRect, 0.04f, 6, theme.bg_sidebar);
    DrawRectangleRoundedLines(modalRect, 0.04f, 6, theme.accent);

    const char* modalTitle = (editing_task_id == -1) ? "CREATE NEW TASK" : "EDIT TASK DETAILS";

    // Render modal title using custom font if available, otherwise default font
    if (theme.custom_font.texture.id != GetFontDefault().texture.id) {
        DrawTextEx(theme.custom_font, modalTitle, (Vector2) { modalRect.x + 30, modalRect.y + 25 }, 18.0f, 1.0f, theme.text_bright);
    }
    else {
        DrawText(modalTitle, (int)modalRect.x + 30, (int)modalRect.y + 28, 16, theme.text_bright);
    }

    // Modal input and button layout rectangles
    Rectangle titleBox = { modalRect.x + 30, modalRect.y + 80, 440, 38 };
    Rectangle descBox = { modalRect.x + 30, modalRect.y + 145, 440, 38 };
    Rectangle assBox = { modalRect.x + 30, modalRect.y + 210, 250, 38 };
    Rectangle minusBtn = { modalRect.x + 305, modalRect.y + 210, 38, 38 };
    Rectangle plusBtn = { modalRect.x + 432, modalRect.y + 210, 38, 38 };
    Rectangle saveBtn = { modalRect.x + 30, modalRect.y + 480, 212, 45 };
    Rectangle cancelBtn = { modalRect.x + 258, modalRect.y + 480, 212, 45 };

    // Task Title input field interaction and rendering
    DrawText("TASK TITLE", (int)titleBox.x, (int)titleBox.y - 15, 10, theme.text_muted);
    if (CheckCollisionPointRec(mousePos, titleBox) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) active_field = 0;
    DrawRectangleRounded(titleBox, 0.15f, 4, (active_field == 0) ? theme.bg_card_hover : theme.bg_card);
    DrawRectangleRoundedLines(titleBox, 0.15f, 4, (active_field == 0) ? theme.accent : theme.border);

    char titleDisplay[MAX_TITLE_LEN + 2];
    snprintf(titleDisplay, sizeof(titleDisplay), "%s%s", input_title, (active_field == 0 && ((int)(GetTime() * 2) % 2 == 0)) ? "|" : "");
    DrawText(titleDisplay, (int)titleBox.x + 12, (int)titleBox.y + 12, 14, theme.text_bright);

    // Task Description input field interaction and rendering
    DrawText("DESCRIPTION", (int)descBox.x, (int)descBox.y - 15, 10, theme.text_muted);
    if (CheckCollisionPointRec(mousePos, descBox) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) active_field = 1;
    DrawRectangleRounded(descBox, 0.15f, 4, (active_field == 1) ? theme.bg_card_hover : theme.bg_card);
    DrawRectangleRoundedLines(descBox, 0.15f, 4, (active_field == 1) ? theme.accent : theme.border);

    char descDisplay[MAX_DESC_LEN + 2];
    snprintf(descDisplay, sizeof(descDisplay), "%s%s", input_desc, (active_field == 1 && ((int)(GetTime() * 2) % 2 == 0)) ? "|" : "");
    DrawText(descDisplay, (int)descBox.x + 12, (int)descBox.y + 12, 14, theme.text_bright);

    // Assignee input field interaction and rendering
    DrawText("ASSIGNEE", (int)assBox.x, (int)assBox.y - 15, 10, theme.text_muted);
    if (CheckCollisionPointRec(mousePos, assBox) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) active_field = 2;
    DrawRectangleRounded(assBox, 0.15f, 4, (active_field == 2) ? theme.bg_card_hover : theme.bg_card);
    DrawRectangleRoundedLines(assBox, 0.15f, 4, (active_field == 2) ? theme.accent : theme.border);

    char assDisplay[MAX_ASSIGNEE_LEN + 2];
    snprintf(assDisplay, sizeof(assDisplay), "%s%s", input_assignee, (active_field == 2 && ((int)(GetTime() * 2) % 2 == 0)) ? "|" : "");
    DrawText(assDisplay, (int)assBox.x + 12, (int)assBox.y + 12, 14, theme.text_bright);

    // Story Points counter controls and click handlers
    DrawText("STORY POINTS", (int)minusBtn.x, (int)minusBtn.y - 15, 10, theme.text_muted);
    DrawRectangleRounded(minusBtn, 0.2f, 4, theme.bg_card);
    DrawRectangleRounded(plusBtn, 0.2f, 4, theme.bg_card);
    DrawText("-", (int)minusBtn.x + 14, (int)minusBtn.y + 9, 16, theme.text_bright);
    DrawText("+", (int)plusBtn.x + 12, (int)plusBtn.y + 9, 16, theme.text_bright);
    DrawText(TextFormat("%d SP", input_sp), (int)minusBtn.x + 55, (int)minusBtn.y + 12, 14, theme.text_bright);

    if (CheckCollisionPointRec(mousePos, minusBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && input_sp > 1) input_sp--;
    if (CheckCollisionPointRec(mousePos, plusBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && input_sp < 20) input_sp++;

    // Workflow status selection buttons
    DrawText("WORKFLOW STATUS", (int)modalRect.x + 30, (int)modalRect.y + 265, 10, theme.text_muted);
    for (int s = 0; s < STATUS_COUNT; s++) {
        Rectangle btn = { modalRect.x + 30 + s * 111, modalRect.y + 282, 105, 34 };
        bool selected = (input_status == s);
        DrawRectangleRounded(btn, 0.2f, 4, selected ? theme.accent : theme.bg_card);
        DrawText(status_labels[s], (int)btn.x + 12, (int)btn.y + 11, 10, theme.text_bright);
        if (CheckCollisionPointRec(mousePos, btn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) input_status = (TaskStatus)s;
    }

    // Priority level selection buttons
    DrawText("PRIORITY LEVEL", (int)modalRect.x + 30, (int)modalRect.y + 335, 10, theme.text_muted);
    for (int p = 0; p < PRIORITY_COUNT; p++) {
        Rectangle btn = { modalRect.x + 30 + p * 111, modalRect.y + 352, 105, 34 };
        bool selected = (input_priority == p);
        DrawRectangleRounded(btn, 0.2f, 4, selected ? theme.accent : theme.bg_card);
        DrawText(priority_labels[p], (int)btn.x + 18, (int)btn.y + 11, 11, theme.text_bright);

        if (CheckCollisionPointRec(mousePos, btn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            input_priority = (TaskPriority)p;
            if (editing_task_id == -1) {
                input_sp = Data_SuggestStoryPoints(input_priority);
            }
        }
    }

    // Route text input to the currently selected active field buffer
    if (active_field == 0) HandleTextInput(input_title, MAX_TITLE_LEN);
    else if (active_field == 1) HandleTextInput(input_desc, MAX_DESC_LEN);
    else if (active_field == 2) HandleTextInput(input_assignee, MAX_ASSIGNEE_LEN);

    // Save changes button and submission logic
    DrawRectangleRounded(saveBtn, 0.2f, 4, theme.accent);
    DrawText("SAVE CHANGES", (int)saveBtn.x + 52, (int)saveBtn.y + 16, 12, theme.text_bright);

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

    // Cancel button to close modal without saving
    DrawRectangleRounded(cancelBtn, 0.2f, 4, theme.bg_card);
    DrawText("CANCEL", (int)cancelBtn.x + 78, (int)cancelBtn.y + 16, 12, theme.text_muted);

    if (CheckCollisionPointRec(mousePos, cancelBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        show_modal = false;
    }
}

// Render top header bar with branding, filters, search bar, KPI metrics, and new task button
void UI_DrawHeader(void) {
    Vector2 mousePos = GetMousePosition();
    int screenWidth = GetScreenWidth();
    Rectangle newBtn = { (float)screenWidth - 155, 18, 130, 40 };
    bool hoverNew = CheckCollisionPointRec(mousePos, newBtn);

    DrawRectangle(0, 0, screenWidth, 76, theme.bg_sidebar);
    DrawRectangle(0, 75, screenWidth, 1, theme.border);

    DrawText("MINI JIRA", 30, 27, 20, theme.text_bright);

    // Priority filter buttons
    const char* filter_labels[] = { "All", "Low", "Med", "High", "Urgent" };
    int filter_vals[] = { -1, PRIORITY_LOW, PRIORITY_MEDIUM, PRIORITY_HIGH, PRIORITY_URGENT };
    float fx = 145.0f;

    for (int f = 0; f < 5; f++) {
        float fwidth = (f == 0) ? 36.0f : 44.0f;
        Rectangle fBtn = { fx, 22.0f, fwidth, 32.0f };
        bool isSelected = (current_priority_filter == filter_vals[f]);
        bool hoverF = CheckCollisionPointRec(mousePos, fBtn) && !show_modal;

        DrawRectangleRounded(fBtn, 0.3f, 4, isSelected ? theme.accent : (hoverF ? theme.bg_card_hover : theme.bg_card));
        DrawRectangleRoundedLines(fBtn, 0.3f, 4, isSelected ? theme.accent_hover : theme.border);

        Color tColor = isSelected ? theme.text_bright : theme.text_muted;
        DrawText(filter_labels[f], (int)fx + (f == 0 ? 8 : 7), (int)fBtn.y + 10, 11, tColor);

        if (hoverF && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            current_priority_filter = filter_vals[f];
        }
        fx += fwidth + 6;
    }

    // Search bar input control
    Rectangle searchBox = { fx + 15.0f, 22.0f, 160.0f, 32.0f };
    if (CheckCollisionPointRec(mousePos, searchBox) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        search_box_active = true;
    }
    else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        search_box_active = false;
    }

    DrawRectangleRounded(searchBox, 0.3f, 4, search_box_active ? theme.bg_card_hover : theme.bg_card);
    DrawRectangleRoundedLines(searchBox, 0.3f, 4, search_box_active ? theme.accent : theme.border);

    if (search_box_active) {
        HandleTextInput(search_query, sizeof(search_query));
    }

    if (search_query[0] == '\0' && !search_box_active) {
        DrawText("🔍 Search...", (int)searchBox.x + 10, (int)searchBox.y + 9, 11, theme.text_muted);
    }
    else {
        char searchDisplay[70];
        snprintf(searchDisplay, sizeof(searchDisplay), "%s%s", search_query, (search_box_active && ((int)(GetTime() * 2) % 2 == 0)) ? "|" : "");
        DrawText(searchDisplay, (int)searchBox.x + 10, (int)searchBox.y + 9, 11, theme.text_bright);
    }

    // KPI metrics display
    DrawText(TextFormat("Total SP: %d", g_data.total_sp), screenWidth - 520, 31, 14, theme.text_normal);
    DrawText(TextFormat("Done: %d", g_data.done_sp), screenWidth - 385, 31, 14, theme.status_done);
    DrawText(TextFormat("In Progress: %d", g_data.in_progress_sp), screenWidth - 280, 31, 14, theme.status_in_progress);

    // "+ New Task" button rendering and event check
    DrawRectangleRounded(newBtn, 0.3f, 4, hoverNew ? theme.accent_hover : theme.accent);
    DrawText("+ New Task", (int)newBtn.x + 22, (int)newBtn.y + 13, 13, theme.text_bright);

    if (hoverNew && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !show_modal) {
        OpenModalForCreate();
    }
}

// Helper function to get theme color corresponding to task priority
static Color GetPriorityColor(TaskPriority priority) {
    switch (priority) {
    case PRIORITY_LOW:    return theme.priority_low;
    case PRIORITY_MEDIUM: return theme.priority_medium;
    case PRIORITY_HIGH:   return theme.priority_high;
    case PRIORITY_URGENT: return theme.priority_urgent;
    default:              return theme.text_muted;
    }
}

// Render the left sidebar displaying the System Audit Log
static void UI_DrawSidebarAuditLog(void) {
    int startY = 96;
    int sidebarWidth = 240;
    int sidebarHeight = GetScreenHeight() - startY - 25;

    Rectangle sidebarRect = { 25, (float)startY, (float)sidebarWidth, (float)sidebarHeight };

    // Draw sidebar container panel background and border lines
    DrawRectangleRounded(sidebarRect, 0.03f, 4, theme.bg_sidebar);
    DrawRectangleRoundedLines(sidebarRect, 0.03f, 4, theme.border);

    DrawText("SYSTEM AUDIT LOG", (int)sidebarRect.x + 15, (int)sidebarRect.y + 18, 12, theme.text_bright);
    DrawText("Recent system activities", (int)sidebarRect.x + 15, (int)sidebarRect.y + 35, 9, theme.text_muted);

    // Divider line below sidebar header
    DrawLine((int)sidebarRect.x + 15, (int)sidebarRect.y + 55, (int)sidebarRect.x + sidebarWidth - 15, (int)sidebarRect.y + 55, theme.border);

    // Render recent log entries list inside the vertical sidebar panel
    int itemY = (int)sidebarRect.y + 70;
    int maxLogs = 12; // Maximum number of log items that fit into the sidebar
    int startIndex = (g_data.log_count > maxLogs) ? g_data.log_count - maxLogs : 0;

    if (g_data.log_count == 0) {
        DrawText("No activity recorded", (int)sidebarRect.x + 15, itemY, 10, theme.text_muted);
    }
    else {
        for (int i = g_data.log_count - 1, count = 0; i >= startIndex && count < maxLogs; i--, count++) {
            ActivityLog* log = &g_data.logs[i];

            // Render timestamp and message
            DrawText(log->timestamp, (int)sidebarRect.x + 15, itemY, 9, theme.text_muted);

            // Truncate long log messages to fit compactly inside the sidebar
            char shortMsg[45];
            snprintf(shortMsg, sizeof(shortMsg), "%s", log->message);
            DrawText(shortMsg, (int)sidebarRect.x + 15, itemY + 12, 10, theme.text_bright);

            itemY += 38; // Vertical spacing between log entries
        }
    }
}

// Render the main Kanban board alongside the left sidebar and filters
void UI_DrawKanbanBoard(void) {
    // Draw the left Audit Log sidebar first
    UI_DrawSidebarAuditLog();

    Vector2 mousePos = GetMousePosition();

    // Offset the Kanban board to the right to make room for the sidebar (width 240 + offsets)
    int startX = 285;
    int startY = 96;
    int colWidth = (GetScreenWidth() - startX - 25) / 4;
    int colGap = 15;
    int colHeight = GetScreenHeight() - startY - 25;

    for (int col = 0; col < STATUS_COUNT; col++) {
        int x = startX + col * (colWidth + colGap);
        Rectangle colRect = { (float)x, (float)startY, (float)colWidth, (float)colHeight };

        DrawRectangleRounded(colRect, 0.02f, 4, theme.bg_column);
        DrawRectangleRoundedLines(colRect, 0.02f, 4, theme.border);
        DrawText(status_labels[col], x + 15, startY + 18, 12, theme.text_bright);

        int cardY = startY + 50;
        for (int i = 0; i < g_data.task_count; i++) {
            Task* t = &g_data.tasks[i];
            if (!t->active || t->status != col) continue;

            // Filter tasks by priority
            if (current_priority_filter != -1 && t->priority != current_priority_filter) {
                continue;
            }

            // Filter tasks by search query (checks title and assignee)
            if (search_query[0] != '\0') {
                bool matchTitle = (FindSubstringIgnoreCase(t->title, search_query) != NULL);
                bool matchAssignee = (FindSubstringIgnoreCase(t->assignee, search_query) != NULL);
                if (!matchTitle && !matchAssignee) {
                    continue;
                }
            }

            Rectangle cardRect = { (float)x + 10, (float)cardY, (float)colWidth - 20, 110.0f };
            bool cardHover = CheckCollisionPointRec(mousePos, cardRect) && !show_modal;
            Rectangle prioBar = { cardRect.x, cardRect.y + 12, 4, cardRect.height - 24 };
            Rectangle delBtn = { cardRect.x + cardRect.width - 24, cardRect.y + 8, 18, 18 };
            bool hoverDel = CheckCollisionPointRec(mousePos, delBtn) && !show_modal;

            DrawRectangleRounded((Rectangle) { cardRect.x + 3, cardRect.y + 3, cardRect.width, cardRect.height }, 0.04f, 4, (Color) { 0, 0, 0, 100 });
            DrawRectangleRounded(cardRect, 0.04f, 4, cardHover ? theme.bg_card_hover : theme.bg_card);
            DrawRectangleRoundedLines(cardRect, 0.04f, 4, cardHover ? theme.accent : theme.border);

            DrawRectangleRounded(prioBar, 0.5f, 2, GetPriorityColor(t->priority));

            DrawText("x", (int)delBtn.x + 5, (int)delBtn.y - 1, 14, hoverDel ? theme.danger : theme.text_muted);

            if (hoverDel && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                Data_DeleteTask(t->id);
                break;
            }

            if (t->status > STATUS_TODO) {
                Rectangle moveBackBtn = { cardRect.x + cardRect.width - 48, cardRect.y + cardRect.height - 26, 18, 18 };
                bool hoverMoveBack = CheckCollisionPointRec(mousePos, moveBackBtn) && !show_modal;
                DrawText("<", (int)moveBackBtn.x + 5, (int)moveBackBtn.y + 2, 12, hoverMoveBack ? theme.accent : theme.text_muted);

                if (hoverMoveBack && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    Data_MoveTask(t->id, t->status - 1);
                    break;
                }
            }

            if (t->status < STATUS_DONE) {
                Rectangle moveBtn = { cardRect.x + cardRect.width - 24, cardRect.y + cardRect.height - 26, 18, 18 };
                bool hoverMove = CheckCollisionPointRec(mousePos, moveBtn) && !show_modal;
                DrawText(">", (int)moveBtn.x + 5, (int)moveBtn.y + 2, 12, hoverMove ? theme.accent : theme.text_muted);

                if (hoverMove && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    Data_MoveTask(t->id, t->status + 1);
                    break;
                }
            }

            if (cardHover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !hoverDel) {
                OpenModalForEdit(t);
            }

            DrawText(TextFormat("MJ-%d", t->id), (int)cardRect.x + 14, (int)cardRect.y + 10, 10, theme.text_muted);
            DrawText(t->title, (int)cardRect.x + 14, (int)cardRect.y + 28, 12, theme.text_bright);
            DrawText(t->description, (int)cardRect.x + 14, (int)cardRect.y + 48, 10, theme.text_normal);
            DrawText(TextFormat("%s  |  %d SP", t->assignee, t->story_points), (int)cardRect.x + 14, (int)cardRect.y + 80, 10, theme.text_muted);

            cardY += 118;
        }
    }

    if (show_modal) {
        DrawModal();
    }
}