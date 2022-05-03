CXX=clang++-9 
CXXFLAGS=-std=c++2a -Werror

HEADERS=stack.hpp 

all: server client test
	
server: server.o stack.o
	$(CXX) -lpthread $(CXXFLAGS) $^ -o server


client: client.o stack.o
	$(CXX) $(CXXFLAGS) $^ -o client


test: test.o
	$(CXX) $(CXXFLAGS) $^ -o test

stack.o: stack.cpp stack.hpp
	$(CXX) $(CXXFLAGS) --compile $< -o $@


malloc.o: malloc.cpp malloc.hpp stack.hpp
	$(CXX) $(CXXFLAGS) --compile $< -o $@

test.o: test.cpp
	$(CXX) $(CXXFLAGS) --compile $< -o $@


clean:
	rm -f *.o server client test