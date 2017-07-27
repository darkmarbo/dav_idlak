#include "WaveIO.h"

WaveIO::WaveIO(void)
: m_nHeadType(0)
, m_pData(NULL)
{
}

WaveIO::~WaveIO(void)
{
	if (m_pData) {
		delete []m_pData;
		m_pData = NULL;
	}
}

// 打开文件，读取头和数据
int WaveIO::openWave(const std::string &pcsFileName)
{
	FILE* fp = fopen(pcsFileName.c_str(), "rb");
	if(fp == NULL) 
		return -1;

	fseek(fp, 0, SEEK_SET);
	fread(&m_pcm_header, 1, sizeof(PCM_Data), fp);
	if (memcmp(m_pcm_header.data_chunk_.ckID, "data", 4) != 0) 
    {
		fseek(fp, 0, SEEK_SET);
		fread(&m_big_header, 1, sizeof(Big_PCM_Data), fp);
		if (memcmp(m_big_header.data_chunk_.ckID, "data", 4) != 0) 
        {
			fseek(fp, 0, SEEK_SET);
			fread(&m_fact_header, 1, sizeof(Fact_PCM_Data), fp);
			if (memcmp(m_fact_header.data_chunk_.ckID, "data", 4) != 0) 
            {
				fseek(fp, 0, SEEK_SET);
				fread(&m_non_header, 1, sizeof(NON_PCM_Data), fp);
				if (memcmp(m_non_header.data_chunk_.ckID, "data", 4) != 0) 
                {
					fseek(fp, 0, SEEK_SET);
					fread(&m_ext_header, 1, sizeof(Ext_PCM_Data), fp);
					if (memcmp(m_ext_header.data_chunk_.ckID, "data", 4) != 0) 
                    {
						// not a wave file
						return -2;
					} 
                    else 
                    {
						//Ext_PCM_Data
						m_nHeadType = EXT_PCM_HEADER;
					}
				} 
                else 
                {	
					//NON_PCM_Data
					m_nHeadType = NON_PCM_HEADER;
				}
			} 
            else 
            {
				//Fact_PCM_Data
				m_nHeadType = FACT_PCM_HEADER;
			}
		} 
        else 
        {
			//big data
			m_nHeadType = BIG_PCM_HEADER;
		}
	}
    else 
    {
		//pcm_data
		m_nHeadType = STD_PCM_HEADER;
	}

	if (m_pData)
	{
		delete []m_pData;
		m_pData = NULL;
	}	

	int nSamples = getSamples() * getChannels();
	m_pData = new short [nSamples];
	if(m_pData == NULL)
	{
		return -3;
	}
	fread(m_pData, 1, nSamples * sizeof(short), fp);
	fclose(fp);
	fp = NULL;

	return 0;
}

// 将语音片段写入文件
int WaveIO::writeWavFile(const std::string &csOutFile, double fBegin, double fEnd)
{
	int nbeginpos = (int)(fBegin * getSampleRate());
	int nendpos = (int)(fEnd * getSampleRate());
	if (nbeginpos < 0) nbeginpos = 0;
	if (nendpos > getSamples()) nendpos = getSamples();

	int nSamples = nendpos - nbeginpos;
	if (nSamples <= 0) return -1;

	short *pData = NULL;
	int nchkSample = getData(&pData, getChannels(), fBegin, fEnd);

	FILE *fp = fopen(csOutFile.c_str(), "wb");
	if (fp == NULL) 
		return -2;

	PCM_Data waveheader = getStdWavHeader(nchkSample);
	fwrite(&waveheader, 1, sizeof(PCM_Data), fp);

	fwrite(pData, 1, nSamples * getChannels() * sizeof(short), fp);
	fclose(fp);

	delete []pData;

	return 0;
}

