#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QTextStream>
#include <QSplitter>
#include <QSplitter>
#include <cmath>
#include <QtMath>
#include <QtCharts/QChart>
#include <QtCharts/QBarCategoryAxis>

// ========== 构造函数 & 界面搭建 ==========
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
    setupChart();
    setupHarmonicChart();
    setupStatusBar();
}

void MainWindow::setupUI()
{
    auto *central = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(central);

    // 主分割器: 上(输入+结果) / 中(波形图) / 下(谐波图)
    auto *splitter = new QSplitter(Qt::Vertical, this);

    // ---- 上半部分: 输入 + 结果 ----
    auto *topWidget = new QWidget;
    auto *topLayout = new QHBoxLayout(topWidget);

    // 输入区
    auto *inputGroup = new QGroupBox("三相输入参数", this);
    auto *inputLayout = new QFormLayout(inputGroup);

    spinVa = new QDoubleSpinBox; spinVa->setRange(0, 50000); spinVa->setValue(220);
    spinVa->setSuffix(" V");  spinVa->setDecimals(1);
    spinVb = new QDoubleSpinBox; spinVb->setRange(0, 50000); spinVb->setValue(220);
    spinVb->setSuffix(" V");  spinVb->setDecimals(1);
    spinVc = new QDoubleSpinBox; spinVc->setRange(0, 50000); spinVc->setValue(220);
    spinVc->setSuffix(" V");  spinVc->setDecimals(1);

    spinIa = new QDoubleSpinBox; spinIa->setRange(0, 5000); spinIa->setValue(10.0);
    spinIa->setSuffix(" A");  spinIa->setDecimals(1);
    spinIb = new QDoubleSpinBox; spinIb->setRange(0, 5000); spinIb->setValue(10.0);
    spinIb->setSuffix(" A");  spinIb->setDecimals(1);
    spinIc = new QDoubleSpinBox; spinIc->setRange(0, 5000); spinIc->setValue(10.0);
    spinIc->setSuffix(" A");  spinIc->setDecimals(1);

    spinPF = new QDoubleSpinBox; spinPF->setRange(0.1, 1.0); spinPF->setValue(0.85);
    spinPF->setSingleStep(0.01); spinPF->setDecimals(2);
    spinFreq = new QDoubleSpinBox; spinFreq->setRange(45, 65); spinFreq->setValue(50.0);
    spinFreq->setSuffix(" Hz"); spinFreq->setDecimals(1);

    // 负载类型选择
    comboLoadType = new QComboBox;
    comboLoadType->addItems({"线性负载", "整流负载(6脉波)", "变频驱动(PWM)", "电弧负载"});

    inputLayout->addRow("A相电压 Ua:", spinVa);
    inputLayout->addRow("B相电压 Ub:", spinVb);
    inputLayout->addRow("C相电压 Uc:", spinVc);
    inputLayout->addRow("A相电流 Ia:", spinIa);
    inputLayout->addRow("B相电流 Ib:", spinIb);
    inputLayout->addRow("C相电流 Ic:", spinIc);
    inputLayout->addRow("功率因数 PF:", spinPF);
    inputLayout->addRow("电网频率:", spinFreq);
    inputLayout->addRow("负载类型:", comboLoadType);

    // 计算结果区
    auto *resultGroup = new QGroupBox("计算结果", this);
    auto *resultLayout = new QFormLayout(resultGroup);

    labelVavg = new QLabel("0.00 V");
    labelIavg = new QLabel("0.00 A");
    labelVunb = new QLabel("0.00 %");
    labelIunb = new QLabel("0.00 %");
    labelS = new QLabel("0.00 VA");
    labelP = new QLabel("0.00 W");
    labelQ = new QLabel("0.00 var");
    labelTHDv = new QLabel("0.00 %");
    labelTHDi = new QLabel("0.00 %");
    labelQuality = new QLabel("——");
    labelQuality->setStyleSheet("font-weight:bold; color:#1a73e8;");

    resultLayout->addRow("平均电压:", labelVavg);
    resultLayout->addRow("平均电流:", labelIavg);
    resultLayout->addRow("电压不平衡度:", labelVunb);
    resultLayout->addRow("电流不平衡度:", labelIunb);
    resultLayout->addRow("视在功率 S:", labelS);
    resultLayout->addRow("有功功率 P:", labelP);
    resultLayout->addRow("无功功率 Q:", labelQ);
    resultLayout->addRow("电压THD:", labelTHDv);
    resultLayout->addRow("电流THD:", labelTHDi);
    resultLayout->addRow("电能质量:", labelQuality);

    topLayout->addWidget(inputGroup, 3);
    topLayout->addWidget(resultGroup, 2);

    // ---- 中间: 波形图 ----
    chartView = new QChartView;
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumHeight(200);

    // ---- 下半部分: 谐波柱状图 ----
    harmonicChartView = new QChartView;
    harmonicChartView->setRenderHint(QPainter::Antialiasing);
    harmonicChartView->setMinimumHeight(200);

    splitter->addWidget(topWidget);
    splitter->addWidget(chartView);
    splitter->addWidget(harmonicChartView);

    // ---- 按钮 ----
    auto *btnLayout = new QHBoxLayout;
    btnCalc   = new QPushButton("计算");
    btnClear  = new QPushButton("清空");
    btnExport = new QPushButton("导出 CSV");
    btnCalc->setStyleSheet("QPushButton{font-weight:bold;}");
    btnLayout->addWidget(btnCalc);
    btnLayout->addWidget(btnClear);
    btnLayout->addWidget(btnExport);
    btnLayout->addStretch();

    // ---- 日志 ----
    textLog = new QTextEdit;
    textLog->setReadOnly(true);
    textLog->setMaximumHeight(80);

    // ---- 历史记录表 ----
    tableHistory = new QTableWidget(0, 12);
    tableHistory->setHorizontalHeaderLabels(
        {"序号","Uavg","Iavg","Vunb%","Iunb%","S(VA)","P(W)","Q(var)","PF","THDv%","THDi%","时间"});
    tableHistory->horizontalHeader()->setStretchLastSection(true);
    tableHistory->setMaximumHeight(120);

    // ---- 布局组装 ----
    mainLayout->addWidget(splitter);
    mainLayout->addLayout(btnLayout);
    mainLayout->addWidget(textLog);
    mainLayout->addWidget(tableHistory);

    setCentralWidget(central);

    // 信号连接
    connect(btnCalc,  &QPushButton::clicked, this, &MainWindow::onCalculate);
    connect(btnClear, &QPushButton::clicked, this, &MainWindow::onClear);
    connect(btnExport,&QPushButton::clicked, this, &MainWindow::onExport);
}

