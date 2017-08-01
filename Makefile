


OUT=vad_tools
src=src
INCLUDEDIR=-I $(src)
LIBDIR=-lm
#CPPFLAGS = -g -finline-functions -Wall -Winline -pipe
CPPFLAGS = -g -finline-functions -Wall -Winline -pipe -Bstatic -lstdc++ -lsupc++  -Bdynamic
name=vad
GCC=g++

obj = \
	WaveIO.o \
	Config.o \
	AutoSegmenter.o \
	main_vad.o

all: clean $(name)
	rm -rf $(OUT) && mkdir -p $(OUT)
	mv $(name) $(OUT)
	cp vad.conf  $(OUT)
	rm -rf *.o


$(name) : $(obj)
	$(GCC) -o $@ $^ $(INCLUDEDIR) $(LIBDIR)

clean:
	rm -rf $(OUT)
	rm -rf *.o

%.o : $(src)/%.cpp
	$(GCC) $(CPPFLAGS) -c $< -o $@  $(INCLUDEDIR) $(LIBDIR)



