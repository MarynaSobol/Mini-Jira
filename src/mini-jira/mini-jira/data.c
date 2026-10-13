#include "data.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// Global board data structure instance
BoardData g_data;

// Helper function to get the current local system time as a formatted string
static void GetCurrentTimeString(char* buffer, size_t size) {
    time_t rawtime;
    struct tm timeinfo;
    time(&rawtime);

    // Safely format the local time into hours, minutes, and seconds
    if (localtime_s(&timeinfo, &rawtime) == 0) {
        strftime(buffer, size, "%H:%M:%S", &timeinfo);
    }
    else {
        // Fallback default time string if formatting fails
        snprintf(buffer, size, "00:00:00");
    }
}

// Appends a new message to the activity log history buffer, shifting older entries if full
void Data_LogActivity(const char* message) {
    if (g_data.log_count >= LOG_HISTORY_SIZE) {
        for (int i = 0; i < LOG_HISTORY_SIZE - 1; i++) {
            g_data.logs[i] = g_data.logs[i + 1];
        }
        g_data.log_count = LOG_HISTORY_SIZE - 1;
    }
    ActivityLog* log = &g_data.logs[g_data.log_count++];
    snprintf(log->message, sizeof(log->message), "%s", message);
    GetCurrentTimeString(log->timestamp, sizeof(log->timestamp));
}

// Recalculates total story points and progress metrics based on active tasks
void Data_RecalculateStats(void) {
    g_data.total_sp = 0;
    g_data.done_sp = 0;
    g_data.in_progress_sp = 0;

    for (int i = 0; i < g_data.task_count; i++) {
        if (!g_data.tasks[i].active) continue;
        int sp = g_data.tasks[i].story_points;
        g_data.total_sp += sp;

        if (g_data.tasks[i].status == STATUS_DONE) g_data.done_sp += sp;
        else if (g_data.tasks[i].status == STATUS_IN_PROGRESS) g_data.in_progress_sp += sp;
    }
}

// Initializes the board data, attempts loading from file, and seeds default tasks if empty
void Data_Init(void) {
    memset(&g_data, 0, sizeof(BoardData));
    g_data.next_id = 101;

    Data_LoadFromFile("board_data.bin");

    if (g_data.task_count == 0) {
        // Seed default initial tasks if no saved data was found
        Data_AddTask("Setup Raylib Workspace", "Configure C99 MSVC environment", "Alex R.", STATUS_DONE, PRIORITY_HIGH, 3);
        Data_AddTask("Implement CRUD Operations", "Add Create, Edit, and Delete features", "Marina S.", STATUS_IN_PROGRESS, PRIORITY_URGENT, 5);
        Data_AddTask("UI Redesign & Modern Styling", "Apply dark indigo themes and rounded cards", "Marina S.", STATUS_IN_PROGRESS, PRIORITY_HIGH, 8);
        Data_AddTask("Export & Data Persistence", "Implement binary file saving mechanism", "Dev Team", STATUS_TODO, PRIORITY_MEDIUM, 5);
        Data_LogActivity("Board initialized with default tasks");
    }
    else {
        Data_LogActivity("Board loaded from file");
    }
}

// Creates and adds a new task to the board, then logs the activity, updates stats, and saves
bool Data_AddTask(const char* title, const char* desc, const char* assignee, TaskStatus status, TaskPriority priority, int sp) {
    if (g_data.task_count >= MAX_TASKS) return false;

    Task* t = &g_data.tasks[g_data.task_count++];
    t->id = g_data.next_id++;
    snprintf(t->title, MAX_TITLE_LEN, "%s", title);
    snprintf(t->description, MAX_DESC_LEN, "%s", desc);
    snprintf(t->assignee, MAX_ASSIGNEE_LEN, "%s", assignee);
    t->status = status;
    t->priority = priority;
    t->story_points = sp;
    t->active = true;

    char logMsg[128];
    snprintf(logMsg, sizeof(logMsg), "Created Task MJ-%d: %s", t->id, t->title);
    Data_LogActivity(logMsg);
    Data_RecalculateStats();
    Data_SaveToFile("board_data.bin");
    return true;
}

// Updates details of an existing active task by its ID
bool Data_UpdateTask(int task_id, const char* title, const char* desc, const char* assignee, TaskStatus status, TaskPriority priority, int sp) {
    for (int i = 0; i < g_data.task_count; i++) {
        if (g_data.tasks[i].id == task_id && g_data.tasks[i].active) {
            Task* t = &g_data.tasks[i];
            snprintf(t->title, MAX_TITLE_LEN, "%s", title);
            snprintf(t->description, MAX_DESC_LEN, "%s", desc);
            snprintf(t->assignee, MAX_ASSIGNEE_LEN, "%s", assignee);
            t->status = status;
            t->priority = priority;
            t->story_points = sp;

            char logMsg[128];
            snprintf(logMsg, sizeof(logMsg), "Updated Task MJ-%d", task_id);
            Data_LogActivity(logMsg);
            Data_RecalculateStats();
            Data_SaveToFile("board_data.bin");
            return true;
        }
    }
    return false;
}

// Soft deletes a task by setting its active flag to false
bool Data_DeleteTask(int task_id) {
    for (int i = 0; i < g_data.task_count; i++) {
        if (g_data.tasks[i].id == task_id && g_data.tasks[i].active) {
            g_data.tasks[i].active = false;

            char logMsg[128];
            snprintf(logMsg, sizeof(logMsg), "Deleted Task MJ-%d", task_id);
            Data_LogActivity(logMsg);
            Data_RecalculateStats();
            Data_SaveToFile("board_data.bin");
            return true;
        }
    }
    return false;
}

// Moves an active task to a new workflow status column
bool Data_MoveTask(int task_id, TaskStatus new_status) {
    for (int i = 0; i < g_data.task_count; i++) {
        if (g_data.tasks[i].id == task_id && g_data.tasks[i].active) {
            if (g_data.tasks[i].status != new_status) {
                g_data.tasks[i].status = new_status;

                static const char* statusNames[] = { "To Do", "In Progress", "In Review", "Done" };
                char logMsg[128];
                snprintf(logMsg, sizeof(logMsg), "MJ-%d moved to %s", task_id, statusNames[new_status]);
                Data_LogActivity(logMsg);
                Data_RecalculateStats();
                Data_SaveToFile("board_data.bin");
            }
            return true;
        }
    }
    return false;
}

// Serializes and saves the current board state structure to a binary file
void Data_SaveToFile(const char* filename) {
    FILE* file = fopen(filename, "wb");
    if (!file) return;
    fwrite(&g_data, sizeof(BoardData), 1, file);
    fclose(file);
}

// Deserializes and loads the board state structure from a binary file
void Data_LoadFromFile(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) return;
    fread(&g_data, sizeof(BoardData), 1, file);
    fclose(file);
    Data_RecalculateStats();
}