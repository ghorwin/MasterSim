Concept for User Interface of MasterSimulator
=============================================

Key ideas
---------
- Single-Document-Interface
- Model-View Architecture
- (Undo-)-Action-based project data modification
- avoid item views whenever possible (simplfies implementation)

Implementation priorities
-------------------------
- basic project handling
- post-processing
- simulation start functionality (only start from begin)

Project Handling
----------------
- New
- Open...
- Save
- Save as...
  ask user to copy result directory as well
- Recent projects...
- Quit

Simulation
----------
Execution in separate thread, MT::fetch(), Thread::run(), MT::store() principle

- Start/Continue
- Step-by-Step execution
- Stop
- Restart/Rewind

Post-Processing
---------------
Expects results to be in directory with same name as project (otherwise 
external post-processing has to be used).

### Navigation tree ###

- Treeview of output files with quantities (updated on Project load)
- Treeview of output variables based on . Notation

### Diagramm view ###
- grid based, user can specify number of rows/columns, diagrams are always 
vertically aligned
- all share same time axis
- diagrams can be selected, widget background/chart frame changes color
- there is always one diagram active

### Diagram modification ###
- Double-click on Quantity in Tree view adds Variable to current diagram
- update button re-reads output files and updates data series in diagram
- double-click on chart opens (non-modal) customization widget with all properties
- line serias dialog has "appearance presets": line style and color, marker-style and color as table widget

### Data-Structure ###
- array of diagram data corresponding to grid
- diagram data holds set of line/marker series:
  - for each series:
    - encoded: file#variable-name
    - visualization data (line/marker type/color), y-axis
  - x-axis data and scale (always time)
  - y1-axis data and scale
  - y2-axis data and scale (always on, if no series, synced to y1)
  - grid info

### Convenience ###
- user can define/remember variable presets ("variable - appearance - preset")

