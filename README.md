**Overview:**
A high-performance Decision Tree engine implemented in C from scratch, featuring a modern Python-based visualization and comparison dashboard.
This project compares a "raw" C implementation against the industry-standard Scikit-Learn.

# C-Engine Decision Tree Classifier 🌲

A high-performance Decision Tree engine implemented in C, featuring a modern Python-based visualization dashboard.

## Key Features

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

## Getting Started:

Before running the project, ensure you have all the required Python libraries installed by - pip install -r requirements.txt.

Note: You also need Graphviz installed on your system to visualize the decision trees.

gcc -o build/decision_tree.exe src/\*.c - in Terminal

Launch the Dashboard: python pycore/comparison_gui.py

## 🚀 Recent Performance Optimization (March 2026)

### 1. Label Mapping & Data Integrity

Fixed a critical logical bug in the C-Engine where leaf nodes used local class indexing instead of a global canonical mapping.

- **Impact:** Resolved prediction inconsistencies, increasing model accuracy from ~70% to **96.1%**.

### 2. Dynamic Depth Synchronization

Synchronized the GUI's Max Depth slider with the C-Engine's recursive build process via CLI arguments.

- **Impact:** The C model is no longer hard-coded to a depth of 4, allowing for complex tree generation (tested up to Depth 8+).

### 3. Execution Logging

Implemented real-time status updates in the GUI to track C-Engine execution flow and parameter passing.
