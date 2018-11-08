CC=g++

daemon: daemon.cpp
	$(CC) -o daemon daemon.cpp script.h script.cpp
clean:
	rm -f daemon
	rm -f *.o
