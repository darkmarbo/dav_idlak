#!/bin/sh


#ls -1 audioSegment|while read file
#do
#    echo $file
#    iconv -f gbk -t utf-8 audioSegment/$file  >  ttt/$file
#done


####   1. 测试切分 
####sox duianyoushenme.wav -t wav duianyoushenme_sox.wav
#dir_out=test_out
#rm -rf ${dir_out}  && mkdir -p ${dir_out}
#./vad_tools/vad   1   test/duianyoushenme_sox.wav  ${dir_out}


###  2. asr 识别 
#将 切分后的wav 移动到
#    /home/szm/cd/kaldi-master/test
#使用sox.sh 转换成 16k 
#运行 nohup ./gdb_test.sh & 
#得到 nohup.out 结果 


#### 3. 矫正结果time  重新切分  
dir_out=re_vad_out
rm -rf ${dir_out}  && mkdir -p ${dir_out}
####   将原始语音 wav  按照duian.out的时间点  切分到目录 dir_out 中 
./vad_tools/vad   2   test/duianyoushenme_sox.wav  duian.out  ${dir_out}

