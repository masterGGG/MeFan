#----------------------------------
# example Makefile used to generate
# binary .so file
#----------------------------------

XX			= g++
LIBS		= -L. -L ./common -lcommon
INCLUDES	= -I. -I ./common -I ./include
LIBS        += -L./hiredis -lhiredis
INCLUDES    += -I ./hiredis
FLAGS		= -std=c++0x -Wall -Wnon-virtual-dtor -DNDEBUG -O2 -ggdb -shared -fPIC -MMD -g
FLAGS       += -DDEBUG
FLAGS       += `pkg-config --libs --cflags protobuf`
SOURCE		= $(wildcard *.cpp)
OBJS		= $(patsubst %.cpp, %.o, $(SOURCE))
TARGET		= ../bin/cache.so

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
