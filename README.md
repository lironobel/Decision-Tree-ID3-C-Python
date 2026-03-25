# C-Engine Decision Tree Classifier

Decision Tree classifier implemented in C (ID3/C4.5 style), with a Python layer for model comparison and visualization.

## Overview

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

```powershell
gcc -Wall -Wextra -g3 -Iinclude src/main.c src/dataset.c src/infogain.c src/utils.c src/tree.c src/buildTree.c src/tree_graph.c src/pretictedvalues.c -o build/decision_tree.exe
```

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
