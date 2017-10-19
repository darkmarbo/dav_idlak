#include "AutoSegmenter.h"
#include "math.h"

using namespace std;

AutoSegmenter::AutoSegmenter(int nSampleRate)
: m_nSampleRate(nSampleRate)
{
}

AutoSegmenter::~AutoSegmenter(void)
{
}

/*  平均值:
 * 计算 vector 内所有double 类型的 算数平均值 
 * */
double AutoSegmenter::mean(vector<double> vArr)
{
	double mean = 0.0;
	
	if (vArr.size() == 0) return mean;

	vector<double>::iterator pIter = vArr.begin();
	for (; pIter != vArr.end(); pIter++) {
		mean += *pIter;
	}

	return mean / vArr.size();
}

/*
 * 方差:
 * 计算 vector 内所有double 数据的方差  
 * */
double AutoSegmenter::variance(vector<double> vArr)
{
	double fmean = mean(vArr);
	double fvariance = 0;

	vector<double>::iterator pIter = vArr.begin();
	for (; pIter != vArr.end(); pIter++) 
    {
		fvariance += (*pIter - fmean) * (*pIter - fmean);
	}
	if (vArr.size() > 1) 
		fvariance /= vArr.size();

	return fvariance;
}

/*
 * 标准差=sqrt(方差)
 * */
double AutoSegmenter::stddev(vector<double> vArr)
{
	return sqrt(variance(vArr));
}


/*
 * 提取出独立的语音段: 距离下一个语音远(>fMaxPause) and 后一个大于200ms
 * fMaxPause 就是 m_fMinSilence
 * */
vector<SEGMENT> AutoSegmenter::combine_utts(vector<SEGMENT> vSeg, double fMaxPause)
{
	vector<SEGMENT> vUtts;
	if (vSeg.empty()) return vUtts;

	vector<SEGMENT>::iterator pIter = vSeg.begin();

	SEGMENT prev = *pIter;
	for (pIter++; pIter != vSeg.end(); pIter++)
	{
        // 当前 语音段 与 前一个语音段 之间的 静音长度 
		double fDiff = pIter->begin - prev.end;

        // 距离前一个语音够远  and  当前语音段长度够长 > 200ms 
        // 说明当前语音不需要与前面语音合并 
		if (fDiff >= fMaxPause && pIter->end - pIter->begin > 0.2) 
        {
			vUtts.push_back(prev);
			prev = *pIter;
		} else 
        {
            // 前一个prev 把当前语音段合并 
			prev.end = pIter->end;
		}
	}
	vUtts.push_back(prev);

	return vUtts;
}

/*
 * fHeadPad: 左侧向左移动的时间长度(秒)
 * fEndPad: 右侧向右移动
 * */
vector<SEGMENT> AutoSegmenter::pad_utts(vector<SEGMENT> vSeg, 
            double fWaveLen, double fHeadPad, double fEndPad)
{
	vector<SEGMENT> vUtts;
	vector<SEGMENT>::iterator pIter = vSeg.begin();
    // 上个片段的真实末尾
	double prevEnd = 0;

    if(vSeg.size() < 1)
    {
        printf("pad_utts:vSeg_size()<1\n");
        return vUtts;
    }

	vector<SEGMENT>::iterator pNext = pIter+1;
	for (; pNext != vSeg.end(); pIter++, pNext++)
	{
		SEGMENT curSeg;
		curSeg.begin = pIter->begin - fHeadPad;
		if (curSeg.begin < prevEnd)
			curSeg.begin = prevEnd;

        // ????
		prevEnd = pIter->end;

		curSeg.end = pIter->end + fEndPad;
		if (curSeg.end > pNext->begin) {
			curSeg.end = pNext->begin;
		}

		vUtts.push_back(curSeg);
	}

    // 最后一段 没有pNext 
	SEGMENT curSeg;
	curSeg.begin = pIter->begin - fHeadPad;
	if (curSeg.begin < prevEnd) 
		curSeg.begin = prevEnd;
	curSeg.end = pIter->end + fEndPad;
	if (curSeg.end > fWaveLen) {
		curSeg.end = fWaveLen;
	}
	vUtts.push_back(curSeg);

	return vUtts;
}

