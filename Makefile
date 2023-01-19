CC = g++
CFLAGS = `wx-config --cxxflags` -Wno-c++11-extensions -std=c++11
CLIBS = `wx-config --libs` -Wno-c++11-extensions -std=c++11
OBJ = hexer.o editView.o docsView.o hexView.o dialogs.o rom.o romEditor.o

hexer: $(OBJ)
	$(CC) -o hexer $(OBJ) $(CLIBS)

hexer.o: hexer.cpp hexer.h
	$(CC) -c hexer.cpp $(CFLAGS)

editView.o: editView.cpp hexer.h
	$(CC) -c editView.cpp $(CFLAGS)

docsView.o: docsView.cpp hexer.h
	$(CC) -c docsView.cpp $(CFLAGS)

hexView.o: hexView.cpp hexer.h
	$(CC) -c hexView.cpp $(CFLAGS)

dialogs.o: dialogs.cpp hexer.h
	$(CC) -c dialogs.cpp $(CFLAGS)

rom.o: rom.cpp rom.h
	$(CC) -c rom.cpp $(CFLAGS)

romEditor.o: romEditor.cpp romEditor.h
	$(CC) -c romEditor.cpp $(CFLAGS)

.PHONY: clean
clean:
	-rm hexer $(OBJ)