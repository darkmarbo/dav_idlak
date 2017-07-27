#pragma once


#include <vector>
#include "common.h"

using namespace std;

#define NUMln10  2.3025850929940456840179914546843642076011
#define NUM2pi  6.2831853071795864769252867665590057683943
#define NUMpi  3.1415926535897932384626433832795028841972
#define NUMpi_2  1.5707963267948966192313216916397514420986
#define NUMpi_4  0.7853981633974483096156608458198757210493

typedef struct tag_SEGMENT_ {
	double begin;
	double end;
} SEGMENT;

typedef struct tag_INTENSITY_ {
	double stamp;
	double value;
} INTENSITY;

class AutoSegmenter
{
public:
	AutoSegmenter(int nSampleRate = 16000);
	~AutoSegmenter(void);

public:
	// 均值
	double mean(vector<double> vArr);
	// 方差
	double variance(vector<double> vArr);
	// 标准差
	double stddev(vector<double> vArr);
	// 合并音段
	vector<SEGMENT> combine_utts(vector<SEGMENT> vSeg, double fMaxPause = 0.35);
	// 添加预留
	vector<SEGMENT> pad_utts(vector<SEGMENT> vSeg, double fWaveLen, double fHeadPad = 0.3, double fEndPad = 0.3);
	// 根据音强计算音段
	vector<SEGMENT>  seg_utts_by_Intensity(vector<INTENSITY>  vIntensity, double fThresholdCoef = 0.33);
	// 计算音强
	vector<INTENSITY> calcIntensity(short* pData = NULL, int nSamples = 0, double fMinPitch = 75, double fTimeStep = 0.0);
	// 返回音段结果
	vector<SEGMENT> getSegment(short* pData = NULL, int nSamples = 0, double fMinPitch = 75, double fTimeStep = 0.0, double fMaxPause = 0.35, double fThresholdCoef = 0.33, double fBeginPad = 0.3, double fEndPad = 0.3);

public:
	// extract from Praat NUM.cpp
	double NUMbessel_i0_f (double x);

public:
	// 采样率
	int m_nSampleRate;
};
