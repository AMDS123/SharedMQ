CUR_OBJS = $(patsubst %.cpp, %.o, $(wildcard *.cpp))

HEADFILES_DIR = ../../include/util/

all:$(CUR_OBJS)

clean:
	rm *.o

%.o: %.cpp $(HEADFILES_DIR)%.hpp
	$(CXX) $(INC) -c -o $@ $< $(LIB)
