#pragma once
#include "common.h"

#define STD_PCM_HEADER	1
#define BIG_PCM_HEADER	2
#define FACT_PCM_HEADER	3
#define NON_PCM_HEADER	4
#define EXT_PCM_HEADER	5

// mmm   4位 32字节
typedef   int   DWORD;
//typedef   unsigned long   DWORD;
// mmm   2位 16字节
typedef   unsigned short   WORD;

typedef struct _WavHeadChunk_ {
	char riff[4];		//"RIFF"
	DWORD filelong;	//
	char waveid[4];		//"WAVE"
} WaveHeadChunk;

typedef struct _PCM_FormatChunk_ {
	char ckID[4];		//"fmt "
	DWORD ckSize;		//Chunk size: 16
	WORD wFormatTag;
	WORD nChannels;
	DWORD nSamplesPerSec;
	DWORD nAvgBytesPerSec;
	WORD  nBlockAlign;
	WORD  wBitsPerSample;
} PCM_FormatChunk;

typedef struct _NON_PCM_FormatChunk_ {
	char ckID[4];		//"fmt "
	DWORD ckSize;		//Chunk size: 16
	WORD wFormatTag;
	WORD nChannels;
	DWORD nSamplesPerSec;
	DWORD nAvgBytesPerSec;
	WORD  nBlockAlign;
	WORD  wBitsPerSample;
	WORD  cbSize;	
} NON_PCM_FormatChunk;

typedef struct _Ext_FormatChunk_ {
	char ckID[4];		//"fmt "
	DWORD ckSize;		//Chunk size: 16
	WORD wFormatTag;
	WORD nChannels;
	DWORD nSamplesPerSec;
	DWORD nAvgBytesPerSec;
	WORD  nBlockAlign;
	WORD  wBitsPerSample;
	WORD  cbSize;
	WORD  wValidBitsPerSample;
	DWORD dwChannelMask;
	char  SubFormat[16];
} Ext_FormatChunk;


typedef struct _FactChunk_ {
	char ckID[4];		//"fact"
	DWORD ckSize;		//chunk size : 4
	DWORD dwSampleLength;
} FactChunk;

typedef struct _DataChunk_ {
	char ckID[4];		//"data"
	DWORD ckSize;
} DataChunk;


// wave format definition
typedef struct _PCM_Data_ {
	WaveHeadChunk	wave_head_;
	PCM_FormatChunk format_chunk_;
	DataChunk	data_chunk_;
} PCM_Data;

typedef struct _Fact_PCM_Data_ {
	WaveHeadChunk	wave_head_;
	PCM_FormatChunk format_chunk_;
	FactChunk   fact_chunk_;
	DataChunk	data_chunk_;
} Fact_PCM_Data;

typedef struct _Big_PCM_Data_ {
	WaveHeadChunk	wave_head_;
	NON_PCM_FormatChunk format_chunk_;
	DataChunk	data_chunk_;
} Big_PCM_Data;

typedef struct _NON_PCM_Data_ {
	WaveHeadChunk	wave_head_;
	NON_PCM_FormatChunk format_chunk_;
	FactChunk   fact_chunk_;
	DataChunk	data_chunk_;
} NON_PCM_Data;

typedef struct _Ext_PCM_Data_ {
	WaveHeadChunk	wave_head_;
	Ext_FormatChunk format_chunk_;
	FactChunk   fact_chunk_;
	DataChunk	data_chunk_;
} Ext_PCM_Data;

class WaveIO
{
public:
	WaveIO(void);
	~WaveIO(void);

public:
	// 打开文件，读取头和数据
	int openWave(const std::string &pcsFileName);
	// 将语音片段写入文件
	int writeWavFile(const std::string &csOutFile, double fBegin, double fEnd);
	// WAV头文件格式
	int m_nHeadType;
	// 从现有语音文件中提取44字节的标准WAV格式头
	PCM_Data getStdWavHeader(int nSamples=0);
	// 返回采样率
	int getSampleRate(void);
	// 返回通道数
	int getChannels(void);
	// 返回采样点个数
	int getSamples(void);
	// 提取某个通道指定时间段的语音数据
	int getData(short** pData = NULL, int nChannel = 0, double fBeginStamp = 0, double fEndStamp = -1);

public:
	// 标准的44字节Wav头
	PCM_Data m_pcm_header;
	// 非标准46字节Wav头
	Big_PCM_Data m_big_header;
	// 标准56字节FACT形式Wav头
	Fact_PCM_Data m_fact_header;
	// 复杂的58字节的NON_PCM的Wav头
	NON_PCM_Data m_non_header;
	// 标准的EXT格式的Wav头
	Ext_PCM_Data m_ext_header;
	// 语音数据
	short* m_pData;
};
