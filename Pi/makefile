$(CXX) = g++
INC_DIR = src
CFLAGS = -I$(INC_DIR)
OBJ = obj

all: speechSynth heroSpeeches senseTest driveTest

debug: CXX += -DDEBUG -g
debug: speechSynth heroSpeeches senseTest driveTest

heroSpeeches: obj/heroSpeeches.o obj/speech.o obj/controller.o
	$(CXX) obj/heroSpeeches.o obj/speech.o obj/controller.o -lsqlite3 -o bin/heroSpeeches $(CFLAGS)

obj/heroSpeeches.o: src/Apps/heroSpeeches.cpp
	$(CXX) -c src/Apps/heroSpeeches.cpp -o $@ $(CFLAGS)	
	
	
driveTest: obj/driveTest.o obj/drive.o obj/controller.o
	$(CXX) obj/driveTest.o obj/drive.o obj/controller.o -o bin/driveTest

obj/driveTest.o: src/Apps/driveTest.cpp
	$(CXX) -c src/Apps/driveTest.cpp -o $@ $(CFLAGS)


speechSynth: obj/speechSynth.o obj/speech.o obj/controller.o
	$(CXX) obj/speechSynth.o obj/speech.o obj/controller.o -lsqlite3 -o bin/speechSynth $(CFLAGS)

obj/speechSynth.o: src/Apps/speechSynth.cpp
	$(CXX) -c src/Apps/speechSynth.cpp -o $@ $(CFLAGS)
	

senseTest: obj/senseTest.o obj/sense.o obj/controller.o
	$(CXX) obj/senseTest.o obj/sense.o obj/controller.o -lwiringPi -o bin/senseTest

obj/senseTest.o: src/Apps/senseTest.cpp
	$(CXX) -c src/Apps/senseTest.cpp -o $@ $(CFLAGS)

obj/drive.o: src/drive/drive.cpp
	$(CXX) -c src/drive/drive.cpp -o $@ $(CFLAGS)
	
obj/speech.o: src/speech/speech.cpp
	$(CXX) -c src/speech/speech.cpp -o $@ $(CFLAGS)

obj/sense.o: src/sense/sense.cpp
	$(CXX) -c src/sense/sense.cpp -o $@ $(CFLAGS)		

obj/controller.o: src/controller.cpp
	$(CXX) -c src/controller.cpp -o $@ $(CFLAGS)		

clean:
	rm -f obj/* bin/speechSynth bin/heroSpeeches bin/senseTest bin/driveTest

$(shell mkdir -p $(OBJ))
