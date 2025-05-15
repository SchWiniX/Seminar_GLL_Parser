#include <stdint.h>

struct str_gen {
	char* str;
	uint32_t size;
	uint32_t alloc_size;
	uint32_t current_position;
};

struct str_ringbuff {
	struct str_gen* ringbuff;
	uint32_t size;
	uint32_t alloc_size;
	uint32_t dequeue_idx;
	uint32_t queue_idx;
};

int realloc_ringbuff(struct str_ringbuff* str_ringbuff);
