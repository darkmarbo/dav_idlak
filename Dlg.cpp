

		// 逐个文件处理
		// 1. open wave file
		WaveIO wav;
		CString strInfo = wav.openWave(csInWavFile);


		// 2. determin channel index
		int nCurChannel = m_nChannelIdx;
		if (nCurChannel >= wav.getChannels()) {
			nCurChannel = 0;
		}

		// 3. get speech data
		short *pData = NULL;
		int nSamples = wav.getData(&pData, nCurChannel);

		// 4. do auto segment
		AutoSegmenter segmenter(wav.getSampleRate());

		vector<SEGMENT> vSegs = segmenter.getSegment(pData,nSamples,m_fMinPitch,
                    m_fTimeStep, m_fMinSilence, m_fThresholdCoef, m_fBeginPad, m_fEndPad);

		delete [] pData;

		strMessage.Format("  %d segments selected", vSegs.size());



		// 5. extract segment wave into files
		for (unsigned int j=0;j<vSegs.size(); j++)
		{
			CString strindex;
			strindex.Format("%05d", nWavIdx++);
			CString szOutFile;
			// szm 输出wav文件名字 和 目录结构
			// csInWavFile.Replace(".wav","");
			szOutFile.Format("%s\\%s+%.4f+%.4f.wav", csOutpath, 
				cs_wav_name_head,vSegs[j].begin, vSegs[j].end);

			wav.writeWavFile(szOutFile, vSegs[j].begin, vSegs[j].end);

			strMessage.Format("    segment %d: begin=%.4f end=%.4f name=%s", 
				j+1, vSegs[j].begin, vSegs[j].end, strindex);
			m_cLog.AddString(strMessage);
			m_cLog.SetCurSel(m_cLog.GetCount()-1);
		}









// mmm
void CAudioSegmentDlg::OnBnClickedButton2()
{
	
	
	// 3. get the log file name
	
	static char szFilter[] = "Log Files (*.log;*.txt)|*.wav;*.txt|All files(*.*)|*.*||";
	CFileDialog   Filedlg(TRUE, NULL, NULL, 
		OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT|OFN_EXPLORER, 
		szFilter, NULL);
	
	if(Filedlg.DoModal()   ==   IDOK) 
	{

		CString logname = Filedlg.GetPathName();
		CStdioFile file;  
		CString strMessage;
		if (!file.Open(_T(logname),CFile::modeCreate |CFile::modeWrite | CFile::typeText)) {
			MessageBox(_T("打开失败"));
			return;
		}
		
		for (unsigned int i=0; i<filenamelist.size(); i++) 
		{
			// 逐个文件处理
			// 1. open wave file
			if (filenamelist[i].Right(3).CompareNoCase(CString("wav")) != 0)
				continue;

			WaveIO wav;
			CString strInfo = wav.openWave(filenamelist[i]);

			// 2. determin channel index
			int nCurChannel = m_nChannelIdx;
			if (nCurChannel >= wav.getChannels()) {
				nCurChannel = 0;
			}

			// 3. get speech data
			short *pData = NULL;
			int nSamples = wav.getData(&pData, nCurChannel);

			// 4. do auto segment
			AutoSegmenter segmenter(wav.getSampleRate());
			vector<SEGMENT> vSegs = segmenter.getSegment(pData,nSamples,
				m_fMinPitch,m_fTimeStep,
				m_fMinSilence, m_fThresholdCoef, 
				m_fBeginPad, m_fEndPad);

			delete [] pData;

			// 5. get the begining and ending point of the max length segment wave

			double fbegin = 0.0, fend = 0.0;
			double maxlen = 0.0;

			for (unsigned int j=0;j<vSegs.size(); j++)
			{
				if (vSegs[j].end - vSegs[j].begin > maxlen) 
				{
					maxlen = vSegs[j].end - vSegs[j].begin;
					fbegin = vSegs[j].begin;
					fend = vSegs[j].end;
				}
			}
			
			// 6. save strMessage to log file in a line;
			strMessage.Format("%s\t%.4f\t%.4f\n", filenamelist[i],fbegin, fend);
			file.WriteString(strMessage); 
			
			strMessage.Format("%s    %.4f    %.4f\n", filenamelist[i],fbegin, fend);
			m_cLog.AddString(strMessage);
			m_cLog.SetCurSel(m_cLog.GetCount()-1);

		}
		file.Close(); 

		AfxMessageBox("干活完毕啦");
	}
}


