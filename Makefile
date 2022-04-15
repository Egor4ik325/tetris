main: main.c
	clang -lncurses -o main main.c

run: main
	./main

debug: main.c
	clang -lncurses -o main -g main.c
	lldb ./main

clean:
	rm -r main main.dSYM
