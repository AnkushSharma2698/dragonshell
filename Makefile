CC = g++
CFLAGS = -Wall -std=c++11
OBJECTS = dragonshell.o
dragonshell: $(OBJECTS)
	$(CC) -o dragonshell $(OBJECTS)
dragonshell.o: dragonshell.cc
	$(CC) $(CFLAGS) -c dragonshell.cc -o dragonshell.o
clean:
	rm *.o $(OBJECTS)
