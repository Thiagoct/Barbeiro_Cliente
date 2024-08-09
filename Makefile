CC = gcc
CFLAGS = -Wall -pthread

all: barbeiros_dorminhocos

produtor_consumidor: barbeiros_dorminhocos.c
	$(CC) $(CFLAGS) -o barbeiros_dorminhocos barbeiros_dorminhocos.c

clean:
	rm -f barbeiros_dorminhocos