void MainWindow::setupStatusBar()
{
    statusBar()->showMessage("就绪 — 版本 1.2 (含THD谐波分析与GB/T 14549评估)");
}

// ========== 电压/电流波形图 ==========
void MainWindow::setupChart()
{
    auto *chart = new QChart;
    chart->setTitle("三相电压/电流实时波形");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    seriesVa = new QLineSeries; seriesVa->setName("Ua");
    seriesVb = new QLineSeries; seriesVb->setName("Ub");
    seriesVc = new QLineSeries; seriesVc->setName("Uc");
    seriesIa = new QLineSeries; seriesIa->setName("Ia");
    seriesIb = new QLineSeries; seriesIb->setName("Ib");
    seriesIc = new QLineSeries; seriesIc->setName("Ic");
    QPen dashPen; dashPen.setStyle(Qt::DashLine); dashPen.setWidth(2);
    seriesIa->setPen(dashPen); seriesIb->setPen(dashPen); seriesIc->setPen(dashPen);

    chart->addSeries(seriesVa); chart->addSeries(seriesVb); chart->addSeries(seriesVc);
    chart->addSeries(seriesIa); chart->addSeries(seriesIb); chart->addSeries(seriesIc);

    axisX = new QValueAxis; axisX->setTitleText("角度 (°)"); axisX->setRange(0, 360);
    axisY_V = new QValueAxis; axisY_V->setTitleText("电压 (V)");
    axisY_I = new QValueAxis; axisY_I->setTitleText("电流 (A)");

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY_V, Qt::AlignLeft);
    chart->addAxis(axisY_I, Qt::AlignRight);

    seriesVa->attachAxis(axisX); seriesVb->attachAxis(axisX); seriesVc->attachAxis(axisX);
    seriesIa->attachAxis(axisX); seriesIb->attachAxis(axisX); seriesIc->attachAxis(axisX);
    seriesVa->attachAxis(axisY_V); seriesVb->attachAxis(axisY_V); seriesVc->attachAxis(axisY_V);
    seriesIa->attachAxis(axisY_I); seriesIb->attachAxis(axisY_I); seriesIc->attachAxis(axisY_I);

    chartView->setChart(chart);
}

