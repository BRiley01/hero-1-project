$(CXX) = g++

all: speechSynth heroSpeeches

debug: CXX += -DDEBUG -g
debug: speechSynth heroSpeeches

heroSpeeches: obj/heroSpeeches.o obj/speech.o
	$(CXX) obj/heroSpeeches.o obj/speech.o -lsqlite3 -o bin/heroSpeeches
	
obj/heroSpeeches.o: src/Apps/heroSpeeches.cpp
	$(CXX) -c src/Apps/heroSpeeches.cpp -o $@	

speechSynth: obj/speechSynth.o obj/speech.o
	$(CXX) obj/speechSynth.o obj/speech.o -lsqlite3 -o bin/speechSynth

obj/speechSynth.o: src/Apps/speechSynth.cpp
	$(CXX) -c src/Apps/speechSynth.cpp -o $@
	
obj/speech.o: src/speech/speech.cpp
	$(CXX) -c src/speech/speech.cpp -o $@

clean:
	rm -f obj/* bin/speechSynth bin/heroSpeeches
