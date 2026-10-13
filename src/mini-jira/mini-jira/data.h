#ifndef DATA_H
#define DATA_H

#include <stdbool.h>

#define MAX_TITLE_LEN 64
#define MAX_DESC_LEN 128
#define MAX_ASSIGNEE_LEN 32
#define MAX_TASKS 100
#define LOG_HISTORY_SIZE 20

typedef enum {
    STATUS_TODO = 0,
    STATUS_IN_PROGRESS,
    STATUS_IN_REVIEW,
    STATUS_DONE,
    STATUS_COUNT
} TaskStatus;

typedef enum {
    PRIORITY_LOW = 0,
    PRIORITY_MEDIUM,
    PRIORITY_HIGH,
    PRIORITY_URGENT,
    PRIORITY_COUNT
} TaskPriority;

typedef struct {
    int id;
    char title[MAX_TITLE_LEN];
    char description[MAX_DESC_LEN];
    char assignee[MAX_ASSIGNEE_LEN];
    TaskStatus status;
    TaskPriority priority;
    int story_points;
    bool active;
} Task;

typedef struct {
    char message[128];
    char timestamp[16];
} ActivityLog;

typedef struct {
    Task tasks[MAX_TASKS];
    int task_count;
    int next_id;

    ActivityLog logs[LOG_HISTORY_SIZE];
    int log_count;

    int total_sp;
    int done_sp;
    int in_progress_sp;
} BoardData;

extern BoardData g_data;

void Data_Init(void);
bool Data_AddTask(const char* title, const char* desc, const char* assignee, TaskStatus status, TaskPriority priority, int sp);
bool Data_UpdateTask(int task_id, const char* title, const char* desc, const char* assignee, TaskStatus status, TaskPriority priority, int sp);
bool Data_DeleteTask(int task_id);
bool Data_MoveTask(int task_id, TaskStatus new_status);

void Data_LogActivity(const char* message);
void Data_RecalculateStats(void);
void Data_SaveToFile(const char* filename);
void Data_LoadFromFile(const char* filename);

#endif