PROGS=httpproxy
OBJS=$(patsubst %,%.o,$(PROGS)) server.o helper.o client.o httpheader.o
all: $(PROGS)
httpproxy: httpproxy.o server.o helper.o client.o httpheader.o
	g++ $(CPPFLAGS) -o $@ $^ -g
%.o: %.cpp
	g++ $(CPPFLAGS) -c $< -g

.PHONY: clean
clean:
	rm -f *~ $(PROGS) $(OBJS)

client.o: client.hpp
server.o: server.hpp
helper.o: helper.hpp
httpheader.o: httpheader.hpp