$(CXX) = g++

all: speechSynth

debug: CXX += -DDEBUG -g
debug: speechSynth

speechSynth: obj/speechSynth.o obj/speech.o
	$(CXX) obj/speechSynth.o obj/speech.o -lsqlite3 -o bin/speechSynth

obj/speechSynth.o: src/Apps/speechSynth.cpp
	$(CXX) -c src/Apps/speechSynth.cpp -o $@
	
obj/speech.o: src/speech/speech.cpp
	$(CXX) -c src/speech/speech.cpp -o $@

clean:
	rm -f obj/* bin/speechSynth