void MainWindow::updateChart(const ThreePhaseParams &p)
{
    seriesVa->clear(); seriesVb->clear(); seriesVc->clear();
    seriesIa->clear(); seriesIb->clear(); seriesIc->clear();

    double w = 2.0 * M_PI * p.freq;
    double Vm_a = p.Va * std::sqrt(2.0);
    double Vm_b = p.Vb * std::sqrt(2.0);
    double Vm_c = p.Vc * std::sqrt(2.0);
    double Im_a = p.Ia * std::sqrt(2.0);
    double Im_b = p.Ib * std::sqrt(2.0);
    double Im_c = p.Ic * std::sqrt(2.0);

    for (int deg = 0; deg <= 360; deg += 2) {
        double rad = deg * M_PI / 180.0;
        seriesVa->append(deg, Vm_a * std::sin(rad));
        seriesVb->append(deg, Vm_b * std::sin(rad - 2.0 * M_PI / 3.0));
        seriesVc->append(deg, Vm_c * std::sin(rad + 2.0 * M_PI / 3.0));
        seriesIa->append(deg, Im_a * std::sin(rad));
        seriesIb->append(deg, Im_b * std::sin(rad - 2.0 * M_PI / 3.0));
        seriesIc->append(deg, Im_c * std::sin(rad + 2.0 * M_PI / 3.0));
    }

    double maxV = qMax({Vm_a, Vm_b, Vm_c}) * 1.2;
    double maxI = qMax({Im_a, Im_b, Im_c}) * 1.2;
    axisY_V->setRange(-maxV, maxV);
    axisY_I->setRange(-maxI, maxI);
}

// ========== 谐波柱状图 ==========
void MainWindow::setupHarmonicChart()
{
    auto *chart = new QChart;
    chart->setTitle("谐波频谱分析 (各次谐波含量)");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    harmonicBarSeries = new QBarSeries;
    chart->addSeries(harmonicBarSeries);

    auto *axisX_har = new QBarCategoryAxis;
    axisX_har->setTitleText("谐波次数");
    chart->addAxis(axisX_har, Qt::AlignBottom);

    auto *axisY_har = new QValueAxis;
    axisY_har->setTitleText("含量 (%)");
    axisY_har->setRange(0, 30);
    chart->addAxis(axisY_har, Qt::AlignLeft);

    harmonicBarSeries->attachAxis(axisX_har);
    harmonicBarSeries->attachAxis(axisY_har);

    harmonicChartView->setChart(chart);
}

void MainWindow::updateHarmonicChart(const HarmonicResult &hr)
{
    harmonicBarSeries->clear();

    auto *vSet = new QBarSet("电压谐波");
    auto *iSet = new QBarSet("电流谐波");
    vSet->setColor(QColor("#4285F4"));
    iSet->setColor(QColor("#EA4335"));

    QStringList categories;

    for (const auto &h : hr.vHarmonics) {
        if (h.order > 1) {
            categories << QString("%1次").arg(h.order);
            *vSet << (h.magnitude / (hr.vHarmonics[0].magnitude > 0 ? hr.vHarmonics[0].magnitude : 1.0)) * 100.0;
        }
    }
    for (const auto &h : hr.iHarmonics) {
        if (h.order > 1) {
            *iSet << (h.magnitude / (hr.iHarmonics[0].magnitude > 0 ? hr.iHarmonics[0].magnitude : 1.0)) * 100.0;
        }
    }

    harmonicBarSeries->append(vSet);
    harmonicBarSeries->append(iSet);

    // 更新类别轴
    auto *chart = harmonicChartView->chart();
    auto axes = chart->axes(Qt::Horizontal);
    if (!axes.isEmpty()) {
        auto *axis = qobject_cast<QBarCategoryAxis*>(axes.first());
        if (axis) axis->clear();
        if (axis) axis->append(categories);
    }
}