// 从现有语音文件中提取44字节的标准WAV格式头
PCM_Data WaveIO::getStdWavHeader(int nSamples)
{
	PCM_Data std_pcm;
	memset(&std_pcm, 0 ,sizeof(PCM_Data));

	if (m_nHeadType == EXT_PCM_HEADER) {
		memcpy(&(std_pcm.wave_head_), &(m_ext_header.wave_head_), sizeof(WaveHeadChunk));
		memcpy(&(std_pcm.data_chunk_), &(m_ext_header.data_chunk_), sizeof(DataChunk));

		memcpy(std_pcm.format_chunk_.ckID, m_ext_header.format_chunk_.ckID, 4);
		std_pcm.format_chunk_.ckSize = 16;
		std_pcm.format_chunk_.nAvgBytesPerSec = m_ext_header.format_chunk_.nAvgBytesPerSec;
		std_pcm.format_chunk_.nBlockAlign = m_ext_header.format_chunk_.nBlockAlign;
		std_pcm.format_chunk_.nChannels = m_ext_header.format_chunk_.nChannels;
		std_pcm.format_chunk_.nSamplesPerSec = m_ext_header.format_chunk_.nSamplesPerSec;
		std_pcm.format_chunk_.wBitsPerSample = m_ext_header.format_chunk_.wBitsPerSample;
		std_pcm.format_chunk_.wFormatTag = m_ext_header.format_chunk_.wFormatTag;

		std_pcm.wave_head_.filelong -= (sizeof(Ext_FormatChunk) - sizeof(PCM_FormatChunk) + sizeof(FactChunk));
		std_pcm.data_chunk_.ckSize = std_pcm.wave_head_.filelong;
	} else if (m_nHeadType == NON_PCM_HEADER) {
		memcpy(&(std_pcm.wave_head_), &(m_non_header.wave_head_), sizeof(WaveHeadChunk));
		memcpy(&(std_pcm.format_chunk_), &(m_non_header.format_chunk_), sizeof(NON_PCM_FormatChunk));
		memcpy(&(std_pcm.data_chunk_), &(m_non_header.data_chunk_), sizeof(DataChunk));

		std_pcm.format_chunk_.ckSize -= 2;
		std_pcm.wave_head_.filelong -= (sizeof(FactChunk) + 2);
		std_pcm.data_chunk_.ckSize = std_pcm.wave_head_.filelong;
	} else if (m_nHeadType == FACT_PCM_HEADER) {
		memcpy(&(std_pcm.wave_head_), &(m_fact_header.wave_head_), sizeof(WaveHeadChunk));
		memcpy(&(std_pcm.format_chunk_), &(m_fact_header.format_chunk_), sizeof(PCM_FormatChunk));
		memcpy(&(std_pcm.data_chunk_), &(m_fact_header.data_chunk_), sizeof(DataChunk));

		std_pcm.wave_head_.filelong -= sizeof(FactChunk);
		std_pcm.data_chunk_.ckSize = std_pcm.wave_head_.filelong;
	} else if (m_nHeadType == BIG_PCM_HEADER) {
		memcpy(&(std_pcm.wave_head_), &(m_big_header.wave_head_), sizeof(WaveHeadChunk));
		memcpy(&(std_pcm.format_chunk_), &(m_big_header.format_chunk_), sizeof(NON_PCM_FormatChunk));
		memcpy(&(std_pcm.data_chunk_), &(m_big_header.data_chunk_), sizeof(DataChunk));

		std_pcm.format_chunk_.ckSize -= 2;
		std_pcm.wave_head_.filelong -= 2;
		std_pcm.data_chunk_.ckSize = std_pcm.wave_head_.filelong;
	} else if (m_nHeadType == STD_PCM_HEADER) {
		memcpy(&std_pcm, &m_pcm_header, sizeof(PCM_Data));
	}

	if (nSamples > 0) {
		std_pcm.data_chunk_.ckSize = nSamples * getChannels() * sizeof(short);
		std_pcm.wave_head_.filelong = std_pcm.data_chunk_.ckSize + 36;
	}
	return std_pcm;
}

