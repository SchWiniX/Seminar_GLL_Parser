build:
	mkdir -p builds/
	echo "#undef DEBUG" > src/debug.h
	gcc -Wall -O2 -o builds/gll_parser src/main.c src/gll_parser.c src/descriptor_set_functions.c src/gss.c src/grammer_handler.c -pg -g

debug:
	mkdir -p builds/
	echo "#define DEBUG" > src/debug.h
	gcc -Wall -O1 -o builds/gll_parser src/main.c src/gll_parser.c src/descriptor_set_functions.c src/gss.c src/grammer_handler.c -g

clean:
	rm builds/*

test:
	mkdir -p builds/
	echo "#undef DEBUG" > src/debug.h
	gcc -Wall -O2 -o builds/gll_parser_test src/test.c src/gll_parser.c src/descriptor_set_functions.c src/gss.c src/grammer_handler.c
	gcc -Wall -O2 -o builds/gll_parser_inf_test src/infinit_test.c src/gll_parser.c src/descriptor_set_functions.c src/gss.c src/grammer_handler.c


generator:
	mkdir -p builds/
	echo "#undef DEBUG" > src/debug.h
	#echo "#define DEBUG" > src/debug.h
	gcc -Wall -O1 -o builds/input_gen src/input_gen.c src/grammer_handler.c -g
