#ifndef HARMONIC_ANALYZER_H
#define HARMONIC_ANALYZER_H

#include <QVector>
#include <QPair>

// 谐波分量
struct HarmonicComponent {
    int order;       // 谐波次数 (1=基波)
    double magnitude; // 幅值 (相对于基波的百分比)
    double angle;     // 相位角 (°)
};

// 谐波分析结果
struct HarmonicResult {
    double thd_v;           // 电压总谐波畸变率 (%)
    double thd_i;           // 电流总谐波畸变率 (%)
    QVector<HarmonicComponent> vHarmonics;  // 电压各次谐波
    QVector<HarmonicComponent> iHarmonics;  // 电流各次谐波
    QString quality;        // 电能质量评级: 优/良/合格/不合格
};

class HarmonicAnalyzer
{
public:
    HarmonicAnalyzer() = default;

    // 核心: 生成典型电网谐波频谱 (基于幅值+负载类型)
    // loadType: 0=线性, 1=整流, 2=变频, 3=电弧
    HarmonicResult analyze(double Vrms, double Irms, int loadType, double baseFreq = 50.0);

    // 快速傅里叶分析 (简化模拟)
    static QVector<double> simulateFFT(const QVector<double> &samples, int sampleRate);

    // 根据 GB/T 14549 标准评估电能质量
    static QString evaluateQuality(double thd_v, double thd_i, double voltageLevel = 0.38);

private:
    // 生成典型负载谐波频谱
    void generateSpectrum(QVector<HarmonicComponent> &spectrum, int loadType);
};

#endif // HARMONIC_ANALYZER_H
