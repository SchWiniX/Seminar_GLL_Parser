#include <sys/types.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>

#include "grammer_handler.h"
#include "info_struct.h"
#include "input_gen.h"
#include "debug.h"

void print_help() {
	printf("usage: input_gen 'grammer_file' [count]\n\n");
	printf(" - 'folder' contains files containing grammers and input of format:\n");
	printf("\tX -> X_1 | X_2 | ...\n");
	printf("\tY -> Y_1 | Y_2 | ...\n");
	printf("\t...\n");
	printf("\t#\n");
	printf(" - [count] is a natural number denoting the total amount of input generated. Will not terminate for -1 which is the default value\n");
	exit(1);
}

int print_ringbuff(const struct str_ringbuff* str_ringbuff) {
	printf("\n{ ");
	for(int i = str_ringbuff->dequeue_idx; i != str_ringbuff->queue_idx; i = (i + 1) % str_ringbuff->alloc_size) {
		printf("(%d: %s, %d, %d), ", i, str_ringbuff->ringbuff[i].str, str_ringbuff->ringbuff[i].current_position, str_ringbuff->ringbuff[i].size);
	}
	printf(" }\n");
	return 0;
}

int realloc_ringbuff(struct str_ringbuff* str_ringbuff) {
	assert(str_ringbuff);

	struct str_gen* buff = malloc(2 * str_ringbuff->alloc_size * sizeof(struct str_gen));
	assert(buff);

	uint32_t i = 0;
	uint32_t j = str_ringbuff->dequeue_idx;
	//the first set needs to be unrolled to avoid triggering the while condition instandly
	buff[i++] = str_ringbuff->ringbuff[j];
	j = (j + 1) % str_ringbuff->alloc_size;
	while(j != str_ringbuff->queue_idx) {
		buff[i++] = str_ringbuff->ringbuff[j];
		j = (j + 1) % str_ringbuff->alloc_size;
	}

	free(str_ringbuff->ringbuff);
	str_ringbuff->ringbuff = buff;
	str_ringbuff->dequeue_idx = 0;
	str_ringbuff->queue_idx = i;
	str_ringbuff->alloc_size *= 2;
	return 0;
}

int free_ringbuff(struct str_ringbuff* str_ringbuff) {
	assert(str_ringbuff);
	assert(str_ringbuff->ringbuff);
	for(uint32_t i = str_ringbuff->dequeue_idx; i != str_ringbuff->queue_idx; i = (i + 1) % str_ringbuff->alloc_size) {
		free(str_ringbuff->ringbuff[i].str);
	}
	free(str_ringbuff->ringbuff);
	return 0;
}

