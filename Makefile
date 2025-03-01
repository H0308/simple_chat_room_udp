.PHONY:all
all:server_udp client_udp

server_udp:udp_server_main.cc
	g++ -o $@ $^ -std=c++17 -lpthread
client_udp:udp_client_main.cc
	g++ -o $@ $^ -std=c++17 -lpthread

.PHONY:clean
clean:
	rm -f server_udp client_udp