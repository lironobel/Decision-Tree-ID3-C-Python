<<<<<<< HEAD
# C-Engine Decision Tree Classifier

Decision Tree classifier implemented in C (ID3/C4.5 style), with a Python layer for model comparison and visualization.
=======
**Overview:**
A high-performance Decision Tree engine implemented in C from scratch, featuring a modern Python-based visualization and comparison dashboard.
This project compares a "raw" C implementation against the industry-standard Scikit-Learn.

# C-Engine Decision Tree Classifier 🌲

A high-performance Decision Tree engine implemented in C, featuring a modern Python-based visualization dashboard.
>>>>>>> gui-build

## Overview

<<<<<<< HEAD
This project combines:

- A custom Decision Tree engine written in C.
- Dataset loading, entropy/information gain calculations, and recursive tree building.
- Prediction export to CSV.
- Python tools for comparison against scikit-learn models.

## Project Structure

- `src/` - C source files (core algorithm and pipeline).
- `include/` - C headers.
- `build/` - compiled executable output.
- `data/` - datasets (for example: `iris.csv`, `cars.csv`, `adult.csv`).
- `pycore/` - Python scripts (comparison and optional UI tools).
- `predictions.csv` - model prediction output file.
- `tree.dot` - Graphviz DOT file representation of the decision tree.

## Features

- Decision Tree implementation in C from scratch.
- Manual memory management and recursive tree construction.
- Information Gain based split selection.
- CSV output for predictions.
- Python-based comparison with scikit-learn.

## Tech Stack

- Backend: C (GCC)
- Scripting and comparison: Python
- Visualization format: Graphviz (DOT)
- Version control: Git

## Build and Run

You can use the predefined VS Code tasks:

1. `Build C Project` - compiles `build/decision_tree.exe`
2. `Run Decision Tree` - runs the C executable
3. `Run Python ML Comparison` - runs Python comparison script
4. `Run Full Pipeline (C + Python)` - full pipeline in one step

If you prefer manual build from terminal:
=======
- **C-Core:** Custom implementation of decision tree logic from scratch.
- **Smart Optimization:** Implemented Dynamic Binning (max 25 splits) to handle large datasets (30k+ rows) efficiently.
- **Memory Efficient:** Manual memory management and optimized recursive structures.
- **Interactive Studio:** A customtkinter dashboard for real-time benchmarking and visualizatio.
- **Tree Inspector:** Export and view visual tree structures using Graphviz integration.
- **Memory Conscious:** Optimized C structures with manual memory management to prevent leaks.

## Tech Stack

- **Backend:** C (GCC, Make)
- **Fronted UI:** Python (customtkinter)
- **Data Analysis:** pandas, scikit-learn, numpy
- **Visualization:** Graphviz & Pillow
- **Version Control:** Git

## Optimization Highlight

In large datasets (e.g., 32,000 rows), a naive search for the best split point can be extremely slow. I implemented a Histogram-based Binning approach. By limiting the search to 25 strategic "jumps" per feature, I reduced execution time for large files from 26 seconds to less than 1 second, with minimal impact on accuracy.

## Getting Started (Windows)

Follow these steps in order:

1. Install Graphviz

- Download and install Graphviz from: https://graphviz.org/download/
- Make sure it is installed at: `C:\Program Files\Graphviz\bin\dot.exe`

2. Open terminal in project root

- Folder should be the one that contains `src`, `pycore`, `build`, and `requirements.txt`.

3. Create Python virtual environment

```powershell
python -m venv .venv
```

4. Activate virtual environment

```powershell
.\.venv\Scripts\Activate.ps1
```

5. Install Python dependencies

```powershell
python -m pip install --upgrade pip
python -m pip install -r requirements.txt
```

6. Build the C executable
>>>>>>> gui-build

```powershell
gcc -Wall -Wextra -g3 -Iinclude src/main.c src/dataset.c src/infogain.c src/utils.c src/tree.c src/buildTree.c src/tree_graph.c src/pretictedvalues.c -o build/decision_tree.exe
```

<<<<<<< HEAD
Then run:

```powershell
./build/decision_tree.exe
```

## Python Comparison

Run from project root:

```powershell
python pycore/LMForComparison.py
```

## Output Files

- `predictions.csv` - generated predictions from the C model.
- `tree.dot` - exported decision tree graph in DOT format (can be rendered with Graphviz).

## Notes

- File `src/pretictedvalues.c` keeps the current project naming and is compiled as-is.
- This repository includes both C and Python flow to support performance and correctness comparison.
=======
7. Run the dashboard

```powershell
python pycore/comparison_gui.py
```

8. Generate and view trees

- Click `Run Comparison` in the GUI.
- Open the `Trees` tab to view Python tree and C tree.

## Common Errors & Fixes

1. `ModuleNotFoundError: No module named 'PIL'`

- Fix: install `Pillow`:

```powershell
python -m pip install Pillow
```

2. `ModuleNotFoundError: No module named 'pandas'`

- Fix: install all dependencies:

```powershell
python -m pip install -r requirements.txt
```

3. `'C:\Program' is not recognized ...`

- Reason: Graphviz path/command issue on Windows.
- Fix: make sure Graphviz is installed in `C:\Program Files\Graphviz\bin`.

4. C tree image not visible in GUI

- Click `Run Comparison` again and check the `Trees` tab while the app is still open.
- Ensure C build completed successfully and `build/decision_tree.exe` exists.

## 🚀 Recent Performance Optimization (March 2026)

### 1. Label Mapping & Data Integrity

Fixed a critical logical bug in the C-Engine where leaf nodes used local class indexing instead of a global canonical mapping.

- **Impact:** Resolved prediction inconsistencies, increasing model accuracy from ~70% to **96.1%**.

### 2. Dynamic Depth Synchronization

Synchronized the GUI's Max Depth slider with the C-Engine's recursive build process via CLI arguments.

- **Impact:** The C model is no longer hard-coded to a depth of 4, allowing for complex tree generation (tested up to Depth 8+).

### 3. Execution Logging

Implemented real-time status updates in the GUI to track C-Engine execution flow and parameter passing.
>>>>>>> gui-build
