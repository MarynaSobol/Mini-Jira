#ifndef DATA_H
#define DATA_H

#include <stdbool.h>

// Field size limits and array capacities
#define MAX_TITLE_LEN 64
#define MAX_DESC_LEN 128
#define MAX_ASSIGNEE_LEN 32
#define MAX_TASKS 100
#define LOG_HISTORY_SIZE 20

// Task workflow statuses
typedef enum {
    STATUS_TODO = 0,
    STATUS_IN_PROGRESS,
    STATUS_IN_REVIEW,
    STATUS_DONE,
    STATUS_COUNT
} TaskStatus;

// Task urgency levels
typedef enum {
    PRIORITY_LOW = 0,
    PRIORITY_MEDIUM,
    PRIORITY_HIGH,
    PRIORITY_URGENT,
    PRIORITY_COUNT
} TaskPriority;

// Core data structure representing a single Jira task
typedef struct {
    int id;
    char title[MAX_TITLE_LEN];
    char description[MAX_DESC_LEN];
    char assignee[MAX_ASSIGNEE_LEN];
    TaskStatus status;
    TaskPriority priority;
    int story_points;
    bool active; // Flag for soft deletion
} Task;

// Log entry structure for tracking board events
typedef struct {
    char message[128];
    char timestamp[16];
} ActivityLog;

// Global board container storing tasks, logs, and calculated metrics
typedef struct {
    Task tasks[MAX_TASKS];
    int task_count;
    int next_id;

    ActivityLog logs[LOG_HISTORY_SIZE];
    int log_count;

    // Aggregated KPI statistics
    int total_sp;
    int done_sp;
    int in_progress_sp;
} BoardData;

// Global instance of the board state
extern BoardData g_data;

// Core lifecycle and CRUD data management functions
void Data_Init(void);
bool Data_AddTask(const char* title, const char* desc, const char* assignee, TaskStatus status, TaskPriority priority, int sp);
bool Data_UpdateTask(int task_id, const char* title, const char* desc, const char* assignee, TaskStatus status, TaskPriority priority, int sp);
bool Data_DeleteTask(int task_id);
bool Data_MoveTask(int task_id, TaskStatus new_status);

// Helpers for logging, recalculating metrics, hybrid SP estimation, and file persistence
void Data_LogActivity(const char* message);
void Data_RecalculateStats(void);
int Data_SuggestStoryPoints(TaskPriority priority);
void Data_SaveToFile(const char* filename);
void Data_LoadFromFile(const char* filename);

#endif // DATA_H
