#ifndef GSS
#define GSS

#include "info_struct.h"


int print_gss_info(rule rules[], struct gss_info* gss_info, struct input_info* input_info, const uint8_t rule_count);

gss_node_idx create(
		struct rule_info* rule_info,
		struct input_info* input_info,
		struct gss_info* gss_info,
		struct set_info* set_info,
		uint8_t label_type
		);

uint32_t pop(
		struct rule_info* rule_info,
		const struct input_info* input_info,
		struct gss_info* gss_info,
		struct set_info* set_info
		);


uint64_t get_gss_node_count(const struct gss_info* gss_info, uint32_t rule_count, uint32_t input_size);
uint64_t get_gss_edge_count(const struct gss_info* gss_info, uint32_t rule_count, uint32_t input_size);
uint64_t get_gss_total_alloc_size(const struct gss_info* gss_info, uint32_t rule_count, uint32_t input_size);
uint64_t get_u_set_total_size(const struct gss_info* gss_info, uint32_t rule_count, uint32_t input_size);
uint64_t get_p_set_total_size(const struct gss_info* gss_info, uint32_t rule_count, uint32_t input_size);

gss_node** init_gss(uint32_t rule_count, uint32_t input_size);
gss_node* init_gss_node(uint16_t edge_arr_size, uint16_t u_size);
gss_node* realloc_gss_node_edge_array(gss_node* node);
gss_node* realloc_gss_node_u_set(gss_node* node);

int free_gss(gss_node** gss, uint32_t rule_count, uint32_t input_size);

#endif
