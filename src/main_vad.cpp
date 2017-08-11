
#include"common.h"
#include"WaveIO.h"
#include"AutoSegmenter.h"
#include"Config.h"


int main(int argc, char *argv[])
{

    /*
     * vad  1  name.wav  dir_out
     * vad  2  name.wav  file_time  dir_out 
     * */
    if(argc < 4)
    {
        printf("usage: %s  1  input_wav  output_directory\n", argv[0]);
        printf("usage: %s  2  input_wav  input_file  output_directory\n", argv[0]);
        printf("\t\t 大语音input_wav  对应时间点input_file 切分到output_directory\n");
        return 0;
    }

    int ret = 0;

    string m_sPath = "vad.conf";
    map<string,string> m_mapConfig;
    // 1. config
    ReadConfig(m_sPath, m_mapConfig);

    double m_fMinPitch = 75;        // 最小音高值
    double m_fTimeStep = 0.;        // 步长
    double m_fMinSilence = 0.6;     // 最短句间静音长度
    double m_fThresholdCoef = 0.33; // Threshold coefficient
    int m_nChannelIdx = 0;          // 用于分隔通道序号
    double m_fBeginPad = 0.3;       // 头静音长度
    double m_fEndPad = 0.3;         // 尾静音长度

    getDouble(m_mapConfig, "MinPitch", m_fMinPitch);
    getDouble(m_mapConfig, "TimeStep", m_fTimeStep);
    getDouble(m_mapConfig, "MinSilence", m_fMinSilence);
    getDouble(m_mapConfig, "ThresholdCoef", m_fThresholdCoef);
    double temp_chann = double(m_nChannelIdx);
    getDouble(m_mapConfig, "ChannelIdx", temp_chann);
    m_nChannelIdx = int(temp_chann);
    getDouble(m_mapConfig, "BeginPad", m_fBeginPad);
    getDouble(m_mapConfig, "EndPad", m_fEndPad);

    // Config  测试  
    printf("Config list:\n");
    printf("\tMinPitch=%.4f\n\tTimeStep=%.4f\n\tMinSilence=%4.f\n\tThresholdCoef=%.4f\n\tChannelIdx=%d\n\tfBeginPad=%.4f\n\tEndPad=%.4f\n",
            m_fMinPitch, m_fTimeStep, m_fMinSilence, m_fThresholdCoef,
            m_nChannelIdx, m_fBeginPad, m_fEndPad);

    std::string wav_name = argv[2];
    WaveIO wav;
    // 2. open wave file
    ret = wav.openWave(wav_name.c_str());
    if(ret != 0)
    {
        printf("读取wav %s 失败! 返回值=%d\n",wav_name.c_str(), ret);
        return 0;
    }

    // 3. 获取 通道数、单个通道采样点数
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

    // 4. get speech data
    short *pData = NULL;
    // nCurChannel 为要读取的通道序号: 0 
    int nSamples = wav.getData(&pData, nCurChannel, 0, -1);
    printf("read nSamples=%d \n",nSamples);
    if(nSamples < 1) return 0;



    /////////////////////       开始处理  
    int flag = atoi(argv[1]);
    // vad 自动切分大语音
    if(flag == 1)
    {


        std::string dir_out = argv[3];

        if(dir_out.empty() == false && dir_out[dir_out.size()-1] == '/')
          dir_out.erase(dir_out.size()-1);

        std::string log_name = dir_out+".log";

        FILE *fp_log = NULL;

        cout<<"LOG:开始切分wav:\t"<<wav_name<<endl;
        cout<<"LOG:切分结果目录:\t"<<dir_out<<endl;
        cout<<"LOG:切分信息输出:\t"<<log_name<<endl;
        fp_log = fopen(log_name.c_str(),"w");
        if(fp_log == NULL)
        {
            printf("ERROR:打开语音文件失败!\n");
            return 0;
        }


        // 5. do auto segment
        AutoSegmenter segmenter(wav.getSampleRate());

        vector<SEGMENT> vSegs = segmenter.getSegment(pData, nSamples, m_fMinPitch,
                    m_fTimeStep, m_fMinSilence, m_fThresholdCoef, m_fBeginPad, m_fEndPad);

        printf("  %d segments selected\n", int(vSegs.size()));
        fprintf(fp_log, "segments selected\t%d\n", int(vSegs.size()));


        // 6. extract segment wave into files
        // 语音命名规则: 000001_0.123_4.567.wav 
        char wav_out[1024] = {0};
        int file_id = 0;
        for (unsigned int j=0; j<vSegs.size(); j++)
        {
            file_id++;
            char wav_name[1024] = {0};
            //snprintf(wav_name, 1024, "%06d.wav", file_id);
            snprintf(wav_name, 1024, "%06d_%.4f_%.4f.wav", file_id, vSegs[j].begin, vSegs[j].end);

            snprintf(wav_out, 1024, "%s/%s", dir_out.c_str(), wav_name);
            fprintf(fp_log, "%s\t%.4f\t%.4f\n", wav_name, vSegs[j].begin, vSegs[j].end);

            wav.writeWavFile(wav_out, vSegs[j].begin, vSegs[j].end);


        }

        fclose(fp_log);


    }
    else if (2 == flag)
    {
        std::string file_time = argv[3];
        std::string dir_out = argv[4];
        double time_st, time_end;

        FILE *fp_time = fopen(file_time.c_str(), "r");

        char line[1024] = {0};
        while(fgets(line, 1024, fp_time))
        {
            vector<string> vec_line;
            ////7	你去过对岸吗？小狐狸问.
            split(vec_line, string(line), '\t');
            if(vec_line.size()<2)
            {
                break;
            }
            string wav_id = vec_line[0];

            ////	33	34	95.6757	99.6142	0.75	6.69
            if(fgets(line, 1024, fp_time) == NULL)
            {
                break;
            }
            vec_line.clear();
            split(vec_line, string(line), '\t');
            if(vec_line.size() != 6)
            {
                break;
            }
            time_st = atof(vec_line[2].c_str());
            time_end = atof(vec_line[3].c_str());

            fprintf(stderr, "line=%s\n", line);
            fprintf(stderr, "wav_id=%s\ttime_st=%.4f\ttime_end=%.4f\n", 
                        wav_id.c_str(), time_st, time_end);
            
            // 写wav 
            char wav_out[1024] = {0};
            snprintf(wav_out, 1024, "%s/%s.wav", dir_out.c_str(), wav_id.c_str());
            ret = wav.writeWavFile(wav_out, time_st, time_end);
            // 写wav 失败 
            if(ret != 0)
            {
                printf("ERROR:writeWav Failed! ret=%d\n", ret);
            }
            

        }

        if(fp_time){fclose(fp_time); fp_time = NULL;}


    }


    //    delete  
    if(pData)
    {
        delete [] pData;
        pData = NULL;
    }
        
    cout<<"completed!"<<endl;
    return 0;


}




