#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#include "leven.h"
#include "parse.h"
#include "log.h"

int main(int argc, char **argv) {
	int rc;
	char *a;
	{
		int fd = open("/Users/delusional/Documents/xmldiff/file_a.xml", O_RDONLY);
		int len = lseek(fd, 0, SEEK_END);
		a = mmap(0, len, PROT_READ, MAP_PRIVATE, fd, 0);
	}

	char* b;
	{
		int fd = open("/Users/delusional/Documents/xmldiff/file_b.xml", O_RDONLY);
		int len = lseek(fd, 0, SEEK_END);
		b = mmap(0, len, PROT_READ, MAP_PRIVATE, fd, 0);
	}

	/* char* a = "<root><a/><b></b></root>"; */
	/* char* b = "<root><a><b/></a></root>"; */

	log("Parse a");
	struct Tree a_tree;
	size_t *a_chunks;
	rc = parse_string(a, &a_tree, &a_chunks);
	if(rc != 0) return 1;
	log("Tree a size %ld", a_tree.len);

	log("Parse b");
	struct Tree b_tree;
	size_t *b_chunks;
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
	for(size_t i = 0; i < 4; i++) {
		size_t a_tag_end = a_chunks[i];
		while(a[a_tag_end] != '>') a_tag_end++;
		size_t a_len = a_tag_end - a_chunks[i];
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
			size_t a_tag_end = a_chunks[i];
			while(a[a_tag_end] != '>') a_tag_end++;
			log("%ld, %ld", i, j);
			
			size_t a_len = a_tag_end - a_chunks[i];
			size_t b_len = b_tag_end - b_chunks[j];
			log("B: %ld, A: %ld", b_len, a_len);
			log("A: %.*s, B: %.*s", (int)a_len, a + a_chunks[i], (int)b_len, b + b_chunks[j]);

			*imat_uint32_t(cost, i+1, j+1) = !(a_len == b_len && memcmp(a + a_chunks[i], b + b_chunks[j], a_len) == 0) * 1;
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
