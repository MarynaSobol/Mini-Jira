# Mini Jira Dashboard

A desktop Kanban board and task management application written in C using the Raylib graphics library

## Contibutors
- **Maryna Sobol**
- **Mariia Vanaga**

## Screenshot
<img width="1854" height="1097" alt="MiniJiraScreenshot" src="https://github.com/user-attachments/assets/5ca429d7-bdee-41b7-8286-182b18b2d057" />

  
## Features

- **Kanban Workflow**: Manage tasks across four status columns: `To Do`, `In Progress`, `In Review`, and `Done`
- **CRUD Operations**: Create, read, update, and soft-delete tasks with automatic `MJ-XXX` ID assignment
- **Task Metadata**: Track task title, description, assignee, priority (`Low`, `Medium`, `High`, `Urgent`), and Story Points
- **Story Point Estimation**: Automatic default Story Point suggestions based on selected task priority (1, 3, 5, or 8 SP)
- **KPI Metrics**: Real-time calculation of total, in-progress, and completed Story Points
- **Activity Log**: Rolling history of the last 20 board events with `HH:MM:SS` system timestamps
- **Binary Persistence**: Automatic state saving and loading via `board_data.bin`
- **Custom UI Theme**: Dark slate and indigo color palette with custom TrueType font (`font.ttf`) support and bilinear texture filtering

## Project Structure

- `main.c` — Application entry point, window initialization (1850x1050, 60 FPS), and main render loop
- `data.h` / `data.c` — Core data structures (`Task`, `ActivityLog`, `BoardData`), CRUD logic, KPI calculation, and binary file I/O
- `theme.h` / `theme.c` — Color palette definitions and custom font loading/unloading
- `ui.h` / `ui.c` — Header bar and Kanban board rendering routines
- `font.ttf` — Custom TrueType font loaded at runtime (falls back to Raylib default font if absent)
- `board_data.bin` — Binary file storing serialized board state, tasks, and activity logs
- `mini-jira.vcxproj` / `packages.config` — Visual Studio project configuration and NuGet dependency manifest

## Requirements

- **OS**: Windows 10 or later
- **IDE / Compiler**: Visual Studio (Platform Toolset v145 / MSVC)
- **Dependencies**: Raylib 5.5.0 (managed via NuGet)

## Building and Running

1. Clone the repository.
2. Open `mini-jira.vcxproj` in Visual Studio
3. Restore NuGet packages to download `raylib 5.5.0` (the project expects the package in `..\packages\raylib.5.5.0\`)
4. Select your build configuration (`Debug` or `Release`, `x64` or `Win32`)
5. Build and run the project.

## Data Persistence

The application reads from and writes to `board_data.bin` in the working directory:
- On startup, `Data_Init()` attempts to load existing data from `board_data.bin`.
- If no file is found, the board initializes with four default sample tasks.
- Every task creation, update, status movement, or deletion immediately recalculates Story Point statistics and writes the updated `BoardData` struct to `board_data.bin`.

## Capacities and Limits

Defined in `data.h`:
- `MAX_TASKS`: 100
- `LOG_HISTORY_SIZE`: 20
- `MAX_TITLE_LEN`: 64 bytes
- `MAX_DESC_LEN`: 128 bytes
- `MAX_ASSIGNEE_LEN`: 32 bytes
