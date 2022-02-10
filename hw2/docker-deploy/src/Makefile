PROGS=httpproxy
OBJS=$(patsubst %,%.o,$(PROGS)) server.o helper.o client.o
all: $(PROGS)
httpproxy: httpproxy.o server.o helper.o client.o
	g++ $(CPPFLAGS) -o $@ $^ -g
%.o: %.cpp
	g++ $(CPPFLAGS) -c $<

.PHONY: clean
clean:
	rm -f *~ $(PROGS) $(OBJS)

client.o: client.hpp
server.o: server.hpp
helper.o: helper.hpp