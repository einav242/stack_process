CXX=clang++-9 
CXXFLAGS=-std=c++2a -Werror

HEADERS=stack.hpp 

all: server client 
	
server: server.o malloc.o
	$(CXX) -lpthread $(CXXFLAGS) $^ -o server

client: client.o malloc.o
	$(CXX) $(CXXFLAGS) $^ -o client

malloc.o: malloc.cpp malloc.hpp stack.hpp
	$(CXX) $(CXXFLAGS) --compile $< -o $@


clean:
	rm -f *.o server client test