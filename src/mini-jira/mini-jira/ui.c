#include "ui.h"
#include "theme.h"
#include "data.h"
#include "raylib.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// Font scale multiplier (balanced for clean and readable UI layout)
#define FONT_SCALE 2.0f

// Draw text using the custom loaded font and scale factor
static void UI_DrawCustomText(const char* text, int posX, int posY, int fontSize, Color color) {
    DrawTextEx(theme.custom_font, text, (Vector2) { (float)posX, (float)posY }, (float)fontSize* FONT_SCALE, 1.0f, color);
}

// Measure exact text width in pixels considering the font scale
static float UI_MeasureText(const char* text, int fontSize) {
    return MeasureTextEx(theme.custom_font, text, (float)fontSize * FONT_SCALE, 1.0f).x;
}

// Draw text in a single line with ellipsis ("...") truncation if it exceeds max width
static void UI_DrawTextClipped(const char* text, int posX, int posY, int fontSize, float maxWidth, Color color) {
    if (UI_MeasureText(text, fontSize) <= maxWidth) {
        UI_DrawCustomText(text, posX, posY, fontSize, color);
        return;
    }
    char buf[256];
    snprintf(buf, sizeof(buf), "%s", text);
    int len = (int)strlen(buf);
    while (len > 0) {
        buf[len] = '\0';
        char temp[264];
        snprintf(temp, sizeof(temp), "%s...", buf);
        if (UI_MeasureText(temp, fontSize) <= maxWidth) {
            UI_DrawCustomText(temp, posX, posY, fontSize, color);
            return;
        }
        len--;
    }
}

// Automatically wrap long description text into multiple lines within task cards
static void UI_DrawTextWrapped(const char* text, int posX, int posY, int fontSize, float maxWidth, int maxLines, Color color) {
    char words[256];
    snprintf(words, sizeof(words), "%s", text);

    char line[256] = "";
    int currentLine = 0;
    int lineHeight = (int)((float)fontSize * FONT_SCALE) + 6;

    char* context = NULL;
#if defined(_MSC_VER)
    char* word = strtok_s(words, " ", &context);
#else
    char* word = strtok(words, " ");
#endif

    while (word != NULL) {
        char testLine[256];
        if (line[0] == '\0') snprintf(testLine, sizeof(testLine), "%s", word);
        else snprintf(testLine, sizeof(testLine), "%s %s", line, word);

        if (UI_MeasureText(testLine, fontSize) <= maxWidth) {
            snprintf(line, sizeof(line), "%s", testLine);
        }
        else {
            if (currentLine + 1 >= maxLines) {
                UI_DrawTextClipped(testLine, posX, posY + currentLine * lineHeight, fontSize, maxWidth, color);
                return;
            }
            UI_DrawCustomText(line, posX, posY + currentLine * lineHeight, fontSize, color);
            snprintf(line, sizeof(line), "%s", word);
            currentLine++;
        }
#if defined(_MSC_VER)
        word = strtok_s(NULL, " ", &context);
#else
        word = strtok(NULL, " ");
#endif
    }

    if (line[0] != '\0' && currentLine < maxLines) {
        UI_DrawTextClipped(line, posX, posY + currentLine * lineHeight, fontSize, maxWidth, color);
    }
}

// Macro override to seamlessly redirect standard DrawText calls to custom implementation
#define DrawText UI_DrawCustomText

// Global UI interaction states and form buffers
static bool show_modal = false;
static int editing_task_id = -1;
static int current_priority_filter = -1;

static char search_query[64] = "";
static bool search_box_active = false;

static char input_title[MAX_TITLE_LEN] = "";
static char input_desc[MAX_DESC_LEN] = "";
static char input_assignee[MAX_ASSIGNEE_LEN] = "";
static int input_sp = 3;
static TaskStatus input_status = STATUS_TODO;
static TaskPriority input_priority = PRIORITY_MEDIUM;
static int active_field = 0;

static const char* status_labels[STATUS_COUNT] = { "TO DO", "IN PROGRESS", "IN REVIEW", "DONE" };
static const char* priority_labels[PRIORITY_COUNT] = { "Low", "Medium", "High", "Urgent" };

