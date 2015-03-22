$(CXX) = g++

all: speechSynth

debug: CXX += -DDEBUG -g
debug: speechSynth

speechSynth: speechSynth.o speech.o
	$(CXX) speechSynth.o speech.o -lsqlite3 -o speechSynth

speechSynth.o: speechSynth.cpp
	$(CXX) -c speechSynth.cpp
	
speech.o: speech.cpp
	$(CXX) -c speech.cpp

clean:
	rm -rf *o speechSynth
