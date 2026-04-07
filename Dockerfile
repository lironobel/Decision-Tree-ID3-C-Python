# 1. בחירת תמונת בסיס - לינוקס עם פייתון מותקן
FROM python:3.11-slim

# 2. התקנת כלי מערכת: GCC לקומפילציה ו-Graphviz ליצירת העץ
RUN apt-get update && apt-get install -y \
    gcc \
    graphviz \
    python3-tk \
    libtk8.6 \
    && rm -rf /var/lib/apt/lists/*

# 3. הגדרת תיקיית עבודה בתוך הקונטיינר
WORKDIR /app

# 4. העתקת קובץ הדרישות והתקנת ספריות פייתון
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# 5. העתקת כל קבצי הפרויקט פנימה
COPY . .

# 6. יצירת תיקיית build וקומפילציה של מנוע ה-C
RUN mkdir -p build && \
    gcc -Wall -Wextra -g3 -Iinclude \
    src/main.c src/dataset.c src/infogain.c src/utils.c \
    src/tree.c src/buildTree.c src/tree_graph.c src/pretictedvalues.c \
    -o build/decision_tree.exe

# 7. פקודת ההרצה (כאן יש "אבל" קטן לגבי ה-GUI)
CMD ["python", "src/main_gui.py"]''

