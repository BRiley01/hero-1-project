all: speechMaster

speechMaster: speechMaster.o
	g++ speechMaster.o -o speechMaster

speechMaster.o: speechMaster.cpp
	g++ -c speechMaster.cpp 

clean:
	rm -rf *o speechMaster
