import pandas as pd
import numpy as np
import subprocess
import os
import json
from pathlib import Path
from sklearn.tree import DecisionTreeClassifier
from sklearn.metrics import accuracy_score, precision_score, recall_score, f1_score

class DecisionTreeEngine:
    def __init__(self, csv_path, exe_path="decision_tree.exe"):
        self.csv_path = str(Path(csv_path).resolve())
        self.exe_path = str(Path(exe_path).resolve())
        self.project_root = str(Path(self.exe_path).resolve().parent.parent)
        self.df = None
        self.X_processed = None
        self.y_true = None
        self.model = None
        self.y_pred_sklearn = None
        self.y_pred_c = None
        self.last_c_stdout = ""
        self.last_c_stderr = ""
        self.last_predictions_path = ""

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
        self.model = DecisionTreeClassifier(
            criterion='entropy', 
            max_depth=depth, 
            random_state=42
        )
        self.model.fit(self.X_processed, self.y_true)
        self.y_pred_sklearn = self.model.predict(self.X_processed)
        return self._get_metrics(self.y_true, self.y_pred_sklearn)

    def run_c_algorithm(self):
        """מריצה את קובץ ה-C המקומפל וטוענת את התחזיות שלו"""
        if not os.path.exists(self.exe_path):
            raise FileNotFoundError(f"לא נמצא קובץ הרצה בנתיב: {self.exe_path}")

        # הרצת ה-C (שולח את ה-CSV כארגומנט)
        result = subprocess.run(
            [self.exe_path, self.csv_path, "--no-pause"],
            capture_output=True,
            text=True,
            cwd=self.project_root,
        )
        self.last_c_stdout = result.stdout.strip()
        self.last_c_stderr = result.stderr.strip()
        if result.returncode != 0:
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
            return self._get_metrics(self.y_true, self.y_pred_c)
        return None

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
            subprocess.run(["dot", "-Tpng", dot_path, "-o", output_path], check=True)
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
                        pct = (100.0 * values[i] / total) if total > 0 else 0.0
                        parts.append(f"{cls_name}: {pct:.2f}%")

                    label = "\\n".join(parts)
                    f.write(
                        f"    node{node_id} [label=\"{label}\", shape=box, style=filled, fillcolor=\"#FFF3CD\"];\n"
                    )
                    return

                feature_idx = tree.feature[node_id]
                threshold = tree.threshold[node_id]
                feature_name = feature_names[feature_idx]
                question = f"{feature_name} <= {threshold:.3f}"
                f.write(
                    f"    node{node_id} [label=\"{question}\", shape=ellipse, style=filled, fillcolor=\"#DDEFFF\"];\n"
                )

                write_node(left_id)
                write_node(right_id)

                f.write(f"    node{node_id} -> node{left_id} [label=\"True\", color=\"green\", penwidth=2];\n")
                f.write(f"    node{node_id} -> node{right_id} [label=\"False\", color=\"red\"];\n")

            write_node(0)
            f.write("}\n")

    def _get_metrics(self, y_true, y_pred):
        """פונקציה פנימית לחישוב מדדים"""
        return {
            "accuracy": accuracy_score(y_true, y_pred),
            "precision": precision_score(y_true, y_pred, average='weighted', zero_division=0),
            "recall": recall_score(y_true, y_pred, average='weighted', zero_division=0),
            "f1": f1_score(y_true, y_pred, average='weighted', zero_division=0)
        }