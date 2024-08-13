LIBS = libavcodec libavformat libavutil

C_FLAGS = $(shell pkg-config --cflags $(LIBS))
L_FLAGS = $(shell pkg-config --libs $(LIBS))
D_FLAGS = -g

main: main.o
	gcc $(L_FLAGS) -o $@ $^

main.o: src/main.c
	gcc $(C_FLAGS) -c $< -o $@

debug: main.d.o
	gcc $(L_FLAGS) -o $@ $^

main.d.o: src/main.c
	gcc $(C_FLAGS) $(D_FLAGS) -c $< -o $@

clean:
	rm -rf *.o main debug *.pgm