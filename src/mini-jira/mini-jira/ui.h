#ifndef UI_H
#define UI_H

#include "raylib.h"

// Initializes UI subsystem components
void UI_Init(void);

// Renders header bar, KPI story points, and "+ New Task" button
void UI_DrawHeader(void);

// Renders Kanban columns, interactive task cards, and modal dialog
void UI_DrawKanbanBoard(void);

#endif // UI_H