// ========== 自动推算功率因数 ==========
double MainWindow::autoCalcPF(const ThreePhaseParams &p)
{
    double vUnb = calcUnbalance(p.Va, p.Vb, p.Vc);
    double iUnb = calcUnbalance(p.Ia, p.Ib, p.Ic);
    double pf = 0.95 - ((vUnb + iUnb) / 20.0) * 0.1;
    return qBound(0.5, pf, 1.0);
}

double MainWindow::calcUnbalance(double a, double b, double c)
{
    double avg = (a + b + c) / 3.0;
    if (avg == 0.0) return 0.0;
    double maxDev = qMax(qAbs(a - avg), qMax(qAbs(b - avg), qAbs(c - avg)));
    return (maxDev / avg) * 100.0;
}

PowerResults MainWindow::calcThreePhasePower(const ThreePhaseParams &p)
{
    PowerResults r;
    r.Vavg = (p.Va + p.Vb + p.Vc) / 3.0;
    r.Iavg = (p.Ia + p.Ib + p.Ic) / 3.0;
    r.Vunbalance = calcUnbalance(p.Va, p.Vb, p.Vc);
    r.Iunbalance = calcUnbalance(p.Ia, p.Ib, p.Ic);
    r.S = std::sqrt(3.0) * r.Vavg * r.Iavg;
    r.P = r.S * p.pf;
    r.Q = r.S * std::sqrt(1.0 - p.pf * p.pf);
    return r;
}

