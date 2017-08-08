#!/bin/sh


#ls -1 audioSegment|while read file
#do
#    echo $file
#    iconv -f gbk -t utf-8 audioSegment/$file  >  ttt/$file
#done


##sox duianyoushenme.wav -t wav duianyoushenme_sox.wav
dir_out=test_out
rm -rf ${dir_out}  && mkdir -p ${dir_out}
./vad_tools/vad   test/duianyoushenme_sox.wav  ${dir_out}

#将 切分后的wav 移动到
#    /home/szm/cd/kaldi-master/test
#使用sox.sh 转换成 16k 
#运行 nohup ./gdb_test.sh & 
#得到 nohup.out 结果 



