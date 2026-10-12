CC := gcc
CFLAGS := -Wall -Wextra -std=c11 -g -O2

compiled/lexer.o: libs/lexer/lexer.c libs/lexer/lexer.h
	$(CC) $(CFLAGS) -c $< -o $@

compiled/lexerDebug.o: libs/lexer/lexerDebug.c libs/lexer/lexerDebug.h
	$(CC) $(CFLAGS) -c $< -o $@

compiled/main.o: main.c libs/includes.h
	$(CC) $(CFLAGS) -c $< -o $@

compiled/hashmap.o: libs/utils/hashmap.c libs/utils/hashmap.h
	$(CC) $(CFLAGS) -c $< -o $@

compiled/parser.o: libs/parser/parser.c libs/parser/parser.h
	$(CC) $(CFLAGS) -c $< -o $@

app: compiled/lexer.o compiled/lexerDebug.o compiled/main.o compiled/hashmap.o compiled/parser.o
	$(CC) $^ -o $@

clean:
	rm -rf ./compiled/*
	rm app

runscript:
	./app ./script.hasan