void MainWindow::onCalculate()
{
    ThreePhaseParams p;
    p.Va   = spinVa->value();
    p.Vb   = spinVb->value();
    p.Vc   = spinVc->value();
    p.Ia   = spinIa->value();
    p.Ib   = spinIb->value();
    p.Ic   = spinIc->value();
    p.pf   = spinPF->value();
    p.freq = spinFreq->value();

    double autoPF = autoCalcPF(p);
    textLog->append(QString("[推算] 自动推算功率因数: %1").arg(autoPF, 0, 'f', 2));

    PowerResults r = calcThreePhasePower(p);

    // 更新基本显示
    labelVavg->setText(QString::number(r.Vavg, 'f', 2) + " V");
    labelIavg->setText(QString::number(r.Iavg, 'f', 2) + " A");
    labelVunb->setText(QString::number(r.Vunbalance, 'f', 2) + " %");
    labelIunb->setText(QString::number(r.Iunbalance, 'f', 2) + " %");
    labelS->setText(QString::number(r.S, 'f', 2) + " VA");
    labelP->setText(QString::number(r.P, 'f', 2) + " W");
    labelQ->setText(QString::number(r.Q, 'f', 2) + " var");

    // 谐波分析
    int loadType = comboLoadType->currentIndex();
    HarmonicResult hr = analyzer.analyze(r.Vavg, r.Iavg, loadType, p.freq);
    labelTHDv->setText(QString::number(hr.thd_v, 'f', 2) + " %");
    labelTHDi->setText(QString::number(hr.thd_i, 'f', 2) + " %");
    labelQuality->setText(hr.quality);

    // 质量颜色
    if (hr.quality.startsWith("优"))
        labelQuality->setStyleSheet("font-weight:bold; color:#0F9D58;");
    else if (hr.quality.startsWith("良"))
        labelQuality->setStyleSheet("font-weight:bold; color:#4285F4;");
    else if (hr.quality.startsWith("合格"))
        labelQuality->setStyleSheet("font-weight:bold; color:#F4B400;");
    else
        labelQuality->setStyleSheet("font-weight:bold; color:#EA4335;");

    // 更新两个图
    updateChart(p);
    updateHarmonicChart(hr);

    // 日志
    QString log = QString("[%1] Uavg=%2V Iavg=%3A S=%4VA P=%5W THDv=%6% THDi=%7% [%8]")
        .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
        .arg(r.Vavg, 0, 'f', 1).arg(r.Iavg, 0, 'f', 1)
        .arg(r.S, 0, 'f', 1).arg(r.P, 0, 'f', 1)
        .arg(hr.thd_v, 0, 'f', 1).arg(hr.thd_i, 0, 'f', 1)
        .arg(hr.quality);
    textLog->append(log);

    // 历史记录
    int row = tableHistory->rowCount();
    tableHistory->insertRow(row);
    tableHistory->setItem(row, 0, new QTableWidgetItem(QString::number(++recordCount)));
    tableHistory->setItem(row, 1, new QTableWidgetItem(QString::number(r.Vavg, 'f', 1)));
    tableHistory->setItem(row, 2, new QTableWidgetItem(QString::number(r.Iavg, 'f', 1)));
    tableHistory->setItem(row, 3, new QTableWidgetItem(QString::number(r.Vunbalance, 'f', 2)));
    tableHistory->setItem(row, 4, new QTableWidgetItem(QString::number(r.Iunbalance, 'f', 2)));
    tableHistory->setItem(row, 5, new QTableWidgetItem(QString::number(r.S, 'f', 1)));
    tableHistory->setItem(row, 6, new QTableWidgetItem(QString::number(r.P, 'f', 1)));
    tableHistory->setItem(row, 7, new QTableWidgetItem(QString::number(r.Q, 'f', 1)));
    tableHistory->setItem(row, 8, new QTableWidgetItem(QString::number(p.pf, 'f', 2)));
    tableHistory->setItem(row, 9, new QTableWidgetItem(QString::number(hr.thd_v, 'f', 1)));
    tableHistory->setItem(row, 10, new QTableWidgetItem(QString::number(hr.thd_i, 'f', 1)));
    tableHistory->setItem(row, 11, new QTableWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")));

    statusBar()->showMessage(QString("计算完成 — 第 %1 条记录 | %2").arg(recordCount).arg(hr.quality));
}

void MainWindow::onClear()
{
    labelVavg->setText("0.00 V");
    labelIavg->setText("0.00 A");
    labelVunb->setText("0.00 %");
    labelIunb->setText("0.00 %");
    labelS->setText("0.00 VA");
    labelP->setText("0.00 W");
    labelQ->setText("0.00 var");
    labelTHDv->setText("0.00 %");
    labelTHDi->setText("0.00 %");
    labelQuality->setText("——");
    labelQuality->setStyleSheet("font-weight:bold; color:#1a73e8;");
    textLog->clear();
    tableHistory->setRowCount(0);
    recordCount = 0;
    seriesVa->clear(); seriesVb->clear(); seriesVc->clear();
    seriesIa->clear(); seriesIb->clear(); seriesIc->clear();
    harmonicBarSeries->clear();
    statusBar()->showMessage("已清空所有记录");
}

void MainWindow::onExport()
{
    if (tableHistory->rowCount() == 0) {
        QMessageBox::information(this, "提示", "没有数据可导出！");
        return;
    }
    QString fileName = QFileDialog::getSaveFileName(
        this, "导出 CSV", "power_data.csv", "CSV 文件 (*.csv)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误", "无法创建文件！");
        return;
    }
    QTextStream out(&file);
    out << "\xEF\xBB\xBF";
    out << "序号,Uavg(V),Iavg(A),Vunb(%),Iunb(%),S(VA),P(W),Q(var),PF,THDv(%),THDi(%),时间\n";
    for (int i = 0; i < tableHistory->rowCount(); ++i) {
        for (int j = 0; j < tableHistory->columnCount(); ++j) {
            if (j > 0) out << ",";
            auto *item = tableHistory->item(i, j);
            out << (item ? item->text() : "");
        }
        out << "\n";
    }
    file.close();
    statusBar()->showMessage("导出成功: " + fileName);
    textLog->append("[导出] " + fileName);
}
