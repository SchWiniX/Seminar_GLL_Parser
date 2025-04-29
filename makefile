build:
	mkdir -p builds/
	#gcc -Wall -O0 -o builds/gll_parser_O0 src/main.c src/descriptor_set_functions.c src/gss.c src/grammer_handler.c
	gcc -Wall -O1 -o builds/gll_parser src/main.c src/descriptor_set_functions.c src/gss.c src/grammer_handler.c
	#gcc -Wall -O2 -o builds/gll_parser_O2 src/main.c src/descriptor_set_functions.c src/gss.c src/grammer_handler.c
	#gcc -Wall -O3 -o builds/gll_parser_O3 src/main.c src/descriptor_set_functions.c src/gss.c src/grammer_handler.c

clean:
	rm builds/*

test:
	mkdir -p builds/
	echo "no test implemented yet"
