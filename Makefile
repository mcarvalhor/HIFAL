
CC_FLAGS = -I headers




all: hifal

hifal: obj obj/main.o obj/hifal.o
	gcc -o hifal obj/*.o $(CC_FLAGS)

obj/main.o: src/main.c headers/hifal.h
	gcc -o obj/main.o -c src/main.c $(CC_FLAGS)

obj/hifal.o: src/hifal.c headers/hifal.h
	gcc -o obj/hifal.o -c src/hifal.c $(CC_FLAGS)

obj:
	mkdir obj


run: hifal
	./hifal wwwroot 8080


clean:
	rm -R ./obj/ ./hifal
