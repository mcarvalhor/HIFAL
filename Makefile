
CC = gcc
CC_FLAGS = -I headers -Os




all: hifal

hifal: obj obj/main.o obj/hifal.o obj/mimes.o
	$(CC) -o hifal obj/*.o $(CC_FLAGS)

obj/main.o: src/main.c headers/hifal.h
	$(CC) -o obj/main.o -c src/main.c $(CC_FLAGS)

obj/hifal.o: src/hifal.c headers/hifal.h headers/mimes.h
	$(CC) -o obj/hifal.o -c src/hifal.c $(CC_FLAGS)

obj/mimes.o: src/mimes.c headers/mimes.h
	$(CC) -o obj/mimes.o -c src/mimes.c $(CC_FLAGS)

obj:
	mkdir obj


run: hifal
	./hifal wwwroot 8080


clean:
	rm -R ./obj/ ./hifal
