import pandas as pd
import numpy as np
import subprocess
import os
import json
import re
import ctypes
import threading
import time
from pathlib import Path
from sklearn.tree import DecisionTreeClassifier
from sklearn.metrics import accuracy_score, precision_score, recall_score, f1_score


def _build_subprocess_kwargs_for_silent_run():
    """Return platform-safe subprocess kwargs that keep C execution silent on Windows."""
    kwargs = {}
    if os.name == "nt":
        kwargs["creationflags"] = subprocess.CREATE_NO_WINDOW
        startup_info = subprocess.STARTUPINFO()
        startup_info.dwFlags |= subprocess.STARTF_USESHOWWINDOW
        kwargs["startupinfo"] = startup_info
    return kwargs

class DecisionTreeEngine:
    def __init__(self, csv_path, exe_path="decision_tree.exe", work_dir=None, dot_exe_path=None):
        self.csv_path = str(Path(csv_path).resolve())
        self.exe_path = str(Path(exe_path).resolve())
        self.dot_exe_path = str(Path(dot_exe_path).resolve()) if dot_exe_path else None
        if work_dir is None:
            output_dir = Path(self.exe_path).resolve().parent.parent
        else:
            output_dir = Path(work_dir).resolve()
        output_dir.mkdir(parents=True, exist_ok=True)
        self.project_root = str(output_dir)
        self.df = None
        self.X_processed = None
        self.y_true = None
        self.model = None
        self.y_pred_sklearn = None
        self.y_pred_c = None
        self.last_c_stdout = ""
        self.last_c_stderr = ""
        self.last_predictions_path = ""
        self.last_python_runtime_sec = 0.0
        self.last_python_peak_memory_mb = None
        self.last_c_runtime_sec = 0.0
        self.last_c_peak_memory_mb = None
        self.last_c_max_depth_reached = None

    def load_data(self):
        """טוענת את הנתונים ומבצעת התאמה בסיסית (Preprocessing)"""
        self.df = pd.read_csv(self.csv_path)
        X = self.df.iloc[:, :-1].copy()
        self.y_true = self.df.iloc[:, -1].astype(str).str.strip()

        # טיפול בנתונים טקסטואליים והפיכתם לקטגוריות מספריות
        self.X_processed = X.copy()
        for col in self.X_processed.columns:
            if not pd.api.types.is_numeric_dtype(self.X_processed[col]):
                self.X_processed[col] = pd.Categorical(self.X_processed[col]).codes
            else:
                self.X_processed[col] = self.X_processed[col].fillna(self.X_processed[col].median())

    def train_sklearn(self, depth=4):
        """מאמנת את עץ ההחלטה של sklearn (המקבילה למודל ה-C)"""
        def sklearn_workload():
            self.model = DecisionTreeClassifier(
                criterion='entropy',
                max_depth=depth,
                random_state=42
            )
            self.model.fit(self.X_processed, self.y_true)
            self.y_pred_sklearn = self.model.predict(self.X_processed)

        elapsed_sec, peak_memory_mb = self._measure_current_process_memory_windows(sklearn_workload)
        self.last_python_runtime_sec = elapsed_sec
        self.last_python_peak_memory_mb = peak_memory_mb
        return self._get_metrics(
            self.y_true,
            self.y_pred_sklearn,
            runtime_sec=self.last_python_runtime_sec,
            peak_memory_mb=self.last_python_peak_memory_mb,
        )

    def run_c_algorithm(self, depth=4, enable_visuals=False):
        """מריצה את קובץ ה-C המקומפל וטוענת את התחזיות שלו"""
        if not os.path.exists(self.exe_path):
            raise FileNotFoundError(f"לא נמצא קובץ הרצה בנתיב: {self.exe_path}")

        # הרצת ה-C במצב core-only כדי למדוד נטו אלגוריתם ללא ויזואליזציה
        command = [
            self.exe_path,
            self.csv_path,
            "--no-pause",
        ]
        if not enable_visuals:
            command.append("--no-visuals")
        command.append(str(int(depth)))
        silent_subprocess_kwargs = _build_subprocess_kwargs_for_silent_run()
        child_env = os.environ.copy()
        if self.dot_exe_path and os.path.exists(self.dot_exe_path):
            child_env["GRAPHVIZ_DOT"] = self.dot_exe_path
            dot_dir = str(Path(self.dot_exe_path).parent)
            child_env["PATH"] = dot_dir + os.pathsep + child_env.get("PATH", "")

        process = subprocess.Popen(
            command,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            cwd=self.project_root,
            stdin=subprocess.DEVNULL,
            env=child_env,
            **silent_subprocess_kwargs,
        )

        start_time = time.perf_counter()
        baseline_memory = self._get_process_working_set_bytes(process.pid) or 0
        peak_memory_bytes = baseline_memory
        while process.poll() is None:
            current_memory = self._get_process_working_set_bytes(process.pid)
            if current_memory is not None:
                peak_memory_bytes = max(peak_memory_bytes, current_memory)
            time.sleep(0.02)

        stdout_text, stderr_text = process.communicate()
        end_memory = self._get_process_working_set_bytes(process.pid)
        if end_memory is not None:
            peak_memory_bytes = max(peak_memory_bytes, end_memory)

        self.last_c_runtime_sec = time.perf_counter() - start_time
        peak_delta_bytes = max(0, peak_memory_bytes - baseline_memory)
        self.last_c_peak_memory_mb = (
            peak_delta_bytes / (1024 * 1024) if peak_delta_bytes > 0 else None
        )

        self.last_c_stdout = stdout_text.strip()
        self.last_c_stderr = stderr_text.strip()
        self.last_c_max_depth_reached = self._extract_max_depth_reached(self.last_c_stdout)
        if process.returncode != 0:
            stderr_text = self.last_c_stderr
            stdout_text = self.last_c_stdout
            details = stderr_text if stderr_text else stdout_text
            raise RuntimeError(f"C executable failed. {details}")

        # טעינת התוצאות מהקובץ שה-C מייצר (predictions.csv)
        predictions_path = os.path.join(self.project_root, "predictions.csv")
        self.last_predictions_path = predictions_path
        if os.path.exists(predictions_path):
            pred_df = pd.read_csv(predictions_path, skiprows=1)
            self.y_pred_c = pred_df['predicted_label'].astype(str).str.strip().values
            return self._get_metrics(
                self.y_true,
                self.y_pred_c,
                runtime_sec=self.last_c_runtime_sec,
                peak_memory_mb=self.last_c_peak_memory_mb,
            )
        return None

    def _extract_max_depth_reached(self, stdout_text):
        if not stdout_text:
            return None
        match = re.search(r"MAX_DEPTH_REACHED:\s*(\d+)", stdout_text)
        if match:
            return int(match.group(1))
        return None

    def _measure_current_process_memory_windows(self, workload):
        """מריץ עומס עבודה ומחזיר זמן ריצה + שיא Working Set של התהליך הנוכחי (Windows)"""
        if os.name != "nt":
            start_time = time.perf_counter()
            workload()
            return time.perf_counter() - start_time, None

        done = threading.Event()
        error_holder = []

        def run_workload():
            try:
                workload()
            except Exception as exc:
                error_holder.append(exc)
            finally:
                done.set()

        current_pid = os.getpid()
        baseline_memory = self._get_process_working_set_bytes(current_pid) or 0
        peak_memory_bytes = baseline_memory
        start_time = time.perf_counter()

        worker = threading.Thread(target=run_workload, daemon=True)
        worker.start()

        while not done.is_set():
            current_memory = self._get_process_working_set_bytes(current_pid)
            if current_memory is not None:
                peak_memory_bytes = max(peak_memory_bytes, current_memory)
            done.wait(0.02)

        worker.join()
        end_memory = self._get_process_working_set_bytes(current_pid)
        if end_memory is not None:
            peak_memory_bytes = max(peak_memory_bytes, end_memory)

        if error_holder:
            raise error_holder[0]

        elapsed = time.perf_counter() - start_time
        peak_delta = max(0, peak_memory_bytes - baseline_memory)
        peak_memory_mb = (peak_delta / (1024 * 1024)) if peak_delta > 0 else None
        return elapsed, peak_memory_mb

    def _get_process_working_set_bytes(self, pid):
        """מחזירה Working Set נוכחי של תהליך ב-Windows, או None אם לא זמין"""
        if os.name != "nt":
            return None

        PROCESS_QUERY_INFORMATION = 0x0400
        PROCESS_VM_READ = 0x0010

        class PROCESS_MEMORY_COUNTERS(ctypes.Structure):
            _fields_ = [
                ("cb", ctypes.c_ulong),
                ("PageFaultCount", ctypes.c_ulong),
                ("PeakWorkingSetSize", ctypes.c_size_t),
                ("WorkingSetSize", ctypes.c_size_t),
                ("QuotaPeakPagedPoolUsage", ctypes.c_size_t),
                ("QuotaPagedPoolUsage", ctypes.c_size_t),
                ("QuotaPeakNonPagedPoolUsage", ctypes.c_size_t),
                ("QuotaNonPagedPoolUsage", ctypes.c_size_t),
                ("PagefileUsage", ctypes.c_size_t),
                ("PeakPagefileUsage", ctypes.c_size_t),
            ]

        kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)
        psapi = ctypes.WinDLL("psapi", use_last_error=True)

        process_handle = kernel32.OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, False, pid)
        if not process_handle:
            return None

        try:
            counters = PROCESS_MEMORY_COUNTERS()
            counters.cb = ctypes.sizeof(PROCESS_MEMORY_COUNTERS)
            ok = psapi.GetProcessMemoryInfo(process_handle, ctypes.byref(counters), counters.cb)
            if not ok:
                return None
            return int(counters.WorkingSetSize)
        finally:
            kernel32.CloseHandle(process_handle)

    def get_comparison(self):
        """משווה בין שני המודלים ומחזירה את אחוז ההסכמה ביניהם"""
        if self.y_pred_sklearn is None or self.y_pred_c is None:
            return None
        
        matches = (self.y_pred_sklearn == self.y_pred_c).sum()
        agreement = (matches / len(self.y_true)) * 100
        return {
            "agreement_rate": agreement,
            "count_diff": len(self.y_true) - matches
        }

    def generate_visual_tree(self, output_path="tree_viz.png"):
        """מייצרת תמונה של העץ (דורש Graphviz מותקן במערכת)"""
        if self.model is None:
            return

        dot_path = os.path.join(self.project_root, "temp.dot")
        self._write_custom_dot(dot_path)

        try:
            dot_cmd = self.dot_exe_path if self.dot_exe_path and os.path.exists(self.dot_exe_path) else "dot"
            child_env = os.environ.copy()
            if self.dot_exe_path and os.path.exists(self.dot_exe_path):
                dot_dir = str(Path(self.dot_exe_path).parent)
                child_env["PATH"] = dot_dir + os.pathsep + child_env.get("PATH", "")
            subprocess.run([dot_cmd, "-Tpng", dot_path, "-o", output_path], check=True, env=child_env)
            return output_path
        except Exception:
            return None
        finally:
            if os.path.exists(dot_path):
                try:
                    os.remove(dot_path)
                except OSError:
                    pass

    def _write_custom_dot(self, dot_path):
        tree = self.model.tree_
        feature_names = list(self.X_processed.columns)
        class_names = [str(c) for c in self.model.classes_]

        with open(dot_path, "w", encoding="utf-8") as f:
            f.write("digraph DecisionTree {\n")
            f.write("    graph [rankdir=TB, nodesep=0.5, ranksep=0.5, fontname=\"Arial\"];\n")

            def write_node(node_id):
                left_id = tree.children_left[node_id]
                right_id = tree.children_right[node_id]
                is_leaf = left_id == right_id

                if is_leaf:
                    values = tree.value[node_id][0]
                    total = float(values.sum())
                    best_idx = int(values.argmax())
                    predicted_class = class_names[best_idx]

                    parts = [f"Leaf: {predicted_class}"]
                    for i, cls_name in enumerate(class_names):
                        count = int(values[i])
                        pct = (100.0 * values[i] / total) if total > 0 else 0.0
                        parts.append(f"{cls_name}: {pct:.2f}% ({count}/{int(total)})")

                    label = "\\n".join(parts)
                    f.write(
                        f"    node{node_id} [label=\"{label}\", shape=box, style=filled, fillcolor=\"#FFF3CD\"];\n"
                    )
                    return

                feature_idx = tree.feature[node_id]
                threshold = tree.threshold[node_id]
                feature_name = feature_names[feature_idx]
                n_samples = tree.n_node_samples[node_id]
                values = tree.value[node_id][0]
                
                # בנייה של תווית עם התפלגות המחלקות
                class_dist = " | ".join([f"{class_names[i]}: {int(values[i])}" for i in range(len(class_names))])
                question = f"{feature_name} <= {threshold:.3f}\\n(Number of samples={n_samples})\\n{class_dist}"
                f.write(
                    f"    node{node_id} [label=\"{question}\", shape=ellipse, style=filled, fillcolor=\"#DDEFFF\"];\n"
                )

                write_node(left_id)
                write_node(right_id)

                f.write(f"    node{node_id} -> node{left_id} [label=\"True\", color=\"green\", penwidth=2];\n")
                f.write(f"    node{node_id} -> node{right_id} [label=\"False\", color=\"red\"];\n")

            write_node(0)
            f.write("}\n")

    def _get_metrics(self, y_true, y_pred, runtime_sec=None, peak_memory_mb=None):
        """פונקציה פנימית לחישוב מדדים"""
        return {
            "accuracy": accuracy_score(y_true, y_pred),
            "precision": precision_score(y_true, y_pred, average='weighted', zero_division=0),
            "recall": recall_score(y_true, y_pred, average='weighted', zero_division=0),
            "f1": f1_score(y_true, y_pred, average='weighted', zero_division=0),
            "runtime_sec": runtime_sec,
            "peak_memory_mb": peak_memory_mb,
        }