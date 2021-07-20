#----------------------------------
# example Makefile used to generate
# binary .so file
#----------------------------------

XX			= g++
LIBS		= -L. -L ../../../friend_backend/common -lcommon
INCLUDES	= -I. -I ../../../friend_backend/common -I ../../../friend_backend/async-server/include
LIBS        += -lhiredis
INCLUDES    += -I ../../../friend_backend/hiredis
# INCLUDES    += -I /usr/local/include/librdkafka
# LIBS        += -L /usr/local/lib -lrdkafka 
FLAGS		= -std=c++0x -Wall -Wnon-virtual-dtor -DNDEBUG -O0 -ggdb -shared -fPIC -MMD -g
FLAGS       += -DDEBUG
FLAGS       += `pkg-config --libs --cflags protobuf`
SOURCE		= $(wildcard *.cpp)
OBJS		= $(patsubst %.cpp, %.o, $(SOURCE))
TARGET		= ../bin/recommend-inbox.so

all: $(TARGET)

$(TARGET): $(OBJS)
	$(XX) $(FLAGS) $(OBJS) -o $(TARGET) $(LIBS)

%.o: %.cpp Makefile
	$(XX) $(FLAGS) $(INCLUDES) -c $< -o $@
	@-mv -f $*.d .dep.$@

clean:
	rm -rf *.o .dep.* $(TARGET)

c: clean

-include /dev/null $(wildcard .dep.*)
