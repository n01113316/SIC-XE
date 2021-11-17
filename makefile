project2:	main.o symbols.o  directives.o opcodes.o records.o headers.h
	gcc -o project2 -Wall -g3 -O0 main.o symbols.o directives.o opcodes.o records.o

main.o:		main.c headers.h
	gcc -c -Wall -g3  -O0 main.c

symbols.o:	symbols.c headers.h
	gcc -c -Wall -g3 -O0 symbols.c

directives.o:	directives.c headers.h
	gcc -c -Wall -g3 -O0 directives.c

opcode.o:	opcodes.c headers.h
	gcc -c -Wall -g3 -O0 opcodes.c

records.o:	records.c headers.h
	gcc -c -Wall -g3 -O0 records.c

clean:
	rm *.o -f
	touch *.c
	rm project2 -f
