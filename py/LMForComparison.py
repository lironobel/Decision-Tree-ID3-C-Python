import pandas as pd
import numpy as np
from sklearn.tree import DecisionTreeClassifier, export_graphviz
from sklearn.metrics import (
    accuracy_score,
    precision_score,
    recall_score,
    f1_score,
    confusion_matrix,
    classification_report
)
import matplotlib.pyplot as plt
import seaborn as sns
from pathlib import Path
import subprocess
import os
import json


class DecisionTreeComparison:
    """
    Class to compare a Decision Tree implemented in C with sklearn's Decision Tree.
    """

    def __init__(self, csv_path, predictions_path="predictions.csv"):
        """
        Args:
            csv_path: path to CSV dataset
            predictions_path: path to C algorithm predictions CSV
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
        """Load dataset from CSV"""
        print("📂 Loading dataset...")
        self.df = pd.read_csv(self.csv_path)
        self.X = self.df.iloc[:, :-1]
        self.y = self.df.iloc[:, -1]
        print(f"✅ Loaded {len(self.df)} rows with {len(self.X.columns)} features")
        print(f"📊 Classes: {self.y.unique()}")
        print(f"📈 Class distribution:\n{self.y.value_counts()}\n")

    def train_sklearn_tree(self, max_depth=15, min_samples_split=10, criterion='entropy', random_state=42):
        """Train a Decision Tree using sklearn"""
        print("🌲 Training sklearn Decision Tree...")
        self.model = DecisionTreeClassifier(
            max_depth=max_depth,
            min_samples_split=min_samples_split,
            criterion=criterion,
            random_state=random_state
        )
        self.model.fit(self.X, self.y)
        self.y_pred_sklearn = self.model.predict(self.X)
        print(f"✅ Tree built successfully!")
        print(f"📏 Tree depth: {self.model.get_depth()}")
        print(f"🍃 Number of leaves: {self.model.get_n_leaves()}\n")

    def load_c_predictions(self):
        """Load predictions from C algorithm CSV"""
        if not self.predictions_path or not Path(self.predictions_path).exists():
            print("⚠️ Predictions file not found")
            return False

        print("📂 Loading C algorithm predictions...")
        pred_df = pd.read_csv(self.predictions_path, skiprows=1)
        self.y_pred_c = pred_df['predicted_label'].values
        print(f"✅ Loaded {len(self.y_pred_c)} predictions\n")
        return True

    def calculate_metrics(self, y_true, y_pred, model_name):
        """Calculate performance metrics"""
        print(f"\n{'='*60}")
        print(f"📊 Performance metrics for: {model_name}")
        print(f"{'='*60}")
        acc = accuracy_score(y_true, y_pred)
        precision = precision_score(y_true, y_pred, average='weighted', zero_division=0)
        recall = recall_score(y_true, y_pred, average='weighted', zero_division=0)
        f1 = f1_score(y_true, y_pred, average='weighted', zero_division=0)
        print(f"🎯 Accuracy: {acc:.4f} ({acc*100:.2f}%)")
        print(f"🔍 Precision: {precision:.4f}")
        print(f"📈 Recall: {recall:.4f}")
        print(f"⚖️  F1-Score: {f1:.4f}")
        print("\n📋 Confusion Matrix:")
        print(confusion_matrix(y_true, y_pred))
        print("\n📝 Classification Report:")
        print(classification_report(y_true, y_pred, zero_division=0))

        return {
            'accuracy': acc,
            'precision': precision,
            'recall': recall,
            'f1_score': f1,
            'confusion_matrix': confusion_matrix(y_true, y_pred).tolist()
        }

    def compare_predictions(self):
        """Compare sklearn predictions with C algorithm predictions"""
        if self.y_pred_c is None:
            print("⚠️ No C predictions loaded")
            return

        print(f"\n{'='*60}")
        print("🔄 Comparing models")
        print(f"{'='*60}")
        matches = (self.y_pred_sklearn == self.y_pred_c).sum()
        total = len(self.y_pred_sklearn)
        agreement = matches / total
        print(f"✅ Same predictions: {matches}/{total} ({agreement*100:.2f}%)")
        print(f"❌ Different predictions: {total-matches}/{total} ({(1-agreement)*100:.2f}%)")

        acc_sklearn = accuracy_score(self.y, self.y_pred_sklearn)
        acc_c = accuracy_score(self.y, self.y_pred_c)
        print(f"\n🎯 Accuracy sklearn: {acc_sklearn:.4f} ({acc_sklearn*100:.2f}%)")
        print(f"🎯 Accuracy C Algorithm: {acc_c:.4f} ({acc_c*100:.2f}%)")
        print(f"📊 Difference: {abs(acc_sklearn - acc_c):.4f} ({abs(acc_sklearn - acc_c)*100:.2f}%)")

        disagreements = np.where(self.y_pred_sklearn != self.y_pred_c)[0]
        if len(disagreements) > 0:
            print(f"\n🔍 Different predictions examples (first 5):")
            for idx in disagreements[:5]:
                print(f"  Row {idx}: sklearn={self.y_pred_sklearn[idx]}, "
                      f"C={self.y_pred_c[idx]}, true={self.y.iloc[idx]}")

    def generate_png_from_dot(self, dot_path="sklearn_tree.dot", output_path="sklearn_tree.png"):
        """Generate PNG from DOT file and open it"""
        if not os.path.exists(dot_path):
            print(f"⚠️ DOT file {dot_path} does not exist")
            return
        command = f"dot -Tpng {dot_path} -o {output_path}"
        try:
            subprocess.run(command, shell=True, check=True)
            print(f"✅ PNG created successfully: {output_path}")
        except subprocess.CalledProcessError as e:
            print(f"[ERROR] dot command failed with code: {e.returncode}")
            return
        if os.path.exists(output_path):
            try:
                os.startfile(output_path)
            except AttributeError:
                subprocess.run(["open" if os.name == "posix" else "xdg-open", output_path])

    def visualize_sklearn_tree(self, feature_names=None, output_path="sklearn_tree.dot"):
        """Export sklearn tree to DOT and generate PNG"""
        if self.model is None:
            print("⚠️ Model not trained yet")
            return
        if feature_names is None:
            feature_names = self.X.columns.tolist()
        export_graphviz(
            self.model,
            out_file=output_path,
            feature_names=feature_names,
            class_names=[str(c) for c in self.model.classes_],
            filled=True,
            rounded=True,
            special_characters=True
        )
        print(f"💾 DOT file saved to {output_path}")
        self.generate_png_from_dot(output_path=output_path.replace(".dot", ".png"))

    def plot_confusion_matrices(self):
        """Plot confusion matrices for sklearn and C algorithm"""
        fig, axes = plt.subplots(1, 2, figsize=(14, 5))
        sns.heatmap(confusion_matrix(self.y, self.y_pred_sklearn), annot=True, fmt='d', cmap='Blues',
                    ax=axes[0], cbar=False)
        axes[0].set_title('sklearn Decision Tree')
        axes[0].set_xlabel('Predicted')
        axes[0].set_ylabel('Actual')

        if self.y_pred_c is not None:
            sns.heatmap(confusion_matrix(self.y, self.y_pred_c), annot=True, fmt='d', cmap='Greens',
                        ax=axes[1], cbar=False)
            axes[1].set_title('C Algorithm Decision Tree')
            axes[1].set_xlabel('Predicted')
            axes[1].set_ylabel('Actual')

        plt.tight_layout()
        plt.savefig('confusion_matrix_comparison.png', dpi=300, bbox_inches='tight')
        print("✅ Confusion matrices saved to confusion_matrix_comparison.png")
        plt.show()

    def export_comparison_report(self, output_path="comparison_report.json"):
        """Export a detailed comparison report"""
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
            matches = (self.y_pred_sklearn == self.y_pred_c).sum()
            report['agreement_rate'] = float(matches / len(self.y_pred_sklearn))
            report['accuracy_difference'] = float(abs(
                sklearn_metrics['accuracy'] - c_metrics['accuracy']
            ))

        with open(output_path, 'w', encoding='utf-8') as f:
            json.dump(report, f, indent=2, ensure_ascii=False)
        print(f"\n✅ Comparison report saved to {output_path}")

    def run_full_comparison(self):
        """Run the full comparison process"""
        print("🚀 Starting full comparison process...\n")
        self.load_data()
        self.train_sklearn_tree()
        self.calculate_metrics(self.y, self.y_pred_sklearn, "sklearn Decision Tree")
        if self.load_c_predictions():
            self.calculate_metrics(self.y, self.y_pred_c, "C Algorithm")
            self.compare_predictions()
            self.plot_confusion_matrices()
        self.visualize_sklearn_tree()
        self.export_comparison_report()
        print("\n" + "="*60)
        print("✨ The comparison process completed successfully!")
        print("="*60)


def main():
    """Main function to run comparison"""
    csv_path = "data\\iris.csv"
    predictions_path = "predictionsPYTHONLM.csv"
    comparator = DecisionTreeComparison(csv_path, predictions_path)
    comparator.run_full_comparison() 
    print("\n📊 Additional statistics:") # מידע סטטיסטי נוסף
    print(f"  • sklearn tree depth: {comparator.model.get_depth()}") # עומק העץ
    print(f"  • Number of leaves: {comparator.model.get_n_leaves()}") # מספר העלים
    for feat, imp in zip(comparator.X.columns, comparator.model.feature_importances_):
        if imp > 0.01:
            print(f"    - {feat}: {imp:.4f}")
    
if __name__ == "__main__":
    main()
