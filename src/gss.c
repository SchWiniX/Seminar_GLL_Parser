
#include<stdint.h>
#include<stdlib.h>
#include<string.h>
#include"gss.h"
#include"binarized_sppf.h"

gss_node* create(gss_node root_node[], const char* grammar_slot, int grammar_slot_pos, int in_pos, sppf_node* record) {
	gss_node* ret_node;
	if((ret_node = dfs(root_node, grammar_slot, grammar_slot_pos, in_pos))) return ret_node;
	ret_node = (gss_node*) malloc(sizeof(gss_node));
	if (!ret_node) return NULL;

}
