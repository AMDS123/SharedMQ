SENDER = sender
RECEIVER = receiver

all: $(SENDER) $(RECEIVER)

$(SENDER): sender.o $(OBJS)
	$(CXX) $(INC) -o $@ $^

$(RECEIVER): receiver.o $(OBJS)
	$(CXX) $(INC) -o $@ $^

clean:
	rm $(SENDER) $(RECEIVER) *.o

%.o: %.cpp
	$(CXX) $(INC) -c -o $@ $<