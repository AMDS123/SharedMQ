SENDER = sender
RECEIVER = receiver

all: $(SENDER) $(RECEIVER)

$(SENDER): sender.o $(OBJS)
	$(CXX) $(INC) -g -o $@ $^ $(LIB)

$(RECEIVER): receiver.o $(OBJS)
	$(CXX) $(INC) -g -o $@ $^ $(LIB)

clean:
	rm $(SENDER) $(RECEIVER) *.o

%.o: %.cpp
	$(CXX) $(INC) -c -o $@ $< $(LIB)
