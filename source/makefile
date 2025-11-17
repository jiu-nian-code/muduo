.PHONY : ALL
ALL : main client

main : main.cpp
	g++ -o $@ $^ -std=c++11 -g
client : client.cpp
	g++ -o $@ $^ -std=c++11
.PHONY : clean
clean :
	rm -f main client