// 返回采样率
int WaveIO::getSampleRate(void)
{
	int nSampleRate = 0;
	if (m_nHeadType == EXT_PCM_HEADER) {
		nSampleRate = m_ext_header.format_chunk_.nSamplesPerSec;
	} else if (m_nHeadType == NON_PCM_HEADER) {
		nSampleRate = m_non_header.format_chunk_.nSamplesPerSec;
	} else if (m_nHeadType == FACT_PCM_HEADER) {
		nSampleRate = m_fact_header.format_chunk_.nSamplesPerSec;
	} else if (m_nHeadType == BIG_PCM_HEADER) {
		nSampleRate = m_big_header.format_chunk_.nSamplesPerSec;
	} else if (m_nHeadType == STD_PCM_HEADER) {
		nSampleRate = m_pcm_header.format_chunk_.nSamplesPerSec;
	}

	return nSampleRate;
}

// 返回通道数
int WaveIO::getChannels(void)
{
	int nChannels = 0;
	if (m_nHeadType == EXT_PCM_HEADER) {
		nChannels = m_ext_header.format_chunk_.nChannels;
	} else if (m_nHeadType == NON_PCM_HEADER) {
		nChannels = m_non_header.format_chunk_.nChannels;
	} else if (m_nHeadType == FACT_PCM_HEADER) {
		nChannels = m_fact_header.format_chunk_.nChannels;
	} else if (m_nHeadType == BIG_PCM_HEADER) {
		nChannels = m_big_header.format_chunk_.nChannels;
	} else if (m_nHeadType == STD_PCM_HEADER) {
		nChannels = m_pcm_header.format_chunk_.nChannels;
	}

	return nChannels;
}

// 返回采样点个数
int WaveIO::getSamples(void)
{
	int nSample = 0;
	if (m_nHeadType == EXT_PCM_HEADER) {
		nSample = m_ext_header.data_chunk_.ckSize;
	} else if (m_nHeadType == NON_PCM_HEADER) {
		nSample = m_non_header.data_chunk_.ckSize;
	} else if (m_nHeadType == FACT_PCM_HEADER) {
		nSample = m_fact_header.data_chunk_.ckSize;
	} else if (m_nHeadType == BIG_PCM_HEADER) {
		nSample = m_big_header.data_chunk_.ckSize;
	} else if (m_nHeadType == STD_PCM_HEADER) {
		nSample = m_pcm_header.data_chunk_.ckSize;
	}

	return nSample / (getChannels() * sizeof(short));
}

// 提取某个通道指定时间段的语音数据
int WaveIO::getData(short** pData, int nChannel, double fBeginStamp, double fEndStamp)
{
	if (m_pData == NULL)
    {
        printf("getData:m_pData==NULL\n");
		return 0;
    }
	if (fBeginStamp < 0)
		fBeginStamp = 0;
	if (fEndStamp > getSamples() * 1.0 / getSampleRate()) 
		fEndStamp = getSamples() * 1.0 / getSampleRate();

	int nSamples = (int)((fEndStamp - fBeginStamp) * getSampleRate());
	if (fEndStamp < 0)		nSamples = getSamples();
	if (nSamples <= 0)		return 0;

	int nTotalChannels = getChannels();
	int nChs = 1;
	if (nChannel < 0 || nChannel >= nTotalChannels) 
		nChs = nTotalChannels;

	short *pbuf = NULL;
	pbuf = new short [nSamples * nChs];
	if (pbuf == NULL)
    {
        printf("getData:pbuf==NULL\n");
		return 0;
    }
	
	short *p = m_pData + (int)(fBeginStamp * getSampleRate()) * nTotalChannels;
	for (int i = 0; i < nSamples; i++)
	{
		if (0 <= nChannel && nChannel < nTotalChannels) {
			pbuf[i] = p[i * nTotalChannels + nChannel];
		} else {
			for (int j = 0; j < nTotalChannels; j++) {
				pbuf[ i * nTotalChannels + j ] = p[ i * nTotalChannels + j ];
			}
		}
	}

	*pData = pbuf;

	return nSamples;
}

