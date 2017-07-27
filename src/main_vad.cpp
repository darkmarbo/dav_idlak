
#include"common.h"
#include"WaveIO.h"
#include"AutoSegmenter.h"
#include"Config.h"


int main(int argc, char *argv[])
{

    if(argc < 3)
    {
        printf("usage: %s input_wav out_txt\n", argv[0]);
        return 0;
    }
    std::string wav_name = argv[1];
    std::string log_name = argv[2];

	double m_fMinPitch = 75;        // 最小音高值
	double m_fTimeStep = 0.;        // 步长
	double m_fMinSilence = 0.6;     // 最短句间静音长度
	double m_fThresholdCoef = 0.33; // Threshold coefficient
	int m_nChannelIdx = 0;          // 用于分隔通道序号
	double m_fBeginPad = 0.3;       // 头静音长度
	double m_fEndPad = 0.3;         // 尾静音长度


    int ret = 0;
    FILE *fp_log = NULL;
	WaveIO wav;

    cout<<"vad start..."<<endl;
    fp_log = fopen(log_name.c_str(),"w");
    if(fp_log == NULL)
    {
        printf("fp_log == NULL\n");
        return 0;
    }

    // config
	string m_sPath = "vad.conf";
	map<string,string> m_mapConfig;
	ReadConfig(m_sPath, m_mapConfig);

    //// 测试 get config value 
    //printf("Config list:\n");
	//PrintConfig(m_mapConfig);

    //double ttt = 111.222; 
    //string tmp = "ttt";
    //if(0 != getDouble(m_mapConfig, tmp, ttt))
    //{
    //    printf("ERROR:getDouble=%.4f\n",ttt);
    //}
    //printf("LOG:getDouble=%.4f\n",ttt);
    
    getDouble(m_mapConfig, "MinPitch", m_fMinPitch);
    getDouble(m_mapConfig, "TimeStep", m_fTimeStep);
    getDouble(m_mapConfig, "MinSilence", m_fMinSilence);
    getDouble(m_mapConfig, "ThresholdCoef", m_fThresholdCoef);
    double temp_chann = double(m_nChannelIdx);
    getDouble(m_mapConfig, "ChannelIdx", temp_chann);
    m_nChannelIdx = int(temp_chann);
    getDouble(m_mapConfig, "BeginPad", m_fBeginPad);
    getDouble(m_mapConfig, "EndPad", m_fEndPad);

    printf("Config list:\n");
    printf("\tMinPitch=%.4f\n\tTimeStep=%.4f\n\tMinSilence=%4.f\n\tThresholdCoef=%.4f\n\tChannelIdx=%d\n\tfBeginPad=%.4f\n\tEndPad=%.4f\n",
                m_fMinPitch, m_fTimeStep, m_fMinSilence, m_fThresholdCoef,
                m_nChannelIdx, m_fBeginPad, m_fEndPad);

	// 1. open wave file
    ret = wav.openWave(wav_name.c_str());
    if(ret != 0)
    {
        printf("读取wav %s 失败! 返回值=%d\n",wav_name.c_str(), ret);
        return 0;
    }



	// 2. 获取通道数、单个通道采样点数
    // 语音包含几个通道 
    int nChan = wav.getChannels(); 
    printf("getChannels=%d \n",nChan);
    // 指定要处理的通道 nCurChannel=0
	int nCurChannel = m_nChannelIdx;
    if (nCurChannel >= nChan) 
    {
		nCurChannel = 0;
	}

    // 单个通道包含的采样点数  
    int nSamp = wav.getSamples();
    printf("getSamples=%d \n",nSamp);

	// 3. get speech data
	short *pData = NULL;
    // nCurChannel 为要读取的通道序号: 0 
	int nSamples = wav.getData(&pData, nCurChannel, 0, -1);
    printf("read nSamples=%d \n",nSamples);
    if(nSamples < 1) return 0;

	// 4. do auto segment
	AutoSegmenter segmenter(wav.getSampleRate());

	vector<SEGMENT> vSegs = segmenter.getSegment(pData, nSamples, m_fMinPitch,
        m_fTimeStep, m_fMinSilence, m_fThresholdCoef, m_fBeginPad, m_fEndPad);

	delete [] pData;

	printf("  %d segments selected\n", int(vSegs.size()));
	fprintf(fp_log, "  %d segments selected\n", int(vSegs.size()));



	// 5. extract segment wave into files
	for (unsigned int j=0; j<vSegs.size(); j++)
	{
		fprintf(fp_log, "%.4f\t%.4f\n", vSegs[j].begin, vSegs[j].end);

		//wav.writeWavFile(szOutFile, vSegs[j].begin, vSegs[j].end);

		//strMessage.Format("    segment %d: begin=%.4f end=%.4f name=%s", 
		//	j+1, vSegs[j].begin, vSegs[j].end, strindex);

	}

    fclose(fp_log);

    cout<<"completed!"<<endl;




}