int main(int argc, char *argv[]) {
	int64_t count = 0;
	switch (argc) {
		case 2:
			count = -1;
			break;
		case 3:
			if(!(count = atoi(argv[2]))) {
				printf("'count' must be a natural number or '-1'\n");
				exit(1);
			}
			break;
		default:
			print_help();
	}

	FILE* grammer_file = fopen(argv[1], "r");
	assert(grammer_file);

	//parse grammer input
	rule rules[26];
	for(int i = 0; i < 26; i++) {
		rules[i].name = '\0';
	}
	create_grammer(rules, grammer_file);
	fclose(grammer_file);

	//print the rules
	for(int i = 0; i < 26; i++) {
		if(rules[i].name != i + 'A') continue;
		printf("%c -> ", i + 'A');
		for(int j = 0; j < rules[i].number_of_alternatives; j++) {
			uint16_t start_idx = rules[i].alternative_sizes[j];
			uint16_t end_idx = rules[i].alternative_sizes[j + 1];
			for(; start_idx < end_idx; start_idx++) {
				printf("%c", rules[i].alternatives[start_idx]);
			}
			if(j + 1 < rules[i].number_of_alternatives) 
				printf(" | ");
			else
				printf("\n");
		}
	}
	printf("#\n");

	uint32_t init_ringbuff_alloc_size = 128;
	uint32_t init_str_alloc_size = 64;
	struct str_ringbuff str_ringbuff = {
		.ringbuff = malloc(init_ringbuff_alloc_size * sizeof(struct str_gen)),
		.size = 0,
		.alloc_size = init_ringbuff_alloc_size,
		.dequeue_idx = 0,
		.queue_idx = 0,
	};
	assert(str_ringbuff.ringbuff);


	//setup starting point
	str_ringbuff.ringbuff[str_ringbuff.queue_idx].str = malloc(init_str_alloc_size);
	assert(str_ringbuff.ringbuff[str_ringbuff.queue_idx].str);

	str_ringbuff.ringbuff[str_ringbuff.queue_idx].str[0] = 'S';
	str_ringbuff.ringbuff[str_ringbuff.queue_idx].str[1] = '\0';
	str_ringbuff.ringbuff[str_ringbuff.queue_idx].size = 1;
	str_ringbuff.ringbuff[str_ringbuff.queue_idx].alloc_size = init_str_alloc_size;
	str_ringbuff.ringbuff[str_ringbuff.queue_idx].current_position = 0;

	str_ringbuff.size = 1;
	str_ringbuff.queue_idx = (str_ringbuff.queue_idx + 1) % str_ringbuff.alloc_size;

	//main loop
	while(str_ringbuff.size > 0 && count) {
		struct str_gen curr_gen = str_ringbuff.ringbuff[str_ringbuff.dequeue_idx];
		uint8_t done = 1;
		uint32_t i = 0;
		if(curr_gen.size == 0) {
			printf(" 1\n");
			str_ringbuff.dequeue_idx = (str_ringbuff.dequeue_idx + 1) % str_ringbuff.alloc_size;
			str_ringbuff.size -= 1;
			if(count > 0) count -= 1;
			continue;
		}
		i = curr_gen.current_position % curr_gen.size;

#ifdef DEBUG
		print_ringbuff(&str_ringbuff);
#endif

		if(is_non_terminal(curr_gen.str[i])) {
			done = 0;
		} else {
			for(i = (i + 1) % curr_gen.size; i != curr_gen.current_position % curr_gen.size && done; i = (i + 1) % curr_gen.size) {
				if(is_non_terminal(curr_gen.str[i])) {
					done = 0;
					break;
				}
			}
		}
		if(done) {
			//move to the next loop
			printf("%s 1\n", str_ringbuff.ringbuff[str_ringbuff.dequeue_idx].str);
			free(str_ringbuff.ringbuff[str_ringbuff.dequeue_idx].str);
			str_ringbuff.dequeue_idx = (str_ringbuff.dequeue_idx + 1) % str_ringbuff.alloc_size;
			str_ringbuff.size -= 1;
			if(count > 0) count -= 1;
			continue;
		}
		//expand this str_gen for each alternative in the found nonterminal
		rule this_rule = rules[curr_gen.str[i] - 'A'];
		for(uint16_t j = 0; j < this_rule.number_of_alternatives; j++) {
			uint16_t start_idx = this_rule.alternative_sizes[j];
			uint16_t end_idx = this_rule.alternative_sizes[j + 1];

			uint32_t new_size;
			if(str_ringbuff.size >= str_ringbuff.alloc_size) {
				realloc_ringbuff(&str_ringbuff);
			}
			if(this_rule.alternatives[start_idx] == '_') {
				new_size = curr_gen.size - 1;
				str_ringbuff.ringbuff[str_ringbuff.queue_idx].str = malloc(2 * new_size + 1);
				assert(str_ringbuff.ringbuff[str_ringbuff.queue_idx].str);
				str_ringbuff.ringbuff[str_ringbuff.queue_idx].alloc_size = 2 * new_size + 1;

				//copy the str but remove the nulled non-terminal
				uint32_t idx = 0;
				if(curr_gen.size == 1) {
					str_ringbuff.ringbuff[str_ringbuff.queue_idx].str[0] = '\0';
				} else {
					for(uint32_t k = 0; k <= curr_gen.size;) {
						if(k == i) {
							k += 1;
							continue;
						}
						str_ringbuff.ringbuff[str_ringbuff.queue_idx].str[idx++] = curr_gen.str[k++];
					}
				}
				str_ringbuff.ringbuff[str_ringbuff.queue_idx].current_position = i;
			} else {
				new_size = curr_gen.size - 1 + end_idx - start_idx;
				str_ringbuff.ringbuff[str_ringbuff.queue_idx].str = malloc(2 * new_size + 1);
				assert(str_ringbuff.ringbuff[str_ringbuff.queue_idx].str);
				str_ringbuff.ringbuff[str_ringbuff.queue_idx].alloc_size = 2 * new_size + 1;

				uint32_t idx = 0;
				for(uint32_t k = 0; k <= curr_gen.size;) {
					if(k != i) {
						str_ringbuff.ringbuff[str_ringbuff.queue_idx].str[idx++] = curr_gen.str[k++];
						continue;
					}
					k += 1;
					for(uint32_t k1 = start_idx; k1 < end_idx; k1++)
						str_ringbuff.ringbuff[str_ringbuff.queue_idx].str[idx++] = this_rule.alternatives[k1];
				}
				str_ringbuff.ringbuff[str_ringbuff.queue_idx].current_position = i + end_idx - start_idx;
			}
			str_ringbuff.ringbuff[str_ringbuff.queue_idx].size = new_size;
			str_ringbuff.queue_idx = (str_ringbuff.queue_idx + 1) % str_ringbuff.alloc_size;
			str_ringbuff.size += 1;
		}
		free(str_ringbuff.ringbuff[str_ringbuff.dequeue_idx].str);
		str_ringbuff.dequeue_idx = (str_ringbuff.dequeue_idx + 1) % str_ringbuff.alloc_size;
		str_ringbuff.size -= 1;
	}
	free_ringbuff(&str_ringbuff);
	if(free_rules(rules)) {
		printf("failed to free rules likely a memory leak\n");
	}
	return 0;
}
