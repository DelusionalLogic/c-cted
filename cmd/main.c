#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

#include "leven.h"
#include "parse.h"

int main(int argc, char **argv) {
	int rc;
	/* int fd = open("filename", O_RDONLY); */
	/* int len = lseek(fd, 0, SEEK_END); */
	/* void *data = mmap(0, len, PROT_READ, MAP_PRIVATE, fd, 0); */

	char* a = "<root><a/><b></b></root>";
	char* b = "<root><a><b/></a></root>";

	struct Tree a_tree;
	size_t *a_chunks;
	rc = parse_string(a, &a_tree, &a_chunks);
	if(rc != 0) return 1;
	printf("%ld\n", a_tree.len);

	struct Tree b_tree;
	size_t *b_chunks;
	rc = parse_string(b, &b_tree, &b_chunks);
	if(rc != 0) return 1;
	printf("%ld\n", b_tree.len);

	DECL_MAT_DATA(cost, uint32_t, 4, 4,
		0, 2, 2, 2,
		2, 0, 1, 1,
		2, 1, 0, 1,
		2, 1, 1, 0,
	);
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
	/* mat_uint32_t cost = { */
	/* 	.data = malloc(a_tree.len * b_tree.len * sizeof(uint32_t)), */
	/* 	.stride = a_tree.len, */
	/* }; */
	/* assert(cost.data != NULL); */

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

	constrained_tree_distance(a_tree, b_tree, cost, cost_n, cost_f, cost_s);

	printf("COST_F\n");
	for(size_t y = 0; y < b_tree.len+1; y++) {
		for(size_t x = 0; x < cost.stride; x++) {
			printf("%d ", *imat_uint32_t(cost_f, x, y));
		}
		printf("\n");
	}
	printf("\n");
	printf("COST_N\n");
	for(size_t y = 0; y < b_tree.len+1; y++) {
		for(size_t x = 0; x < cost.stride; x++) {
			printf("%d ", *imat_uint32_t(cost_n, x, y));
		}
		printf("\n");
	}
	printf("\n");

	printf("%d\n", *imat_uint32_t(cost_n, 1, 1));

	DECL_MAT(alignment, uint32_t, 2, 2);
	uint32_t adj_alignment[2] = {0, 0};
	constrained_tree_alignment(
		a_tree,
		b_tree,
		cost,
		cost_n,
		cost_f,
		cost_s,
		adj_alignment,
		alignment
	);

	printf("Hello sailor\n");
	return 0;
}
