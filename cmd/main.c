#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/mman.h>

#include "leven.h"
#include "parse.h"
#include "log.h"

#define ACTIONS(X) \
        X(ACTION_MATCH) \
        X(ACTION_DEL_A) \
        X(ACTION_ADD_B) \
        X(ACTION_ET) \

#define ENUM_VALUE(NAME) NAME,
#define STRING_ARRAY_VALUE(NAME) #NAME,

enum Action {
    ACTIONS(ENUM_VALUE)
};

static const char *ActionNames[] = {
    ACTIONS(STRING_ARRAY_VALUE)
};

int main(int argc, char **argv) {
	int rc;
	char *a;
	{
		int fd = open("../xmldiff/file_a.xml", O_RDONLY);
		int len = lseek(fd, 0, SEEK_END);
		a = mmap(0, len, PROT_READ, MAP_PRIVATE, fd, 0);
	}

	char* b;
	{
		int fd = open("../xmldiff/file_b.xml", O_RDONLY);
		int len = lseek(fd, 0, SEEK_END);
		b = mmap(0, len, PROT_READ, MAP_PRIVATE, fd, 0);
	}

	/* char* a = "<root><a/><b></b></root>"; */
	/* char* b = "<root><a><b/></a></root>"; */

	log("Parse a");
	struct Tree a_tree;
	ssize_t *a_chunks;
	rc = parse_string(a, &a_tree, &a_chunks);
	if(rc != 0) return 1;
	log("Tree a size %ld", a_tree.len);

	log("Parse b");
	struct Tree b_tree;
	ssize_t *b_chunks;
	rc = parse_string(b, &b_tree, &b_chunks);
	if(rc != 0) return 1;
	log("Tree b size %ld", b_tree.len);

	DECL_MAT_DATA(cost, uint32_t, 5, 5,
		0, 2, 2, 2, 2,
		2, 1, 1, 1, 1,
		2, 1, 1, 1, 1,
		2, 1, 1, 0, 1,
		2, 1, 1, 1, 1
	);

	size_t *a_opens = malloc(a_tree.len * sizeof(size_t));
	size_t *b_opens = malloc(b_tree.len * sizeof(size_t));

	{
		size_t *a_cursor = a_opens;
		for(size_t i = 0; i < a_tree.len*2; i++) {
			if(a_chunks[i] >= 0) {
				*(a_cursor++) = a_chunks[i];
			}
		}
	}
	{
		size_t *b_cursor = b_opens;
		for(size_t i = 0; i < b_tree.len*2; i++) {
			if(b_chunks[i] >= 0) {
				*(b_cursor++) = b_chunks[i];
			}
		}
	}

	for(size_t i = 0; i < 4; i++) {
		size_t a_tag_end = a_opens[i];
		while(a[a_tag_end] != '>') a_tag_end++;
		size_t a_len = a_tag_end - a_opens[i];
		*imat_uint32_t(cost, i+1, 0) = a_len;
	}
	for(size_t j = 0; j < 4; j++) {
		size_t b_tag_end = b_chunks[j];
		while(b[b_tag_end] != '>') b_tag_end++;
		size_t b_len = b_tag_end - b_chunks[j];
		*imat_uint32_t(cost, 0, j+1) = b_len;
	}
	for(size_t j = 0; j < 4; j++) {
		size_t b_tag_end = b_chunks[j];
		while(b[b_tag_end] != '>') b_tag_end++;
		for(size_t i = 0; i < 4; i++) {
			size_t a_tag_end = a_opens[i];
			while(a[a_tag_end] != '>') a_tag_end++;
			log("%ld, %ld", i, j);
			
			size_t a_len = a_tag_end - a_opens[i];
			size_t b_len = b_tag_end - b_chunks[j];
			log("B: %ld, A: %ld", b_len, a_len);
			log("A: %.*s, B: %.*s", (int)a_len, a + a_opens[i], (int)b_len, b + b_chunks[j]);

			*imat_uint32_t(cost, i+1, j+1) = !(a_len == b_len && memcmp(a + a_opens[i], b + b_chunks[j], a_len) == 0) * 1;
		}
	}
	printf("\n");
	for(size_t y = 0; y < b_tree.len; y++) {
		for(size_t x = 0; x < b_tree.adj.stride; x++) {
			printf("%d ", *imat_nid(b_tree.adj, x, y));
		}
		printf("\n");
	}
	printf("\n");
	printf("\n");
	for(size_t y = 0; y < a_tree.len; y++) {
		for(size_t x = 0; x < a_tree.adj.stride; x++) {
			printf("%d ", *imat_nid(a_tree.adj, x, y));
		}
		printf("\n");
	}
	printf("\n");

	logb("2D Array %s [%ldx%d]", "cost", cost.stride, 5);
	for(uint32_t y = 0; y < 5; y++) {
		lognl();
		for(uint32_t x = 0; x < cost.stride; x++) {
			logc("%03d ", *imat_uint32_t(cost, x, y));
		}
	}
	loge();

	mat_uint32_t cost_n = {
		.data = malloc((a_tree.len+1) * (b_tree.len+1) * sizeof(uint32_t)),
		.stride = a_tree.len+1,
	};
	assert(cost_n.data != NULL);

	mat_uint32_t cost_f = {
		.data = malloc((a_tree.len+1) * (b_tree.len+1) * sizeof(uint32_t)),
		.stride = a_tree.len+1,
	};
	assert(cost_f.data != NULL);

	mat_uint32_t cost_s = {
		.data = malloc((a_tree.adj.stride+1) * (b_tree.adj.stride+1) * sizeof(uint32_t)),
		.stride = a_tree.adj.stride+1,
	};
	assert(cost_s.data != NULL);

	CTedData cted = {cost, cost_n, cost_f, cost_s};
	constrained_tree_distance(a_tree, b_tree, cted);

	log("Final Cost %d", *imat_uint32_t(cost_n, 1, 1));

	uint32_t *alignment = malloc(b_tree.len * sizeof(*alignment));
	uint32_t *adj_alignment = malloc(b_tree.adj.stride * sizeof(*adj_alignment));
	constrained_tree_alignment(
		a_tree,
		b_tree,
		cted,
		adj_alignment,
		alignment
	);

	logb("Alignment: ");
	for(size_t i = 0; i < b_tree.len; i++) {
		logc("%d ", alignment[i]);
	}
	loge();

	// @CORRECT Theres a bug here where we will erroneously match an inner
	// close with the wrong outer close. I dont quite know how we are
	// going to solve right now. I think we may have to lookup what decision
	// we made for the open tag
	// The worst case would be that we remove the whole A tree and the whole B tree
	enum Action *emit_actions = malloc((a_tree.len + b_tree.len) * 2 * sizeof(*emit_actions));
	size_t current_action = 0;
	size_t a_cursor = 0;
	size_t b_cursor = 0;
	size_t alignment_cursor = 0;

	size_t *a_close = malloc(a_tree.len * sizeof(size_t));
	size_t a_close_top = 0;
	size_t *b_close = malloc(b_tree.len * sizeof(size_t));
	size_t b_close_top = 0;

	while(a_cursor < a_tree.len && b_cursor < b_tree.len) {
		if(alignment[alignment_cursor] == 0) {
			alignment_cursor++;
			emit_actions[current_action++] = ACTION_ADD_B;

			b_close[b_close_top] = 0;
			while(b_close[b_close_top] < b_tree.adj.stride && *imat_nid(b_tree.adj, b_close[b_close_top], b_cursor) != 0)
				b_close[b_close_top]++;
			b_close_top++;

			b_cursor++;
		} else if(a_cursor + 1 < alignment[alignment_cursor]) {
			emit_actions[current_action++] = ACTION_DEL_A;

			a_close[a_close_top] = 0;
			while(a_close[a_close_top] < a_tree.adj.stride && *imat_nid(a_tree.adj, a_close[a_close_top], a_cursor) != 0)
				a_close[a_close_top]++;
			a_close_top++;

			a_cursor++;
		} else {
			alignment_cursor++;
			emit_actions[current_action++] = ACTION_MATCH;

			b_close[b_close_top] = 0;
			while(b_close[b_close_top] < b_tree.adj.stride && *imat_nid(b_tree.adj, b_close[b_close_top], b_cursor) != 0)
				b_close[b_close_top]++;
			b_close_top++;

			a_close[a_close_top] = 0;
			while(a_close[a_close_top] < a_tree.adj.stride && *imat_nid(a_tree.adj, a_close[a_close_top], a_cursor) != 0)
				a_close[a_close_top]++;
			a_close_top++;

			a_cursor++;
			b_cursor++;
		}

		while(a_close[a_close_top-1] == 0 || b_close[b_close_top-1] == 0) {
			if(a_close[a_close_top-1] == 0 && b_close[b_close_top-1] == 0) {
				a_close_top--;
				a_close[a_close_top-1]--;
				b_close_top--;
				b_close[b_close_top-1]--;

				emit_actions[current_action++] = ACTION_MATCH;
			} else if(a_close[a_close_top-1] == 0) {
				a_close_top--;
				a_close[a_close_top-1]--;

				emit_actions[current_action++] = ACTION_DEL_A;
			} else if(b_close[b_close_top-1] == 0) {
				b_close_top--;
				b_close[b_close_top-1]--;

				emit_actions[current_action++] = ACTION_ADD_B;
			}
		}
	}

	logb("Actions: ");
	for(size_t i = 0; i < current_action; i++) {
		lognl();
		logc("    %s", ActionNames[emit_actions[i]]);
	}
	loge();
	
	b_cursor = 0;
	a_cursor = 0;

	for(size_t i = 0; i < current_action; i++) {
		size_t bc_len = (b_cursor+1 >= b_tree.len*2 ? strlen(b) : b_chunks[b_cursor+1]) - b_chunks[b_cursor];
		size_t ac_len = (a_cursor+1 >= a_tree.len*2 ? strlen(a) : a_chunks[a_cursor+1]) - a_chunks[a_cursor];
		if(emit_actions[i] == ACTION_MATCH) {
			fwrite(b + b_chunks[b_cursor], bc_len, 1, stdout);
			a_cursor++;
			b_cursor++;
		} else if(emit_actions[i] == ACTION_ADD_B) {
			fprintf(stdout, "{+");
			fwrite(b + b_chunks[b_cursor], bc_len, 1, stdout);
			fprintf(stdout, "+}");
			b_cursor++;
		} else if(emit_actions[i] == ACTION_DEL_A) {
			fprintf(stdout, "{-");
			fwrite(a + a_chunks[a_cursor], ac_len, 1, stdout);
			fprintf(stdout, "-}");
			a_cursor++;
		} else {
			abort();
		}
	}

	return 0;
}
