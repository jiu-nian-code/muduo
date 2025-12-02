ALL : client main

client : client.cpp
	g++ -o $@ $^ -std=c++11
main : main.cc
	g++ -o $@ $^ -std=c++11 -pthread -g
.PHONY : clean
clean : 
	rm -f main