/* Modified Bessel function I0. Abramowicz & Stegun, p. 378.*/
double AutoSegmenter::NUMbessel_i0_f (double x) {
	double t;
	if (x < 0.0) return NUMbessel_i0_f (- x);
	if (x < 3.75) {
		/* Formula 9.8.1. Accuracy 1.6e-7. */
		t = x / 3.75;
		t *= t;
		return 1.0 + t * (3.5156229 + t * (3.0899424 + t * (1.2067492
			+ t * (0.2659732 + t * (0.0360768 + t * 0.0045813)))));
	}
	/*
	otherwise: x >= 3.75
	*/
	/* Formula 9.8.2. Accuracy of the polynomial factor 1.9e-7. */
	t = 3.75 / x;   /* <= 1.0 */
	return exp (x) / sqrt (x) * (0.39894228 + t * (0.01328592
		+ t * (0.00225319 + t * (-0.00157565 + t * (0.00916281
		+ t * (-0.02057706 + t * (0.02635537 + t * (-0.01647633
		+ t * 0.00392377))))))));
}


/*
 * calculate intensity vector
 * 所有窗内 计算能量10log10(平方平均)
 *
 */
vector<INTENSITY> AutoSegmenter::calcIntensity(short* pData, int nSamples, 
            double fMinPitch, double fTimeStep)
{
	if (pData == NULL || nSamples == 0)
		return vector<INTENSITY>();

	// 计算窗长与帧移  单位是秒
	double WinDur = 6.4 / fMinPitch;  // 6.4/75=0.853
	double halfWinDur = 0.5 * WinDur; // 0.427
	int halfWinSample = (int)(halfWinDur * m_nSampleRate);

	double ShiftDur = 0.8 / fMinPitch; // 0.8/75=0.01  == 10ms
	//if (fTimeStep != 0.0) 
	//	ShiftDur = fTimeStep / fMinPitch;
	int nShiftSample = (int)(ShiftDur * m_nSampleRate);

	// 计算均值   szm 没有取绝对值?
	double fmean = 0.0;
	for (int i = 0; i< nSamples; i++)
		fmean += pData[i];
	fmean /= nSamples;

	//// bessel窗函数   // 没有使用
	//vector<double> vWinCoef;
	//for (long i = - halfWinSample; i <= halfWinSample; i ++) {
	//	double x = i * 1.0 / (halfWinSample), root = 1. - x * x;
	//	double value = root <= 0.0 ? 0.0 : NUMbessel_i0_f ((2 * NUMpi * NUMpi + 0.5) * sqrt (root));
	//	vWinCoef.push_back(value);
	//}

	// 逐窗计算音强
	vector<INTENSITY> vPower;

    // 可以移动多少个 
	int nFrameNum = (int)( (nSamples - halfWinSample * 2 - 1) / nShiftSample ) - 3;

    // 窗 中心点 
	int mididx = 2 * halfWinSample + 1 - (int)(nShiftSample * 0.5);

    // 没用到...
	double fmin = 200, fmax = 0;
	for (int i=0; i<nFrameNum; i++)
	{
		mididx += nShiftSample;

        // 窗内平均能量 
		double rms = 0.;

		for (int j = -halfWinSample; j <= halfWinSample; j++)
		{
			double value = (pData[mididx + j] - fmean) / 32768.0;
			rms += value * value;
		}
		rms /= ( 2 * halfWinSample + 1 );

		//double sumw = 0., sumxw = 0.;
		//for (int j = -halfWinSample; j <= halfWinSample; j++)
		//{
		//	double value = (pData[mididx + j] - fmean) / 32768.0;
		//	sumxw += value * value * vWinCoef[j+halfWinSample];
		//	sumw += vWinCoef[j+halfWinSample];
		//}
		//rms = sumxw / sumw;

		INTENSITY power;
        // 中心点对应的时间(秒)
		power.stamp = mididx * 1.0 / m_nSampleRate;
		power.value = 10.0 * log10(rms / 4e-10);
        // 4e-10     = 0.00004539993*4=0.00018

		vPower.push_back(power);

		if (power.value > fmax) {
			fmax = power.value;
		}
		if (power.value < fmin) {
			fmin = power.value;
		}
	}

	return vPower;
}


