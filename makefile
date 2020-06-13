CC = gcc
CFLAGS = -pedantic -I .

SERVER = obj/server.o obj/sockets.o obj/picoHTTPparser.o obj/daemon.o


all: objects obj/server.o obj/sockets.o obj/picoHTTPparser.o obj/daemon.o turbo-server 

turbo-server: $(SERVER)
	$(CC) $^ -o $@ $(LINK)

objects:
	rm -rf obj
	mkdir obj

obj/server.o: src/server.c include/sockets.h
	$(CC) $(CFLAGS) -c -o $@ $<

obj/sockets.o: src/sockets.c include/sockets.h
	$(CC) $(CFLAGS) -c -o $@ $<

obj/picoHTTPparser.o: src/picoHTTPparser.c include/picoHTTPparser.h
	$(CC) $(CFLAGS) -c -o $@ $<

obj/daemon.o: src/daemon.c include/daemon.h
	$(CC) $(CFLAGS) -c -o $@ $<

run:
	./turbo-server 1200 5

kill:
	killall turbo-server

clean:
	rm -f turbo-server
	rm -f *.o
	rm -R obj
