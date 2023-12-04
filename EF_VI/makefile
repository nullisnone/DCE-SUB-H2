CC = g++

#CXXFLAGS = -Wall -m64 -g -I./include/ -std=c++11
CXXFLAGS = -Wall -m64 -g -I./ -I./include/ 

TARGET = dce-sub-efvi

OBJS = main.o mc_client_efvi.o utils.o
LIBS = -L./ ./libpacket_handler_gplus.a ./libciul1.a ./libciapp1.a -lpacket_handler_gplus -lm -lpthread -lrt -Wl,-E

.phony : all clean

all: $(TARGET)
	echo "make done!"

$(TARGET) : $(OBJS)
	$(CC) $(CXXFLAGS) -o $@ $(OBJS) $(LIBS)

$%.o : %.cpp
	$(CC) $(CXXFLAGS) -c $< -o $@

clean :
	rm -rf *.o $(TARGET)
	echo "clean done!"