/*
 *  使用每个窗内的平均能量 vIntensity
 *  1.检测到语音段 2.继续检测 直到语音段结束 记录begin和end时间点
 * */
vector<SEGMENT> AutoSegmenter::seg_utts_by_Intensity(
            vector<INTENSITY>  vIntensity, double fThresholdCoef)
{
    // 所有 大于0的 能量值 
	vector<double> vEnergy;
	vector<INTENSITY>::iterator pIter = vIntensity.begin();
	for (; pIter != vIntensity.end(); pIter++)
	{
		if (pIter->value > 0)
			vEnergy.push_back(pIter->value);
	}

    // 能量的平均值 标准差 
	double fmean = mean(vEnergy);
	double fstddev = stddev(vEnergy);
	double fthreshold = fmean - fstddev * fThresholdCoef;

	vector<SEGMENT> vUtts;
    // 小于0表示:前面还没有语音
	double beg = -1;

	pIter = vIntensity.begin();
	for (; pIter != vIntensity.end(); pIter++)
	{
		if (pIter->value <= 0)
			continue;

        // 前面 未检测到语音开始 
		if (beg < 0)
		{
            // 检测到能量大于阈值窗 语音开始
			if (pIter->value >= fthreshold)
				beg = pIter->stamp;
			else
				continue;
		} 
        else // 前面已经检测到语音开始了
        {
            // 当前还是语音区域 那么继续 
			if (pIter->value >= fthreshold)
				continue;
			else 
            {
                // 前面是语音 当前语音结束区域
				SEGMENT curseg;
                // 这个时间是 窗中心 
				curseg.begin = beg; 
				curseg.end = pIter->stamp;
				vUtts.push_back(curseg);
                // 后面继续检测语音开始 
				beg = -1;
			}
		}
	}

	if (beg > 0) 
    {
		INTENSITY pint = vIntensity.at(vIntensity.size()-1);
		if (pint.stamp - beg > 0.2) 
        {
			SEGMENT curseg;
			curseg.begin = beg;
			curseg.end = pint.stamp;
			vUtts.push_back(curseg);
		}
		beg = -1;
	}

	return vUtts;
}

/*
 * pData: 语音数据 
 * nSamples: 采样点个数
 * fMinPitch: 最小pitch
 * fTimeStep:
 * fMaxPause:
 * fThresholdCoef: 阈值系数
 * fBeginPad: 左侧向左移动的时间长度(秒)
 * fEndPad: 右侧向右移动
 * */
vector<SEGMENT> AutoSegmenter::getSegment(short* pData, int nSamples, double fMinPitch, double fTimeStep, double fMaxPause, double fThresholdCoef, double fBeginPad, double fEndPad)
{
	// 1. calculate intensity vector
	vector<INTENSITY> vIntensity = calcIntensity(pData, nSamples, fMinPitch, fTimeStep);

	// 2. make utterances
	vector<SEGMENT> vSegment = seg_utts_by_Intensity(vIntensity, fThresholdCoef);

	// 3. combine utterances
	vector<SEGMENT> vUtts = combine_utts(vSegment, fMaxPause);

	// 4. pad utterances
	vector<SEGMENT> vPad = pad_utts(vUtts, nSamples * 1.0/m_nSampleRate, fBeginPad, fEndPad);

	return vPad;
}






