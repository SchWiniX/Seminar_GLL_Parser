build:
	mkdir -p builds/
	gcc -Wall -o builds/gll_parser src/main.c src/descriptor_set_functions.c src/gss.c src/grammer_handler.c

clean:
	rm builds/*

test:
	mkdir -p builds/
	echo "no test implemented yet"
