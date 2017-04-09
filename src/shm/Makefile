CUR_OBJS = $(patsubst %.cpp, %.o, $(wildcard *.cpp))

HEADFILES_DIR = ../../include/shm/

UTIL_HEADFILE_REF = $(wildcard ../../include/util/*.h)

all:$(CUR_OBJS)
	echo $(CUR_OBJS)

clean:
	rm *.o

%.o: %.cpp $(HEADFILES_DIR)%.h $(UTIL_HEADFILE_REF)
	echo $(CUR_OBJS)
	$(CXX) $(INC) -c -o $@ $< $(LIB)
