SENDER = sender.test
RECEIVER = receiver.test

all: $(SENDER) $(RECEIVER)

HEADFILES_REF = $(wildcard ../../include/shm/*.hpp)

$(SENDER): sender.o $(OBJS) $(HEADFILES_REF)
	$(CXX) $(INC) -g -o $@ $^ $(LIB)

$(RECEIVER): receiver.o $(OBJS) $(HEADFILES_REF)
	$(CXX) $(INC) -g -o $@ $^ $(LIB)

clean:
	rm $(SENDER) $(RECEIVER) *.o

%.o: %.cpp
	$(CXX) $(INC) -c -o $@ $< $(LIB)