// Initialize UI subsystem components
void UI_Init(void) {}

// Case-insensitive substring search utility for filtering tasks
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

// Open modal window initialized for creating a brand new task
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

// Open modal window populated with selected task details for editing
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

// Handle keyboard character input and backspace events for form fields
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

// Render task creation and editing modal dialog with responsive form controls
static void DrawModal(void) {
    Vector2 mousePos = GetMousePosition();
    float screenW = (float)GetScreenWidth();
    float screenH = (float)GetScreenHeight();

    // Dim background overlay
    DrawRectangle(0, 0, (int)screenW, (int)screenH, (Color) { 8, 12, 20, 220 });

    Rectangle modalRect = { screenW / 2.0f - 310, screenH / 2.0f - 310, 620, 620 };

    // Modal panel container and border styling
    DrawRectangleRounded((Rectangle) { modalRect.x + 6, modalRect.y + 6, modalRect.width, modalRect.height }, 0.04f, 6, (Color) { 0, 0, 0, 140 });
    DrawRectangleRounded(modalRect, 0.04f, 6, theme.bg_sidebar);
    DrawRectangleRoundedLines(modalRect, 0.04f, 6, theme.accent);

    const char* modalTitle = (editing_task_id == -1) ? "CREATE NEW TASK" : "EDIT TASK DETAILS";
    DrawText(modalTitle, (int)modalRect.x + 35, (int)modalRect.y + 30, 16, theme.text_bright);

    Rectangle titleBox = { modalRect.x + 35, modalRect.y + 95, 550, 46 };
    Rectangle descBox = { modalRect.x + 35, modalRect.y + 170, 550, 46 };
    Rectangle assBox = { modalRect.x + 35, modalRect.y + 245, 320, 46 };
    Rectangle minusBtn = { modalRect.x + 385, modalRect.y + 245, 46, 46 };
    Rectangle plusBtn = { modalRect.x + 539, modalRect.y + 245, 46, 46 };
    Rectangle saveBtn = { modalRect.x + 35, modalRect.y + 530, 265, 50 };
    Rectangle cancelBtn = { modalRect.x + 320, modalRect.y + 530, 265, 50 };

    // Task Title field rendering and input handling
    DrawText("TASK TITLE", (int)titleBox.x, (int)titleBox.y - 20, 10, theme.text_muted);
    if (CheckCollisionPointRec(mousePos, titleBox) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) active_field = 0;
    DrawRectangleRounded(titleBox, 0.15f, 4, (active_field == 0) ? theme.bg_card_hover : theme.bg_card);
    DrawRectangleRoundedLines(titleBox, 0.15f, 4, (active_field == 0) ? theme.accent : theme.border);

    char titleDisplay[MAX_TITLE_LEN + 2];
    snprintf(titleDisplay, sizeof(titleDisplay), "%s%s", input_title, (active_field == 0 && ((int)(GetTime() * 2) % 2 == 0)) ? "|" : "");
    UI_DrawTextClipped(titleDisplay, (int)titleBox.x + 14, (int)titleBox.y + 12, 13, titleBox.width - 28, theme.text_bright);

    // Task Description field rendering and input handling
    DrawText("DESCRIPTION", (int)descBox.x, (int)descBox.y - 20, 10, theme.text_muted);
    if (CheckCollisionPointRec(mousePos, descBox) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) active_field = 1;
    DrawRectangleRounded(descBox, 0.15f, 4, (active_field == 1) ? theme.bg_card_hover : theme.bg_card);
    DrawRectangleRoundedLines(descBox, 0.15f, 4, (active_field == 1) ? theme.accent : theme.border);

    char descDisplay[MAX_DESC_LEN + 2];
    snprintf(descDisplay, sizeof(descDisplay), "%s%s", input_desc, (active_field == 1 && ((int)(GetTime() * 2) % 2 == 0)) ? "|" : "");
    UI_DrawTextClipped(descDisplay, (int)descBox.x + 14, (int)descBox.y + 12, 13, descBox.width - 28, theme.text_bright);

    // Assignee field rendering and input handling
    DrawText("ASSIGNEE", (int)assBox.x, (int)assBox.y - 20, 10, theme.text_muted);
    if (CheckCollisionPointRec(mousePos, assBox) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) active_field = 2;
    DrawRectangleRounded(assBox, 0.15f, 4, (active_field == 2) ? theme.bg_card_hover : theme.bg_card);
    DrawRectangleRoundedLines(assBox, 0.15f, 4, (active_field == 2) ? theme.accent : theme.border);

    char assDisplay[MAX_ASSIGNEE_LEN + 2];
    snprintf(assDisplay, sizeof(assDisplay), "%s%s", input_assignee, (active_field == 2 && ((int)(GetTime() * 2) % 2 == 0)) ? "|" : "");
    UI_DrawTextClipped(assDisplay, (int)assBox.x + 14, (int)assBox.y + 12, 13, assBox.width - 28, theme.text_bright);

    // Story Points increment/decrement counter controls
    DrawText("STORY POINTS", (int)minusBtn.x, (int)minusBtn.y - 20, 10, theme.text_muted);
    DrawRectangleRounded(minusBtn, 0.2f, 4, theme.bg_card);
    DrawRectangleRounded(plusBtn, 0.2f, 4, theme.bg_card);
    DrawText("-", (int)(minusBtn.x + (minusBtn.width - UI_MeasureText("-", 16)) / 2), (int)minusBtn.y + 12, 16, theme.text_bright);
    DrawText("+", (int)(plusBtn.x + (plusBtn.width - UI_MeasureText("+", 16)) / 2), (int)plusBtn.y + 12, 16, theme.text_bright);

    const char* spText = TextFormat("%d SP", input_sp);
    DrawText(spText, (int)(minusBtn.x + 46 + (108 - UI_MeasureText(spText, 13)) / 2), (int)minusBtn.y + 14, 13, theme.text_bright);

    if (CheckCollisionPointRec(mousePos, minusBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && input_sp > 1) input_sp--;
    if (CheckCollisionPointRec(mousePos, plusBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && input_sp < 20) input_sp++;

    // Workflow status options selection
    DrawText("WORKFLOW STATUS", (int)modalRect.x + 35, (int)modalRect.y + 315, 10, theme.text_muted);
    for (int s = 0; s < STATUS_COUNT; s++) {
        Rectangle btn = { modalRect.x + 35 + s * 140, (float)(modalRect.y + 335), 130, 40 };
        bool selected = (input_status == s);
        DrawRectangleRounded(btn, 0.2f, 4, selected ? theme.accent : theme.bg_card);
        float tw = UI_MeasureText(status_labels[s], 10);
        DrawText(status_labels[s], (int)(btn.x + (btn.width - tw) / 2), (int)btn.y + 13, 10, theme.text_bright);
        if (CheckCollisionPointRec(mousePos, btn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) input_status = (TaskStatus)s;
    }

    // Priority level options selection
    DrawText("PRIORITY LEVEL", (int)modalRect.x + 35, (int)modalRect.y + 420, 10, theme.text_muted);
    for (int p = 0; p < PRIORITY_COUNT; p++) {
        Rectangle btn = { modalRect.x + 35 + p * 140, (float)(modalRect.y + 440), 130, 40 };
        bool selected = (input_priority == p);
        DrawRectangleRounded(btn, 0.2f, 4, selected ? theme.accent : theme.bg_card);
        float tw = UI_MeasureText(priority_labels[p], 11);
        DrawText(priority_labels[p], (int)(btn.x + (btn.width - tw) / 2), (int)btn.y + 13, 11, theme.text_bright);

        if (CheckCollisionPointRec(mousePos, btn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            input_priority = (TaskPriority)p;
            if (editing_task_id == -1) {
                input_sp = Data_SuggestStoryPoints(input_priority);
            }
        }
    }

    // Route active character inputs to the selected buffer
    if (active_field == 0) HandleTextInput(input_title, MAX_TITLE_LEN);
    else if (active_field == 1) HandleTextInput(input_desc, MAX_DESC_LEN);
    else if (active_field == 2) HandleTextInput(input_assignee, MAX_ASSIGNEE_LEN);

    // Save changes button action
    DrawRectangleRounded(saveBtn, 0.2f, 4, theme.accent);
    float saveW = UI_MeasureText("SAVE CHANGES", 12);
    DrawText("SAVE CHANGES", (int)(saveBtn.x + (saveBtn.width - saveW) / 2), (int)saveBtn.y + 16, 12, theme.text_bright);

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

    // Cancel button action
    DrawRectangleRounded(cancelBtn, 0.2f, 4, theme.bg_card);
    float cancelW = UI_MeasureText("CANCEL", 12);
    DrawText("CANCEL", (int)(cancelBtn.x + (cancelBtn.width - cancelW) / 2), (int)cancelBtn.y + 16, 12, theme.text_muted);

    if (CheckCollisionPointRec(mousePos, cancelBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        show_modal = false;
    }
}

// Render top navigation header bar with brand title, filters, search, metrics, and create button
void UI_DrawHeader(void) {
    Vector2 mousePos = GetMousePosition();
    int screenWidth = GetScreenWidth();

    DrawRectangle(0, 0, screenWidth, 88, theme.bg_sidebar);
    DrawRectangle(0, 87, screenWidth, 1, theme.border);

    DrawText("MINI JIRA", 30, 28, 20, theme.text_bright);
    float fx = 30.0f + UI_MeasureText("MINI JIRA", 20) + 30.0f;

    // Priority filter pill buttons with auto-scaling width
    const char* filter_labels[] = { "All", "Low", "Med", "High", "Urgent" };
    int filter_vals[] = { -1, PRIORITY_LOW, PRIORITY_MEDIUM, PRIORITY_HIGH, PRIORITY_URGENT };

    for (int f = 0; f < 5; f++) {
        float textW = UI_MeasureText(filter_labels[f], 11);
        float fwidth = textW + 24.0f;
        Rectangle fBtn = { fx, 24.0f, fwidth, 38.0f };
        bool isSelected = (current_priority_filter == filter_vals[f]);
        bool hoverF = CheckCollisionPointRec(mousePos, fBtn) && !show_modal;

        DrawRectangleRounded(fBtn, 0.3f, 4, isSelected ? theme.accent : (hoverF ? theme.bg_card_hover : theme.bg_card));
        DrawRectangleRoundedLines(fBtn, 0.3f, 4, isSelected ? theme.accent_hover : theme.border);

        Color tColor = isSelected ? theme.text_bright : theme.text_muted;
        DrawText(filter_labels[f], (int)(fx + (fwidth - textW) / 2.0f), (int)fBtn.y + 11, 11, tColor);

        if (hoverF && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            current_priority_filter = filter_vals[f];
        }
        fx += fwidth + 10.0f;
    }

    // Search filter text input box
    Rectangle searchBox = { fx + 15.0f, 24.0f, 220.0f, 38.0f };
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
        DrawText("Search...", (int)searchBox.x + 14, (int)searchBox.y + 11, 11, theme.text_muted);
    }
    else {
        char searchDisplay[70];
        snprintf(searchDisplay, sizeof(searchDisplay), "%s%s", search_query, (search_box_active && ((int)(GetTime() * 2) % 2 == 0)) ? "|" : "");
        UI_DrawTextClipped(searchDisplay, (int)searchBox.x + 14, (int)searchBox.y + 11, 11, searchBox.width - 28, theme.text_bright);
    }

    // "+ New Task" action button with auto-calculated text width padding
    float newBtnTextW = UI_MeasureText("+ New Task", 13);
    float newBtnW = newBtnTextW + 42.0f;
    Rectangle newBtn = { (float)screenWidth - newBtnW - 30.0f, 22.0f, newBtnW, 44.0f };
    bool hoverNew = CheckCollisionPointRec(mousePos, newBtn);

    DrawRectangleRounded(newBtn, 0.3f, 4, hoverNew ? theme.accent_hover : theme.accent);
    DrawText("+ New Task", (int)(newBtn.x + (newBtn.width - newBtnTextW) / 2.0f), (int)newBtn.y + 13, 13, theme.text_bright);

    if (hoverNew && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !show_modal) {
        OpenModalForCreate();
    }

    // Right-aligned KPI performance summary metrics
    const char* inProgStr = TextFormat("In Progress: %d", g_data.in_progress_sp);
    const char* doneStr = TextFormat("Done: %d", g_data.done_sp);
    const char* totalStr = TextFormat("Total SP: %d", g_data.total_sp);

    float rightX = newBtn.x - 30.0f;
    rightX -= UI_MeasureText(inProgStr, 13);
    DrawText(inProgStr, (int)rightX, 36, 13, theme.status_in_progress);

    rightX -= 30.0f + UI_MeasureText(doneStr, 13);
    DrawText(doneStr, (int)rightX, 36, 13, theme.status_done);

    rightX -= 30.0f + UI_MeasureText(totalStr, 13);
    DrawText(totalStr, (int)rightX, 36, 13, theme.text_normal);
}

// Return accent color mapping for given task priority level
static Color GetPriorityColor(TaskPriority priority) {
    switch (priority) {
    case PRIORITY_LOW:    return theme.priority_low;
    case PRIORITY_MEDIUM: return theme.priority_medium;
    case PRIORITY_HIGH:   return theme.priority_high;
    case PRIORITY_URGENT: return theme.priority_urgent;
    default:              return theme.text_muted;
    }
}

// Render left sidebar container with scrolling system audit logs
static void UI_DrawSidebarAuditLog(void) {
    int startY = 105;
    int sidebarWidth = 320;
    int sidebarHeight = GetScreenHeight() - startY - 25;

    Rectangle sidebarRect = { 30, (float)startY, (float)sidebarWidth, (float)sidebarHeight };

    DrawRectangleRounded(sidebarRect, 0.03f, 4, theme.bg_sidebar);
    DrawRectangleRoundedLines(sidebarRect, 0.03f, 4, theme.border);

    DrawText("SYSTEM AUDIT LOG", (int)sidebarRect.x + 18, (int)sidebarRect.y + 18, 12, theme.text_bright);
    DrawText("Recent system activities", (int)sidebarRect.x + 18, (int)sidebarRect.y + 38, 9, theme.text_muted);

    DrawLine((int)sidebarRect.x + 18, (int)sidebarRect.y + 58, (int)sidebarRect.x + sidebarWidth - 18, (int)sidebarRect.y + 58, theme.border);

    int itemY = (int)sidebarRect.y + 74;
    int stepY = (int)(40.0f * FONT_SCALE * 0.55f);
    int maxLogs = (sidebarHeight - 90) / stepY;
    int startIndex = (g_data.log_count > maxLogs) ? g_data.log_count - maxLogs : 0;

    if (g_data.log_count == 0) {
        DrawText("No activity recorded", (int)sidebarRect.x + 18, itemY, 10, theme.text_muted);
    }
    else {
        for (int i = g_data.log_count - 1, count = 0; i >= startIndex && count < maxLogs; i--, count++) {
            ActivityLog* log = &g_data.logs[i];

            DrawText(log->timestamp, (int)sidebarRect.x + 18, itemY, 9, theme.text_muted);
            UI_DrawTextClipped(log->message, (int)sidebarRect.x + 18, itemY + (int)(12.0f * FONT_SCALE * 0.55f), 10, sidebarRect.width - 36.0f, theme.text_bright);

            itemY += stepY;
        }
    }
}

// Render main Kanban columns, workflow lanes, and interactive task cards
void UI_DrawKanbanBoard(void) {
    UI_DrawSidebarAuditLog();

    Vector2 mousePos = GetMousePosition();

    int startX = 370;
    int startY = 105;
    int colGap = 18;
    int colWidth = (GetScreenWidth() - startX - 30 - (colGap * 3)) / 4;
    int colHeight = GetScreenHeight() - startY - 25;

    for (int col = 0; col < STATUS_COUNT; col++) {
        int x = startX + col * (colWidth + colGap);
        Rectangle colRect = { (float)x, (float)startY, (float)colWidth, (float)colHeight };

        DrawRectangleRounded(colRect, 0.02f, 4, theme.bg_column);
        DrawRectangleRoundedLines(colRect, 0.02f, 4, theme.border);
        DrawText(status_labels[col], x + 18, startY + 18, 12, theme.text_bright);

        int cardY = startY + 56;
        float cardHeight = 165.0f; // Perfect proportional card height for FONT_SCALE 1.7f

        for (int i = 0; i < g_data.task_count; i++) {
            Task* t = &g_data.tasks[i];
            if (!t->active || t->status != col) continue;

            if (current_priority_filter != -1 && t->priority != current_priority_filter) continue;

            if (search_query[0] != '\0') {
                bool matchTitle = (FindSubstringIgnoreCase(t->title, search_query) != NULL);
                bool matchAssignee = (FindSubstringIgnoreCase(t->assignee, search_query) != NULL);
                if (!matchTitle && !matchAssignee) continue;
            }

            Rectangle cardRect = { (float)x + 12, (float)cardY, (float)colWidth - 24, cardHeight };
            bool cardHover = CheckCollisionPointRec(mousePos, cardRect) && !show_modal;
            Rectangle prioBar = { cardRect.x, cardRect.y + 14, 4, cardRect.height - 28 };
            Rectangle delBtn = { cardRect.x + cardRect.width - 30, cardRect.y + 10, 22, 22 };
            bool hoverDel = CheckCollisionPointRec(mousePos, delBtn) && !show_modal;

            DrawRectangleRounded((Rectangle) { cardRect.x + 3, cardRect.y + 3, cardRect.width, cardRect.height }, 0.04f, 4, (Color) { 0, 0, 0, 100 });
            DrawRectangleRounded(cardRect, 0.04f, 4, cardHover ? theme.bg_card_hover : theme.bg_card);
            DrawRectangleRoundedLines(cardRect, 0.04f, 4, cardHover ? theme.accent : theme.border);

            DrawRectangleRounded(prioBar, 0.5f, 2, GetPriorityColor(t->priority));

            DrawText("x", (int)delBtn.x + 6, (int)delBtn.y + 2, 13, hoverDel ? theme.danger : theme.text_muted);

            if (hoverDel && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                Data_DeleteTask(t->id);
                break;
            }

            if (t->status > STATUS_TODO) {
                Rectangle moveBackBtn = { cardRect.x + cardRect.width - 58, cardRect.y + cardRect.height - 30, 22, 22 };
                bool hoverMoveBack = CheckCollisionPointRec(mousePos, moveBackBtn) && !show_modal;
                DrawText("<", (int)moveBackBtn.x + 6, (int)moveBackBtn.y + 2, 12, hoverMoveBack ? theme.accent : theme.text_muted);

                if (hoverMoveBack && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    Data_MoveTask(t->id, t->status - 1);
                    break;
                }
            }

            if (t->status < STATUS_DONE) {
                Rectangle moveBtn = { cardRect.x + cardRect.width - 30, cardRect.y + cardRect.height - 30, 22, 22 };
                bool hoverMove = CheckCollisionPointRec(mousePos, moveBtn) && !show_modal;
                DrawText(">", (int)moveBtn.x + 6, (int)moveBtn.y + 2, 12, hoverMove ? theme.accent : theme.text_muted);

                if (hoverMove && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    Data_MoveTask(t->id, t->status + 1);
                    break;
                }
            }

            if (cardHover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !hoverDel) {
                OpenModalForEdit(t);
            }

            float textMaxWidth = cardRect.width - 32.0f;

            DrawText(TextFormat("MJ-%d", t->id), (int)cardRect.x + 16, (int)cardRect.y + 12, 10, theme.text_muted);
            UI_DrawTextClipped(t->title, (int)cardRect.x + 16, (int)cardRect.y + 34, 12, textMaxWidth, theme.text_bright);
            UI_DrawTextWrapped(t->description, (int)cardRect.x + 16, (int)cardRect.y + 64, 10, textMaxWidth, 2, theme.text_normal);
            UI_DrawTextClipped(TextFormat("%s  |  %d SP", t->assignee, t->story_points), (int)cardRect.x + 16, (int)(cardRect.y + cardRect.height - 30), 10, textMaxWidth - 60.0f, theme.text_muted);

            cardY += (int)cardHeight + 12;
        }
    }

    if (show_modal) {
        DrawModal();
    }
}
