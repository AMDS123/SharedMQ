CUR_OBJS = ${patsubst %.cpp, %.o, ${wildcard *.cpp}}

all:$(CUR_OBJS)

clean:
	rm *.o

%.o: %.cpp
	$(CXX) $(INC) -c -o $@ $< $(LIB)
