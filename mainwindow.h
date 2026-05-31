#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QTextEdit>

// 三相电气参数结构体
struct ThreePhaseParams {
    double Va, Vb, Vc;   // 三相电压 (V)
    double Ia, Ib, Ic;   // 三相电流 (A)
    double pf;           // 功率因数
    double freq;         // 电网频率 (Hz)
};

// 计算结果结构体
struct PowerResults {
    double Vavg, Iavg;           // 平均电压/电流
    double Vunbalance, Iunbalance; // 三相不平衡度 (%)
    double S, P, Q;              // 视在/有功/无功功率
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() = default;

private slots:
    void onCalculate();    // 计算按钮
    void onClear();        // 清空按钮
    void onExport();       // 导出按钮

private:
    void setupUI();
    void setupStatusBar();
    PowerResults calcThreePhasePower(const ThreePhaseParams &p);
    double calcUnbalance(double a, double b, double c);

    // 输入控件
    QDoubleSpinBox *spinVa, *spinVb, *spinVc;
    QDoubleSpinBox *spinIa, *spinIb, *spinIc;
    QDoubleSpinBox *spinPF, *spinFreq;

    // 显示控件
    QLabel *labelVavg, *labelIavg;
    QLabel *labelVunb, *labelIunb;
    QLabel *labelS, *labelP, *labelQ;
    QTextEdit *textLog;
    QTableWidget *tableHistory;
    QPushButton *btnCalc, *btnClear, *btnExport;

    int recordCount = 0;
};

#endif // MAINWINDOW_H
