#!/bin/sh


#ls -1 audioSegment|while read file
#do
#    echo $file
#    iconv -f gbk -t utf-8 audioSegment/$file  >  ttt/$file
#done


sox duianyoushenme.wav -t wav duianyoushenme_sox.wav
./bin/vad   duianyoushenme_sox.wav

