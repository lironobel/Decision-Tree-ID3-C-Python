import pandas as pd #ספריה לעבודה עם דאטה
import numpy as np #ספריה לעבודה עם מערכים ומטריצות
from sklearn.tree import DecisionTreeClassifier # ייבוא מחלקת Decision Tree מ-sklearn
from sklearn.model_selection import train_test_split #יבוא לפיצול דאטה
from sklearn.metrics import ( 
    accuracy_score,   # חישוב דיוק המודל 
    precision_score,  # חישוב דיוק חיובי
    recall_score,     # חישוב זיהוי חיובי
    f1_score,         # חישוב מדד F1
    confusion_matrix, # חישוב מטריצת בלבול
    classification_report # דוח סיווג מלא
)
from sklearn.tree import export_graphviz, export_text
import matplotlib.pyplot as plt
import seaborn as sns
from pathlib import Path
import json

class DecisionTreeComparison:
    """
    מחלקה להשוואת Decision Tree שנבנה ב-C עם sklearn
    """
    
    # אתחול המחלקה
    def __init__(self, csv_path, predictions_path=None): 
        """
        Args:
            csv_path: נתיב לקובץ ה-CSV המקורי
            predictions_path: נתיב לקובץ התחזיות של האלגוריתם ב-C
        """
        self.csv_path = csv_path
        self.predictions_path = predictions_path
        self.df = None
        self.X = None
        self.y = None
        self.model = None
        self.y_pred_sklearn = None
        self.y_pred_c = None
        
    def load_data(self):
        """טעינת הנתונים מהקובץ"""
        print("📂 טוען נתונים...")
        self.df = pd.read_csv(self.csv_path)
        
        # הפרדה בין features ל-target
        self.X = self.df.iloc[:, :-1]
        self.y = self.df.iloc[:, -1]
        
        print(f"✅ נטענו {len(self.df)} שורות עם {len(self.X.columns)} תכונות")
        print(f"📊 מחלקות: {self.y.unique()}")
        print(f"📈 התפלגות מחלקות:\n{self.y.value_counts()}\n")
        
    def train_sklearn_tree(self, max_depth=15, min_samples_split=10, 
                          criterion='entropy', random_state=42):
        """
        אימון Decision Tree עם sklearn
        
        Args:
            max_depth: עומק מקסימלי של העץ
            min_samples_split: מינימום דגימות לפיצול
            criterion: קריטריון לפיצול (entropy/gini)
            random_state: seed לשחזוריות
        """
        print("🌲 בונה Decision Tree עם sklearn...")
        
        self.model = DecisionTreeClassifier(
            max_depth=max_depth,
            min_samples_split=min_samples_split,
            criterion=criterion,
            random_state=random_state
        )
        
        # אימון על כל הדאטה (כמו שהאלגוריתם שלך עושה)
        self.model.fit(self.X, self.y)
        self.y_pred_sklearn = self.model.predict(self.X)
        
        print(f"✅ העץ נבנה בהצלחה!")
        print(f"📏 עומק העץ: {self.model.get_depth()}")
        print(f"🍃 מספר עלים: {self.model.get_n_leaves()}\n")
        
    def load_c_predictions(self):
        """טעינת התחזיות מהאלגוריתם ב-C"""
        if not self.predictions_path or not Path(self.predictions_path).exists():
            print("⚠️  קובץ התחזיות לא נמצא")
            return False
            
        print("📂 טוען תחזיות מהאלגוריתם ב-C...")
        pred_df = pd.read_csv(self.predictions_path, skiprows=1)
        self.y_pred_c = pred_df['predicted_label'].values
        
        print(f"✅ נטענו {len(self.y_pred_c)} תחזיות\n")
        return True
        
    def calculate_metrics(self, y_true, y_pred, model_name):
        """חישוב מדדי ביצועים"""
        print(f"\n{'='*60}")
        print(f"📊 מדדי ביצועים עבור: {model_name}")
        print(f"{'='*60}")
        
        # Accuracy
        acc = accuracy_score(y_true, y_pred)
        print(f"🎯 Accuracy: {acc:.4f} ({acc*100:.2f}%)")
        
        # Precision, Recall, F1 (weighted average)
        precision = precision_score(y_true, y_pred, average='weighted', zero_division=0)
        recall = recall_score(y_true, y_pred, average='weighted', zero_division=0)
        f1 = f1_score(y_true, y_pred, average='weighted', zero_division=0)
        
        print(f"🔍 Precision: {precision:.4f}")
        print(f"📈 Recall: {recall:.4f}")
        print(f"⚖️  F1-Score: {f1:.4f}")
        
        # Confusion Matrix
        cm = confusion_matrix(y_true, y_pred)
        print(f"\n📋 Confusion Matrix:")
        print(cm)
        
        # Classification Report
        print(f"\n📝 Classification Report:")
        print(classification_report(y_true, y_pred, zero_division=0))
        
        return {
            'accuracy': acc,
            'precision': precision,
            'recall': recall,
            'f1_score': f1,
            'confusion_matrix': cm.tolist()
        }
        
    def compare_predictions(self):
        """השוואה בין התחזיות של sklearn לבין C"""
        if self.y_pred_c is None:
            print("⚠️  אין תחזיות מהאלגוריתם ב-C להשוואה")
            return
            
        print(f"\n{'='*60}")
        print("🔄 השוואה בין המודלים")
        print(f"{'='*60}")
        
        # זהות בתחזיות
        matches = (self.y_pred_sklearn == self.y_pred_c).sum()
        total = len(self.y_pred_sklearn)
        agreement = matches / total
        
        print(f"✅ same predictions: {matches}/{total} ({agreement*100:.2f}%)")
        print(f"❌ different predictions: {total-matches}/{total} ({(1-agreement)*100:.2f}%)")
        
        # השוואת דיוק מול ground truth
        acc_sklearn = accuracy_score(self.y, self.y_pred_sklearn)
        acc_c = accuracy_score(self.y, self.y_pred_c)
        
        print(f"\n🎯 Accuracy sklearn: {acc_sklearn:.4f} ({acc_sklearn*100:.2f}%)")
        print(f"🎯 Accuracy C Algorithm: {acc_c:.4f} ({acc_c*100:.2f}%)")
        print(f"📊 Difference: {abs(acc_sklearn - acc_c):.4f} ({abs(acc_sklearn - acc_c)*100:.2f}%)")
        
        # מציאת דוגמאות עם חילוקי דעות
        disagreements = np.where(self.y_pred_sklearn != self.y_pred_c)[0]
        if len(disagreements) > 0:
            print(f"\n🔍 Different predictions examples (first 5):")
            for idx in disagreements[:5]:
                print(f"  Row {idx}: sklearn={self.y_pred_sklearn[idx]}, "
                      f"C={self.y_pred_c[idx]}, true={self.y.iloc[idx]}")
                
    def visualize_sklearn_tree(self, feature_names=None, output_path="sklearn_tree.dot"):
        """Visualization of sklearn tree for use with Graphviz"""
        if self.model is None:
            print("⚠️  The model has not been trained yet")
            return
            
        print(f"\n💾 Saving sklearn tree to {output_path}...")
        
        if feature_names is None:
            feature_names = self.X.columns.tolist()
            
        class_names = [str(c) for c in self.model.classes_]
        
        export_graphviz(
            self.model,
            out_file=output_path,
            feature_names=feature_names,
            class_names=class_names,
            filled=True,
            rounded=True,
            special_characters=True
        )
        
        print("✅ DOT file created successfully!")
        print(f"💡 Run: dot -Tpng {output_path} -o sklearn_tree.png")
        
    def plot_confusion_matrices(self):
        """Plot confusion matrices for comparison"""
        fig, axes = plt.subplots(1, 2, figsize=(14, 5))
        
        # sklearn confusion matrix
        cm_sklearn = confusion_matrix(self.y, self.y_pred_sklearn)
        sns.heatmap(cm_sklearn, annot=True, fmt='d', cmap='Blues', 
                   ax=axes[0], cbar=False)
        axes[0].set_title('sklearn Decision Tree')
        axes[0].set_xlabel('Predicted')
        axes[0].set_ylabel('Actual')
        
        # C algorithm confusion matrix
        if self.y_pred_c is not None:
            cm_c = confusion_matrix(self.y, self.y_pred_c)
            sns.heatmap(cm_c, annot=True, fmt='d', cmap='Greens',
                       ax=axes[1], cbar=False)
            axes[1].set_title('C Algorithm Decision Tree')
            axes[1].set_xlabel('Predicted')
            axes[1].set_ylabel('Actual')
        
        plt.tight_layout()
        plt.savefig('confusion_matrix_comparison.png', dpi=300, bbox_inches='tight')
        print("✅ Confusion matrices נשמרו ל-confusion_matrix_comparison.png")
        plt.show()
        
    def export_comparison_report(self, output_path="comparison_report.json"):
        """יצוא דוח השוואה מפורט"""
        sklearn_metrics = self.calculate_metrics(self.y, self.y_pred_sklearn, "sklearn")
        
        report = {
            'dataset': self.csv_path,
            'n_samples': len(self.df),
            'n_features': len(self.X.columns),
            'n_classes': len(self.y.unique()),
            'sklearn_metrics': sklearn_metrics,
            'tree_depth': int(self.model.get_depth()),
            'n_leaves': int(self.model.get_n_leaves())
        }
        
        if self.y_pred_c is not None:
            c_metrics = self.calculate_metrics(self.y, self.y_pred_c, "C Algorithm")
            report['c_metrics'] = c_metrics
            
            # השוואה
            matches = (self.y_pred_sklearn == self.y_pred_c).sum()
            report['agreement_rate'] = float(matches / len(self.y_pred_sklearn))
            report['accuracy_difference'] = float(abs(
                sklearn_metrics['accuracy'] - c_metrics['accuracy']
            ))
        
        with open(output_path, 'w', encoding='utf-8') as f:
            json.dump(report, f, indent=2, ensure_ascii=False)
            
        print(f"\n✅ דוח השוואה נשמר ל-{output_path}")
        
    def run_full_comparison(self):
        """Full run of the entire comparison process"""
        print("🚀 Starting full comparison process...\n")
        
        # 1. Load data
        self.load_data()
        
        # 2. אימון sklearn
        self.train_sklearn_tree()
        
        # 3. חישוב מדדים ל-sklearn
        self.calculate_metrics(self.y, self.y_pred_sklearn, "sklearn Decision Tree")
        
        # 4. טעינת תחזיות C
        if self.load_c_predictions():
            # 5. חישוב מדדים ל-C
            self.calculate_metrics(self.y, self.y_pred_c, "C Algorithm")
            
            # 6. השוואה בין המודלים
            self.compare_predictions()
            
            # 7. ציור confusion matrices
            self.plot_confusion_matrices()
        
        # 8. ויזואליזציה של עץ sklearn
        self.visualize_sklearn_tree()
        
        # 9. יצוא דוח
        self.export_comparison_report()
        
        print("\n" + "="*60)
        print("✨ The comparison process completed successfully!")
        print("="*60)


def main():
    """Main function to run the comparison"""
    
    # Paths
    csv_path = "data\\iris.csv"
    predictions_path = "predictionsPYTHONLM.csv"
    
    # יצירת אובייקט ההשוואה
    comparator = DecisionTreeComparison(csv_path, predictions_path)
    
    # הרצה מלאה
    comparator.run_full_comparison()
    
    # Additional statistics
    print("\n📊 Additional statistics:") # דיווח נוסף
    print(f"  • sklearn tree depth: {comparator.model.get_depth()}") # עומק העץ
    print(f"  • Number of leaves: {comparator.model.get_n_leaves()}") # מספר עלים
    print(f"  • Feature importances:") # חשיבות תכונות
    for feat, imp in zip(comparator.X.columns, comparator.model.feature_importances_): # חשיבות תכונה
        if imp > 0.01:  # רק features משמעותיות
            print(f"    - {feat}: {imp:.4f}")


if __name__ == "__main__":
    main()