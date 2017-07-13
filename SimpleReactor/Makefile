CXXFLAGS = -std=c99 -g -o2 -Wall
all: server client
	@echo "...builing"

server: server.c reactor.c

client: client.c

.PHONY: clean
	@echo "...cleaning"
	rm -rf *.o 
	rm -rf $(all)