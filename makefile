build:
	mkdir -p builds/
	gcc -Wall -O1 -o builds/gll_parser src/main.c src/gll_parser.c src/descriptor_set_functions.c src/gss.c src/grammer_handler.c -g

clean:
	rm builds/*

test:
	mkdir -p builds/
	gcc -Wall -O1 -o builds/gll_parser_test src/test.c src/gll_parser.c src/descriptor_set_functions.c src/gss.c src/grammer_handler.c -g
	gcc -Wall -O1 -o builds/gll_parser_inf_test src/infinit_test.c src/gll_parser.c src/descriptor_set_functions.c src/gss.c src/grammer_handler.c -g
