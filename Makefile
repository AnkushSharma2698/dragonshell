CC = g++
CFLAGS = -Wall -std=c++11
OBJECTS = dragonshell.o execution_handler.o
dragonshell: $(OBJECTS)
	$(CC) -o dragonshell $(OBJECTS)
dragonshell.o: dragonshell.cc execution_handler.h
	$(CC) $(CFLAGS) -c dragonshell.cc -o dragonshell.o
execution_handler.o: execution_handler.cc execution_handler.h
	$(CC) $(CFLAGS) -c execution_handler.cc -o execution_handler.o
clean:
	rm *.o $(OBJECTS)
