CXX = g++

INCLUDES = -I./ 
CXXFLAGS = -O3 -Wall -DNDEBUG $(INCLUDES) 
LDFLAGS = -lexanic -lpacket_handler_gcc ./libpacket_handler_gcc.a -L./ 

TARGETS = dce-sub-exanic

SRC = $(wildcard ./*.c)
OBJS = $(SRC:%.c=%.o)

#防止all clean文件时，make all或者make clean执行失败
.PHONY: all clean $(TARGETS)

all : $(TARGETS)

dce-sub-exanic: dce-sub-exanic.o
	$(CXX) $^ -o $@ $(LDFLAGS)

%.o:%.c
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJS) $(TARGETS) *.o

