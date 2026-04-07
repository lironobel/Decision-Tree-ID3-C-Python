import os
import shutil
import sys
import threading
from pathlib import Path
import time
import tkinter as tk
from tkinter import filedialog, messagebox


import customtkinter as ctk
from PIL import Image


def _prepare_tk_runtime_for_frozen_app():
    """In onefile builds, avoid picking Graphviz Tcl 8.6.10 from system PATH."""
    if not getattr(sys, "frozen", False):
        return

    mei_base = getattr(sys, "_MEIPASS", None)
    if mei_base:
        tcl_dir = Path(mei_base) / "_tcl_data"
        tk_dir = Path(mei_base) / "_tk_data"
        if tcl_dir.exists():
            os.environ["TCL_LIBRARY"] = str(tcl_dir)
        if tk_dir.exists():
            os.environ["TK_LIBRARY"] = str(tk_dir)

    path_value = os.environ.get("PATH", "")
    if not path_value:
        return

    # Remove system Graphviz bin from DLL search order to prevent Tcl mismatch.
    filtered_parts = []
    for part in path_value.split(os.pathsep):
        if "graphviz" in part.lower() and "bin" in part.lower():
            continue
        filtered_parts.append(part)
    os.environ["PATH"] = os.pathsep.join(filtered_parts)


_prepare_tk_runtime_for_frozen_app()

try:
    from .LMForComparison import DecisionTreeEngine
except ImportError:
    from LMForComparison import DecisionTreeEngine

def resource_path(relative_path):
    """ קבלת נתיב אבסולוטי לקבצים, עובד גם בפיתוח וגם ב-EXE """
    try:
        # PyInstaller יוצר תיקייה זמנית ושומר את הנתיב ב-sys._MEIPASS
        base_path = sys._MEIPASS
    except Exception:
        base_path = os.path.abspath(".")
    return os.path.join(base_path, relative_path)

    
