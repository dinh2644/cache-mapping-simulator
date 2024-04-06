all: cache-sim

cache-sim: main.o Cache.o 
	g++ -g main.o Cache.o -o cache-sim

main.o: main.cpp Cache.h
	g++ -g -c -g main.cpp

Cache.o: Cache.cpp Cache.h Trace.h
	g++ -g -c -g Cache.cpp

clean: 
	rm -f *.o *.txt cache-sim