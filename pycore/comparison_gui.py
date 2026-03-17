import threading
from pathlib import Path
import time
import tkinter as tk
from tkinter import filedialog, messagebox

import customtkinter as ctk
from PIL import Image

try:
    from .LMForComparison import DecisionTreeEngine
except ImportError:
    from LMForComparison import DecisionTreeEngine


class ComparisonApp(ctk.CTk):
    def __init__(self):
        super().__init__()

        self.project_root = Path(__file__).resolve().parent.parent 
        self.default_csv = self.project_root / "data" / "iris.csv" # נתיב ברירת המחדל לקובץ ה-CSV
        self.exe_path = self.project_root / "build" / "decision_tree.exe" # נתיב ברירת המחדל לקובץ ה-C המהודר
        self.tree_img_path = self.project_root / "tree_viz.png" # נתיב לתמונה שתיווצר עבור עץ ההחלטה של הפייתון
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

    def _set_status(self, text):
        self.status_box.delete("1.0", "end")
        self.status_box.insert("1.0", text)

    def _append_status(self, text):
        self.status_box.insert("end", f"\n{text}")
        self.status_box.see("end")

    def _on_depth_change(self, _value):
        self.depth_label.configure(text=f"Selected depth: {self.depth_var.get()}")

    def browse_csv(self):
        chosen = filedialog.askopenfilename(
            title="Select CSV dataset",
            initialdir=str(self.project_root / "data"),
            filetypes=[("CSV files", "*.csv"), ("All files", "*.*")],
        )
        if chosen:
            self.csv_var.set(chosen)

    def start_comparison(self):
        csv_path = Path(self.csv_var.get().strip())
        if not csv_path.exists() or csv_path.suffix.lower() != ".csv":
            messagebox.showerror("Invalid file", "Please choose a valid CSV file.")
            return

        self.run_btn.configure(state="disabled", text="Running...")
        self._set_status("Starting comparison...")

        worker = threading.Thread(target=self._run_pipeline, args=(csv_path,), daemon=True)
        worker.start()

    def _run_pipeline(self, csv_path):
        start_time = time.time()
        try:
            engine = DecisionTreeEngine(str(csv_path), exe_path=str(self.exe_path))
            self.after(0, self._append_status, f"Dataset: {csv_path.name}")
            self.after(0, self._append_status, "Loading data...")
            engine.load_data()

            self.after(0, self._append_status, "Training Python model...")
            py_metrics = engine.train_sklearn(depth=int(self.depth_var.get()))

            self.after(0, self._append_status, "Running C executable...")
            c_metrics = engine.run_c_algorithm()
            if c_metrics is None:
                raise RuntimeError("C model did not produce predictions.csv")

            self.after(0, self._append_status, "C executable finished successfully.")
            if engine.last_c_stdout:
                first_line = engine.last_c_stdout.splitlines()[0]
                self.after(0, self._append_status, f"C output: {first_line}")
            self.after(0, self._append_status, f"Predictions file: {engine.last_predictions_path}")

            comparison = engine.get_comparison()
            tree_path = engine.generate_visual_tree(output_path=str(self.tree_img_path))
            c_tree_path = self.project_root / "tree.png"

            self.after(0, self._render_metrics, py_metrics, c_metrics, comparison)
            self.after(0, self._render_predictions, engine)
            self.after(0, self._render_tree_images, tree_path, str(c_tree_path))
            elapsed = time.time() - start_time
            self.after(0, self._append_status, f"Done. Comparison completed successfully in {elapsed:.2f}s.")
        except Exception as exc:
            self.after(0, messagebox.showerror, "Comparison error", str(exc))
            self.after(0, self._append_status, f"Error: {exc}")
        finally:
            self.after(0, self._finish_run)

    def _render_metrics(self, py_metrics, c_metrics, comparison):
        rows = [
            ["Accuracy", f"{py_metrics['accuracy']:.4f}", f"{c_metrics['accuracy']:.4f}"],
            ["Precision", f"{py_metrics['precision']:.4f}", f"{c_metrics['precision']:.4f}"],
            ["Recall", f"{py_metrics['recall']:.4f}", f"{c_metrics['recall']:.4f}"],
            ["F1 Score", f"{py_metrics['f1']:.4f}", f"{c_metrics['f1']:.4f}"],
        ]

        if comparison:
            rows.append(["Agreement Rate", f"{comparison['agreement_rate']:.2f}%", "-"])
            rows.append(["Different Predictions", str(comparison["count_diff"]), "-"])

        lines = self._format_table(["Metric", "Python", "C"], rows)

        self.metrics_box.configure(state="normal")
        self.metrics_box.delete("1.0", "end")
        self.metrics_box.insert("1.0", "\n".join(lines))
        self.metrics_box.configure(state="disabled")

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

    def _format_table(self, headers, rows):
        widths = [len(header) for header in headers]
        for row in rows:
            for idx, value in enumerate(row):
                widths[idx] = max(widths[idx], len(str(value)))

        def format_row(values):
            return "| " + " | ".join(str(values[i]).ljust(widths[i]) for i in range(len(values))) + " |"

        separator = "+-" + "-+-".join("-" * width for width in widths) + "-+"
        table_lines = [separator, format_row(headers), separator]
        for row in rows:
            table_lines.append(format_row(row))
        table_lines.append(separator)
        return table_lines

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

    def _finish_run(self):
        self.run_btn.configure(state="normal", text="Run Comparison")

    def on_close(self):
        cleanup_paths = [
            self.project_root / "tree.dot",
            self.project_root / "tree.png",
            self.project_root / "tree_viz.png",
            self.project_root / "temp.dot",
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


if __name__ == "__main__":
    app = ComparisonApp()
    app.mainloop()
