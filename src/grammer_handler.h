#include <stdint.h>

typedef struct {
	char name;
	char** blocks;
} rule;

rule* create_rule(char name);

void add(rule* r, char* block);


