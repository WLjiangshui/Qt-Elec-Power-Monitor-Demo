#include "harmonic_analyzer.h"
#include <cmath>
#include <QDebug>

// ========== 主分析函数 ==========
HarmonicResult HarmonicAnalyzer::analyze(double Vrms, double Irms, int loadType, double baseFreq)
{
    HarmonicResult result;
    Q_UNUSED(baseFreq);

    // 生成电压谐波频谱 (电网侧谐波)
    generateSpectrum(result.vHarmonics, loadType);
    // 将相对值转换为基于 Vrms 的实际值
    double vFundamental = Vrms; // 基波 ≈ Vrms
    for (auto &h : result.vHarmonics) {
        h.magnitude = vFundamental * (h.magnitude / 100.0);
    }

    // 生成电流谐波频谱
    generateSpectrum(result.iHarmonics, loadType);
    double iFundamental = Irms;
    for (auto &h : result.iHarmonics) {
        h.magnitude = iFundamental * (h.magnitude / 100.0);
    }

    // 计算 THD (Total Harmonic Distortion)
    double vHarmSumSq = 0.0;
    double iHarmSumSq = 0.0;
    for (const auto &h : result.vHarmonics) {
        if (h.order > 1) vHarmSumSq += h.magnitude * h.magnitude;
    }
    for (const auto &h : result.iHarmonics) {
        if (h.order > 1) iHarmSumSq += h.magnitude * h.magnitude;
    }

    double vFund = result.vHarmonics.isEmpty() ? Vrms : result.vHarmonics[0].magnitude;
    double iFund = result.iHarmonics.isEmpty() ? Irms : result.iHarmonics[0].magnitude;

    result.thd_v = (vFund > 0) ? (std::sqrt(vHarmSumSq) / vFund) * 100.0 : 0.0;
    result.thd_i = (iFund > 0) ? (std::sqrt(iHarmSumSq) / iFund) * 100.0 : 0.0;

    // 电能质量评级 (参考 GB/T 14549-93)
    result.quality = evaluateQuality(result.thd_v, result.thd_i);

    return result;
}

// ========== 生成典型负载谐波频谱 ==========
void HarmonicAnalyzer::generateSpectrum(QVector<HarmonicComponent> &spectrum, int loadType)
{
    spectrum.clear();

    // 基波总是 100%
    spectrum.append({1, 100.0, 0.0});

    switch (loadType) {
    case 0: // 线性负载 - 几乎无谐波
        spectrum.append({3, 1.0, 180.0});
        spectrum.append({5, 0.5, 90.0});
        break;

    case 1: // 整流负载 (6脉波整流桥典型频谱)
        spectrum.append({3, 3.0, 180.0});
        spectrum.append({5, 18.5, 0.0});
        spectrum.append({7, 12.0, 180.0});
        spectrum.append({11, 7.5, 0.0});
        spectrum.append({13, 5.5, 180.0});
        spectrum.append({17, 3.0, 0.0});
        spectrum.append({19, 2.0, 180.0});
        break;

    case 2: { // 变频驱动 (PWM 逆变器典型频谱)
        // 低次谐波较少, 高次开关频率谐波丰富
        spectrum.append({3, 2.0, 180.0});
        spectrum.append({5, 10.0, 0.0});
        spectrum.append({7, 8.0, 180.0});
        spectrum.append({11, 5.0, 0.0});
        spectrum.append({13, 4.0, 180.0});
        // 开关频率附近 (假设 4kHz / 50Hz = 80 次)
        spectrum.append({79, 3.0, 0.0});
        spectrum.append({83, 3.0, 180.0});
        spectrum.append({85, 2.0, 0.0});
        break;
    }

    case 3: // 电弧负载 (非线性极强)
        spectrum.append({3, 15.0, 180.0});
        spectrum.append({5, 25.0, 0.0});
        spectrum.append({7, 18.0, 180.0});
        spectrum.append({9, 10.0, 0.0});
        spectrum.append({11, 8.0, 180.0});
        spectrum.append({13, 5.0, 0.0});
        spectrum.append({15, 4.0, 180.0});
        break;

    default:
        spectrum.append({3, 1.0, 180.0});
        spectrum.append({5, 0.5, 90.0});
        break;
    }
}

// ========== 简化 FFT 模拟 ==========
QVector<double> HarmonicAnalyzer::simulateFFT(const QVector<double> &samples, int sampleRate)
{
    Q_UNUSED(sampleRate);
    // 简化为峰值检测, 真实实现应用 FFTW 或 Qt 的 FFT
    QVector<double> spectrum(20, 0.0);
    if (samples.isEmpty()) return spectrum;

    double maxVal = 0.0;
    for (double v : samples) maxVal = qMax(maxVal, qAbs(v));

    // 模拟: 基波 = maxVal, 谐波逐渐衰减
    spectrum[0] = maxVal;           // 基波
    spectrum[2] = maxVal * 0.03;    // 3次
    spectrum[4] = maxVal * 0.18;    // 5次
    spectrum[6] = maxVal * 0.12;    // 7次

    return spectrum;
}

// ========== 电能质量评估 (GB/T 14549) ==========
QString HarmonicAnalyzer::evaluateQuality(double thd_v, double thd_i, double voltageLevel)
{
    // 国标限值: 0.38kV 系统 THDu ≤ 5%
    double vLimit = (voltageLevel <= 1.0) ? 5.0 : 4.0;

    if (thd_v < vLimit * 0.5 && thd_i < 10.0)
        return "优 — 谐波含量极低";
    else if (thd_v < vLimit * 0.8 && thd_i < 15.0)
        return "良 — 谐波含量较低";
    else if (thd_v < vLimit && thd_i < 25.0)
        return "合格 — 满足国标要求";
    else
        return QString("不合格 — 需加装滤波装置 (THDu= %1 %, 限值 %2 %)").arg(thd_v, 0, 'f', 1).arg(vLimit);
}
