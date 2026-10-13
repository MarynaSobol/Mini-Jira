#define _CRT_SECURE_NO_WARNINGS // Suppress MSVC unsafe string/time warnings
#include "data.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// Global board data definition
BoardData g_data;

// Formats the current system time into HH:MM:SS format
static void GetCurrentTimeString(char* buffer, size_t size) {
    time_t rawtime;
    struct tm* timeinfo;
    time(&rawtime);

#if defined(_MSC_VER)
    struct tm result;
    if (localtime_s(&result, &rawtime) == 0) {
        strftime(buffer, size, "%H:%M:%S", &result);
    }
    else {
        snprintf(buffer, size, "00:00:00");
    }
#else
    timeinfo = localtime(&rawtime);
    if (timeinfo) strftime(buffer, size, "%H:%M:%S", timeinfo);
    else snprintf(buffer, size, "00:00:00");
#endif
}

// Appends a new message to the rolling activity history
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

// Recalculates total, completed, and in-progress Story Points across active tasks
void Data_RecalculateStats(void) {
    g_data.total_sp = 0;
    g_data.done_sp = 0;
    g_data.in_progress_sp = 0;

    for (int i = 0; i < g_data.task_count; i++) {
        if (!g_data.tasks[i].active) continue;

        int sp = g_data.tasks[i].story_points;
        g_data.total_sp += sp;

        if (g_data.tasks[i].status == STATUS_DONE) {
            g_data.done_sp += sp;
        }
        else if (g_data.tasks[i].status == STATUS_IN_PROGRESS) {
            g_data.in_progress_sp += sp;
        }
    }
}

// Returns a suggested default Story Point value matching task priority
int Data_SuggestStoryPoints(TaskPriority priority) {
    switch (priority) {
    case PRIORITY_LOW:    return 1;
    case PRIORITY_MEDIUM: return 3;
    case PRIORITY_HIGH:   return 5;
    case PRIORITY_URGENT: return 8;
    default:              return 3;
    }
}

// Initializes data structures and loads existing binary state from disk
void Data_Init(void) {
    memset(&g_data, 0, sizeof(BoardData));
    g_data.next_id = 101;

    Data_LoadFromFile("board_data.bin");

    if (g_data.task_count == 0) {
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

// Creates a new task and appends it to the active board
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

// Modifies all fields of an existing task by ID
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

// Marks a task as inactive (soft delete)
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

// Updates the workflow status of a task
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

// Writes the complete state structure into a binary file
void Data_SaveToFile(const char* filename) {
    FILE* file = fopen(filename, "wb");
    if (!file) return;
    fwrite(&g_data, sizeof(BoardData), 1, file);
    fclose(file);
}

// Reads and populates the board structure from a binary file
void Data_LoadFromFile(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) return;
    fread(&g_data, sizeof(BoardData), 1, file);
    fclose(file);
    Data_RecalculateStats();
}
