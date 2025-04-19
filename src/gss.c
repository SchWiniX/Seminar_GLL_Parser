
#include<stdint.h>
#include<stdlib.h>
#include<string.h>
#include"gss.h"
#include"binarized_sppf.h"

gss_node* dfs(gss_node* root_node, const char* grammar_slot, int grammar_slot_pos, int in_pos) {
	if(!root_node) return NULL;
	if(
			root_node->grammar_slot_pos == grammar_slot_pos &&
			!strcmp(root_node->grammar_slot, grammar_slot) //oh nyo this is unsafe
	) return root_node;
	gss_node* node;
	for(int i = 0; i < root_node->edge_list_size; i++) {
		if((node = dfs(root_node->outgoing_edges[i]->target_node, grammar_slot, grammar_slot_pos, in_pos))) return node;
	}
	return NULL;
}

gss_node* create(gss_node* root_node, const char* grammar_slot, int grammar_slot_pos, int in_pos, sppf_node* record) {
	gss_node* ret_node;
	if((ret_node = dfs(root_node, grammar_slot, grammar_slot_pos, in_pos))) return ret_node;
	ret_node = (gss_node*) malloc(sizeof(gss_node));
	if (!ret_node) return NULL;

}
