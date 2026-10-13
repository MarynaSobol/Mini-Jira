#ifndef UI_H
#define UI_H

#include "raylib.h"

// User interface module initialization function prototype
void UI_Init(void);

// Renders the top dashboard header including application title and KPI counters
void UI_DrawHeader(void);

// Renders the main Kanban board columns, layout, and task cards
void UI_DrawKanbanBoard(void);

#endif