class ComparisonApp(ctk.CTk):
    def __init__(self):
        super().__init__()

        # נתיב הבסיס להרצה רגילה
        self.project_root = Path(os.path.abspath(".")).resolve()
        self.runtime_output_dir = Path(os.environ.get("LOCALAPPDATA", str(Path.home()))) / "DecisionTreeStudio"
        self.runtime_output_dir.mkdir(parents=True, exist_ok=True)
        self.default_csv = resource_path("data/iris.csv") # נתיב ברירת המחדל לקובץ ה-CSV
        self.exe_path = resource_path("build/decision_tree.exe") # נתיב ברירת המחדל לקובץ ה-C המהודר
        self.dot_exe_path = self._resolve_dot_executable()
        self.tree_img_path = self.runtime_output_dir / "tree_viz.png" # נתיב לתמונה שתיווצר עבור עץ ההחלטה של הפייתון
        self.c_tree_img_path = self.runtime_output_dir / "tree.png"
        self.current_python_tree_image = None
        self.current_c_tree_image = None
        self.python_tree_preview_path = None
        self.c_tree_preview_path = None
        self.preview_images = []
        self.protocol("WM_DELETE_WINDOW", self.on_close)

        ctk.set_appearance_mode("light") # מצב בהיר כברירת מחדל
        ctk.set_default_color_theme("blue") 

        # הגדרת תכונות החלון הראשי
        self.title("Decision Tree Comparison Studio")
        self.geometry("1180x760")
        self.minsize(980, 660)

        # הגדרת רשת עמודות ושורות עבור הפריסה
        self.grid_columnconfigure(0, weight=3)
        self.grid_columnconfigure(1, weight=8)
        self.grid_rowconfigure(0, weight=1)
        
        #  יצירת הפאנלים הראשיים
        self.left_panel = ctk.CTkFrame(self, corner_radius=18, fg_color="#F5F1EA")
        self.left_panel.grid(row=0, column=0, sticky="nsew", padx=(14, 7), pady=14)
        self.left_panel.grid_columnconfigure(0, weight=1)
        self.left_panel.configure(width=320)
        self.left_panel.grid_propagate(False)

        # הפאנל הימני
        self.right_panel = ctk.CTkFrame(self, corner_radius=18, fg_color="#E9F1F7")
        self.right_panel.grid(row=0, column=1, sticky="nsew", padx=(7, 14), pady=14)
        self.right_panel.grid_columnconfigure(0, weight=1)
        self.right_panel.grid_rowconfigure(2, weight=1)
        self.right_panel.configure(width=860)
        self.right_panel.grid_propagate(False)

        # בניית התוכן בכל פאנל
        self._build_left_panel()
        self._build_right_panel()

    # שיטות עזר לבניית הממשק והפונקציונליות
    def _build_left_panel(self):
        header = ctk.CTkLabel(
            self.left_panel,
            text="Model Comparison Lab",
            font=("Bahnschrift", 34, "bold"),
            text_color="#12324A",
        )
        header.grid(row=0, column=0, sticky="w", padx=22, pady=(22, 6))

        sub = ctk.CTkLabel(
            self.left_panel,
            text="Compare Python (scikit-learn) vs C decision-tree outputs on the same dataset.",
            font=("Segoe UI", 14),
            text_color="#2E4E62",
            wraplength=460,
            justify="left",
        )
        sub.grid(row=1, column=0, sticky="w", padx=22, pady=(0, 14))

        file_frame = ctk.CTkFrame(self.left_panel, corner_radius=14, fg_color="#FFFFFF")
        file_frame.grid(row=2, column=0, sticky="ew", padx=22, pady=(0, 14))
        file_frame.grid_columnconfigure(0, weight=1)

        ctk.CTkLabel(
            file_frame,
            text="Dataset CSV",
            font=("Bahnschrift", 16, "bold"),
            text_color="#12324A",
        ).grid(row=0, column=0, sticky="w", padx=14, pady=(12, 8))

        self.csv_var = tk.StringVar(value=str(self.default_csv))
        self.csv_entry = ctk.CTkEntry(file_frame, textvariable=self.csv_var, height=38)
        self.csv_entry.grid(row=1, column=0, sticky="ew", padx=(14, 10), pady=(0, 12))

        browse_btn = ctk.CTkButton(
            file_frame,
            text="Browse",
            width=98,
            height=38,
            fg_color="#1F6E8C",
            hover_color="#18566C",
            command=self.browse_csv,
        )
        browse_btn.grid(row=1, column=1, sticky="e", padx=(0, 14), pady=(0, 12))

        depth_frame = ctk.CTkFrame(self.left_panel, corner_radius=14, fg_color="#FFFFFF")
        depth_frame.grid(row=3, column=0, sticky="ew", padx=22, pady=(0, 14))
        depth_frame.grid_columnconfigure(0, weight=1)

        ctk.CTkLabel(
            depth_frame,
            text="Max Depth (Python Model)",
            font=("Bahnschrift", 16, "bold"),
            text_color="#12324A",
        ).grid(row=0, column=0, sticky="w", padx=14, pady=(12, 8))

        self.depth_var = tk.IntVar(value=4)
        self.depth_slider = ctk.CTkSlider(
            depth_frame,
            from_=2,
            to=30,
            number_of_steps=28,
            variable=self.depth_var,
            progress_color="#1F6E8C",
            command=self._on_depth_change,
        )
        self.depth_slider.grid(row=1, column=0, sticky="ew", padx=14, pady=(0, 8))

        self.depth_label = ctk.CTkLabel(
            depth_frame,
            text=f"Selected depth: {self.depth_var.get()}",
            font=("Segoe UI", 13),
            text_color="#2E4E62",
        )
        self.depth_label.grid(row=2, column=0, sticky="w", padx=14, pady=(0, 12))

        self.run_btn = ctk.CTkButton(
            self.left_panel,
            text="Run Comparison",
            height=44,
            fg_color="#E36414",
            hover_color="#B44D10",
            font=("Bahnschrift", 17, "bold"),
            command=self.start_comparison,
        )
        self.run_btn.grid(row=4, column=0, sticky="ew", padx=22, pady=(0, 14))

        ctk.CTkLabel(
            self.left_panel,
            text="Live Status",
            font=("Bahnschrift", 15, "bold"),
            text_color="#12324A",
        ).grid(row=5, column=0, sticky="w", padx=22, pady=(2, 6))

        self.status_box = ctk.CTkTextbox(self.left_panel, height=170, corner_radius=12)
        self.status_box.grid(row=6, column=0, sticky="nsew", padx=22, pady=(0, 18))
        self.left_panel.grid_rowconfigure(6, weight=1)
        self._set_status("Ready. Select a CSV file and click 'Run Comparison'.")

    # שיטה לבניית הפאנל הימני שבו יוצגו התוצאות, כולל טבלת המדדים, טבלת התחזיות ותמונות העצים
    def _build_right_panel(self):
        ctk.CTkLabel(
            self.right_panel,
            text="Results Dashboard",
            font=("Bahnschrift", 30, "bold"),
            text_color="#12324A",
        ).grid(row=0, column=0, sticky="w", padx=22, pady=(20, 8))

        self.metrics_box = ctk.CTkTextbox(self.right_panel, height=170, corner_radius=12)
        self.metrics_box.grid(row=1, column=0, sticky="ew", padx=22, pady=(0, 12))
        self.metrics_box.configure(font=("Consolas", 13))
        self.metrics_box.insert("1.0", "Comparison table will appear here after running the models.")
        self.metrics_box.configure(state="disabled")

        self.result_tabs = ctk.CTkTabview(self.right_panel, corner_radius=14)
        self.result_tabs.grid(row=2, column=0, sticky="nsew", padx=22, pady=(0, 22))

        self.predictions_tab = self.result_tabs.add("Predictions")
        self.predictions_tab.grid_columnconfigure(0, weight=1)
        self.predictions_tab.grid_rowconfigure(1, weight=1)

        ctk.CTkLabel(
            self.predictions_tab,
            text="Predictions Comparison",
            font=("Bahnschrift", 15, "bold"),
            text_color="#12324A",
        ).grid(row=0, column=0, sticky="w", padx=12, pady=(10, 6))

        self.predictions_box = ctk.CTkTextbox(self.predictions_tab, corner_radius=10)
        self.predictions_box.grid(row=1, column=0, sticky="nsew", padx=12, pady=(0, 12))
        self.predictions_box.configure(font=("Consolas", 12))
        self.predictions_box.insert("1.0", "Run comparison to view prediction table.")
        self.predictions_box.configure(state="disabled")

        self.images_tab = self.result_tabs.add("Trees")
        self.images_tab.grid_columnconfigure(0, weight=1)
        self.images_tab.grid_columnconfigure(1, weight=1)
        self.images_tab.grid_rowconfigure(1, weight=1)

        ctk.CTkLabel(
            self.images_tab,
            text="Python Tree",
            font=("Bahnschrift", 15, "bold"),
            text_color="#12324A",
        ).grid(row=0, column=0, sticky="w", padx=12, pady=(10, 6))

        ctk.CTkLabel(
            self.images_tab,
            text="C Tree",
            font=("Bahnschrift", 15, "bold"),
            text_color="#12324A",
        ).grid(row=0, column=1, sticky="w", padx=12, pady=(10, 6))

        self.python_tree_label = ctk.CTkLabel(
            self.images_tab,
            text="Python tree image will appear here.",
            font=("Segoe UI", 13),
            text_color="#4B6678",
        )
        self.python_tree_label.grid(row=1, column=0, sticky="nsew", padx=(12, 8), pady=(0, 12))

        self.c_tree_label = ctk.CTkLabel(
            self.images_tab,
            text="C tree image will appear here.",
            font=("Segoe UI", 13),
            text_color="#4B6678",
        )
        self.c_tree_label.grid(row=1, column=1, sticky="nsew", padx=(8, 12), pady=(0, 12))

    # שיטה שמעדכנת את תיבת הסטטוס עם טקסט חדש, מוחקת את הקודם ומציגה רק את ההודעה החדשה
    def _set_status(self, text):
        self.status_box.delete("1.0", "end")
        self.status_box.insert("1.0", text)

    # שיטה שמוסיפה שורה חדשה לתיבת הסטטוס מבלי למחוק את הקודם, ומגלילה אוטומטית לתחתית כדי להראות את העדכון האחרון
    def _append_status(self, text):
        self.status_box.insert("end", f"\n{text}")
        self.status_box.see("end")

    # שיטה שמעדכנת את התווית שמתחת לסליידר עומק כדי להראות את הערך הנבחר בזמן אמת#
    def _on_depth_change(self, _value):
        self.depth_label.configure(text=f"Selected depth: {self.depth_var.get()}")

    # שיטה עיקרית שמריצה את מודל הפייתון, מריצה את ה-C, טוענת את התוצאות ומשווה ביניהם
    def browse_csv(self):
        chosen = filedialog.askopenfilename(
            title="Select CSV dataset",
            initialdir=str(self.project_root / "data"),
            filetypes=[("CSV files", "*.csv"), ("All files", "*.*")],
        )
        if chosen:
            self.csv_var.set(chosen)

    # מחלקה עיקרית שמטפלת בכל הלוגיקה של טעינת הנתונים, הרצת המודלים, השוואת התוצאות ויצירת העצים הויזואליים
    def start_comparison(self):
        csv_path = Path(self.csv_var.get().strip())
        if not csv_path.exists() or csv_path.suffix.lower() != ".csv":
            messagebox.showerror("Invalid file", "Please choose a valid CSV file.")
            return

        self.run_btn.configure(state="disabled", text="Running...")
        self._set_status("Starting comparison...")

        worker = threading.Thread(target=self._run_pipeline, args=(csv_path,), daemon=True)
        worker.start()

    # שיטה עיקרית שמריצה את כל תהליך ההשוואה, עם טיפול בשגיאות ועדכון הסטטוס בזמן אמת
    def _run_pipeline(self, csv_path):
        start_time = time.time()
        try:
            engine = DecisionTreeEngine(
                str(csv_path),
                exe_path=str(self.exe_path),
                work_dir=str(self.runtime_output_dir),
                dot_exe_path=self.dot_exe_path,
            )
            self.after(0, self._append_status, f"Dataset: {csv_path.name}")
            self.after(0, self._append_status, "Loading data...")
            engine.load_data()

            self.after(0, self._append_status, "Training Python model...")
            selected_depth = int(self.depth_var.get())
            py_metrics = engine.train_sklearn(depth=selected_depth)

            self.after(0, self._append_status, "Running C executable...")
            c_metrics = engine.run_c_algorithm(depth=selected_depth, enable_visuals=True)
            if c_metrics is None:
                raise RuntimeError("C model did not produce predictions.csv")

            self.after(0, self._append_status, "C executable finished successfully.")
            if engine.last_c_max_depth_reached is not None:
                self.after(0, self._append_status, f"C max depth reached: {engine.last_c_max_depth_reached}")
            if engine.last_c_stdout:
                first_line = engine.last_c_stdout.splitlines()[0]
                self.after(0, self._append_status, f"C output: {first_line}")
            self.after(0, self._append_status, f"Predictions file: {engine.last_predictions_path}")
            if self.dot_exe_path:
                self.after(0, self._append_status, f"Graphviz dot: {self.dot_exe_path}")
            else:
                self.after(0, self._append_status, "Graphviz dot not found (images may fail).")

            comparison = engine.get_comparison()
            tree_path = engine.generate_visual_tree(output_path=str(self.tree_img_path))
            c_tree_path = self.c_tree_img_path

            self.after(0, self._render_metrics, py_metrics, c_metrics, comparison)
            self.after(0, self._render_predictions, engine)
            self.after(0, self._render_tree_images, tree_path, str(c_tree_path))
            elapsed = time.time() - start_time
        except Exception as exc:
            self.after(0, messagebox.showerror, "Comparison error", str(exc))
            self.after(0, self._append_status, f"Error: {exc}")
        finally:
            self.after(0, self._finish_run)

    # שיטה עזר להצגת טבלת מדדים של שני המודלים והשוואתם, כולל אחוז ההסכמה ביניהם    
    def _render_metrics(self, py_metrics, c_metrics, comparison):
        def fmt_accuracy_percent(value):
            return f"{value * 100:.2f}%" if value is not None else "N/A"

        def fmt_metric(value):
            return f"{value:.4f}" if value is not None else "N/A"

        def fmt_runtime(value):
            return f"{value:.4f}" if value is not None else "N/A"

        def fmt_memory(value):
            return f"{value:.2f}" if value is not None else "N/A"

        rows = [
            [
                "Python",
                fmt_accuracy_percent(py_metrics.get("accuracy")),
                fmt_metric(py_metrics.get("precision")),
                fmt_metric(py_metrics.get("recall")),
                fmt_metric(py_metrics.get("f1")),
                fmt_runtime(py_metrics.get("runtime_sec")),
                fmt_memory(py_metrics.get("peak_memory_mb")),
            ],
            [
                "C",
                fmt_accuracy_percent(c_metrics.get("accuracy")),
                fmt_metric(c_metrics.get("precision")),
                fmt_metric(c_metrics.get("recall")),
                fmt_metric(c_metrics.get("f1")),
                fmt_runtime(c_metrics.get("runtime_sec")),
                fmt_memory(c_metrics.get("peak_memory_mb")),
            ],
        ]

        lines = self._format_table(
            ["Model", "Accuracy", "Precision", "Recall", "F1 Score", "Runtime (s)", "Peak Memory (MB)"],
            rows,
        )

        if comparison:
            lines.append("")
            lines.append(f"Agreement Rate (Py vs C): {comparison['agreement_rate']:.2f}%")

        self.metrics_box.configure(state="normal")
        self.metrics_box.delete("1.0", "end")
        self.metrics_box.insert("1.0", "\n".join(lines))
        self.metrics_box.configure(state="disabled")

    # שיטה עזר להצגת טבלת התחזיות עם הדגשת התאמות ושגיאות בין שני המודלים
    def _render_predictions(self, engine):
        total_rows = min(len(engine.y_true), len(engine.y_pred_sklearn), len(engine.y_pred_c))
        rows = []

        for idx in range(total_rows):
            row_id = idx + 1
            true_label = str(engine.y_true.iloc[idx])
            py_pred = str(engine.y_pred_sklearn[idx])
            c_pred = str(engine.y_pred_c[idx])
            match_flag = "Yes" if py_pred == c_pred else "No"
            rows.append([str(row_id), true_label, py_pred, c_pred, match_flag])

        lines = self._format_table_minimal(["Row", "True Label", "Python", "C", "Match"], rows)
        if total_rows < len(engine.y_true):
            lines.append("")
            lines.append("[WARN] Display limited to common prediction rows.")

        self.predictions_box.configure(state="normal")
        self.predictions_box.delete("1.0", "end")
        self.predictions_box.insert("1.0", "\n".join(lines))
        self.predictions_box.configure(state="disabled")

    # שיטה עזר לטעינת תמונת עץ והצגתה בתוך התווית המתאימה, עם טיפול בשגיאות ויצירת קישורים לתצוגה מוגדלת
    def _render_tree_images(self, python_tree_path, c_tree_path):
        self._render_single_tree_image(
            python_tree_path,
            self.python_tree_label,
            "Could not generate Python tree image. Check Graphviz installation.",
            "python",
        )
        self._render_single_tree_image(
            c_tree_path,
            self.c_tree_label,
            "C tree image was not generated.",
            "c",
        )

    # שיטה עזר לטעינת תמונת עץ והצגתה בתוך התווית המתאימה, עם טיפול בשגיאות ויצירת קישורים לתצוגה מוגדלת
    def _render_single_tree_image(self, path_value, label_widget, error_text, model_type):
        path_obj = Path(path_value) if path_value else None
        if not path_obj or not path_obj.exists():
            label_widget.configure(text=error_text, image=None)
            label_widget.unbind("<Button-1>")
            label_widget.configure(cursor="")
            return

        img = Image.open(path_obj)
        img.thumbnail((320, 240))
        tree_image = ctk.CTkImage(light_image=img, dark_image=img, size=img.size)
        if model_type == "python":
            self.current_python_tree_image = tree_image
            self.python_tree_preview_path = path_obj
        else:
            self.current_c_tree_image = tree_image
            self.c_tree_preview_path = path_obj
        label_widget.configure(text="", image=tree_image)
        label_widget.bind(
            "<Button-1>",
            lambda _event, mt=model_type: self._open_image_preview(mt),
        )
        label_widget.configure(cursor="hand2")

    # עיצוב טבלה בסיסי להצגת מדדים ונתונים בצורה מסודרת בתוך תיבות הטקסט
    def _format_table(self, headers, rows, separator_after=None):
        widths = [len(header) for header in headers]
        for row in rows:
            for idx, value in enumerate(row):
                widths[idx] = max(widths[idx], len(str(value)))

        def format_row(values):
            return "| " + " | ".join(str(values[i]).ljust(widths[i]) for i in range(len(values))) + " |"

        separator = "+-" + "-+-".join("-" * width for width in widths) + "-+"
        table_lines = [separator, format_row(headers), separator]
        for idx, row in enumerate(rows, start=1):
            table_lines.append(format_row(row))
            if separator_after and idx in separator_after:
                table_lines.append(separator)
        table_lines.append(separator)
        return table_lines

    # גרסה מינימלית של טבלת תוצאות להצגה בחלון התחזיות, עם פחות עיצוב כדי להתאים למגבלות רוחב
    def _format_table_minimal(self, headers, rows):
        widths = [len(header) for header in headers]
        for row in rows:
            for idx, value in enumerate(row):
                widths[idx] = max(widths[idx], len(str(value)))

        def format_row(values):
            return " | ".join(str(values[i]).ljust(widths[i]) for i in range(len(values)))

        table_lines = [format_row(headers)]
        for row in rows:
            table_lines.append(format_row(row))
        return table_lines

    # לא בשימוש כרגע, אבל יכול לשמש להרחבה עתידית להצגת העץ בצורה ויזואלית בתוך האפליקציה עצמה
    def _open_image_preview(self, model_type):
        image_path = self.python_tree_preview_path if model_type == "python" else self.c_tree_preview_path
        if not image_path or not image_path.exists():
            messagebox.showwarning("Image not found", "The selected tree image is not available.")
            return

        pil_img = Image.open(image_path)
        max_width = int(self.winfo_screenwidth() * 0.85)
        max_height = int(self.winfo_screenheight() * 0.85)
        display_img = pil_img.copy()
        display_img.thumbnail((max_width, max_height))

        preview = ctk.CTkToplevel(self)
        preview.title("Tree Preview")
        window_width = display_img.width + 30
        window_height = display_img.height + 70
        x_pos = max(20, (self.winfo_screenwidth() - window_width) // 2)
        y_pos = 30
        preview.geometry(f"{window_width}x{window_height}+{x_pos}+{y_pos}")
        preview.minsize(420, 320)
        preview.attributes("-topmost", True)
        preview.lift()
        preview.focus_force()

        preview_label = ctk.CTkLabel(preview, text="")
        preview_label.pack(fill="both", expand=True, padx=10, pady=10)

        preview_img = ctk.CTkImage(light_image=display_img, dark_image=display_img, size=display_img.size)
        self.preview_images.append(preview_img)
        preview_label.configure(image=preview_img)

    # סיום הריצה - הפעלת כפתור מחדש
    def _finish_run(self):
        self.run_btn.configure(state="normal", text="Run Comparison")

    # ניקוי קבצים זמניים בעת סגירת האפליקציה
    def on_close(self):
        cleanup_paths = [
            self.runtime_output_dir / "tree.dot",
            self.runtime_output_dir / "tree.png",
            self.runtime_output_dir / "tree_viz.png",
            self.runtime_output_dir / "temp.dot",
            self.runtime_output_dir / "predictions.csv",
        ]

        for path in cleanup_paths:
            if path.exists():
                try:
                    path.unlink()
                except OSError:
                    pass

        pycache_dir = self.project_root / "pycore" / "__pycache__"
        if pycache_dir.exists():
            patterns = [
                "comparison_gui.cpython-312*.pyc",
                "LMForComparison.cpython-312*.pyc",
            ]
            for pattern in patterns:
                for file_path in pycache_dir.glob(pattern):
                    try:
                        file_path.unlink()
                    except OSError:
                        pass

        self.destroy()

    def _resolve_dot_executable(self):
        bundled_dot = Path(resource_path("graphviz/bin/dot.exe"))
        if bundled_dot.exists():
            return str(bundled_dot)

        system_dot = shutil.which("dot")
        if system_dot:
            return system_dot

        return None


if __name__ == "__main__":
    app = ComparisonApp()
    app.mainloop()
