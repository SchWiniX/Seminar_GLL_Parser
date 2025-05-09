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

struct str_pool {
	struct str_gen* str_buff;
	uint32_t size;
	uint32_t alloc_size;
};

int get_from_pool(struct str_pool* str_pool, struct str_ringbuff* str_ringbuff, uint32_t new_size);

int realloc_ringbuff(struct str_ringbuff* str_ringbuff);
