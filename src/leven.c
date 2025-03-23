#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "leven.h"
#include "log.h"

#if 1
#define debug2D(array, len) debug2D_impl(#array, array, len)
static void debug2D_impl(char* name, mat_uint32_t mat, uint32_t len) {
	logb("2D Array %s [%ldx%d]", name, mat.stride, len);
	for(uint32_t y = 0; y < len; y++) {
		lognl();
		for(uint32_t x = 0; x < mat.stride; x++) {
			logc("%03d ", *imat_uint32_t(mat, x, y));
		}
	}
	loge();
}
#endif

static inline int min(uint32_t a, uint32_t b) {
	return (a < b) ? a : b;
}

void string_edit_distance(const nid *a, size_t la, const nid *b, size_t lb, const mat_uint32_t weight, mat_uint32_t cost) {
	*imat_uint32_t(cost, 0, 0) = 0;

	for (int i = 1; i <= la; i++) {
		nid ac = a[i-1];
		assert(ac != 0);
		*imat_uint32_t(cost, i, 0) = *imat_uint32_t(cost, i-1, 0) + *imat_uint32_t(weight, ac, 0);
	}

	for (int i = 1; i <= lb; i++) {
		nid bc = b[i-1];
		assert(bc != 0);
		*imat_uint32_t(cost, 0, i) = *imat_uint32_t(cost, 0, i-1) + *imat_uint32_t(weight, 0, bc);
	}

	for(int j = 1; j <= lb; j++) {
		for(int i = 1; i <= la; i++) {
			nid ac = a[i-1];
			assert(ac != 0);
			nid bc = b[j-1];
			assert(bc != 0);

			// Remove
			*imat_uint32_t(cost, i, j) =     *imat_uint32_t(cost, i-1, j  ) + *imat_uint32_t(weight, ac,  0);
			// Add
			*imat_uint32_t(cost, i, j) = min(*imat_uint32_t(cost, i  , j-1) + *imat_uint32_t(weight,  0, bc), *imat_uint32_t(cost, i, j));
			// Replace
			*imat_uint32_t(cost, i, j) = min(*imat_uint32_t(cost, i-1, j-1) + *imat_uint32_t(weight, ac, bc), *imat_uint32_t(cost, i, j));
		}
	}
	debug2D(cost, lb+1);
}

void string_edit_alignment(const nid *a, size_t la, const nid *b, size_t lb, const mat_uint32_t weight, const mat_uint32_t cost, uint32_t *alignment) {
	int i = la;
	int j = lb;
	while(j > 0) {
		// The order of these operations dictate our preference if we have multiple equivalent answers
		if(i == 0) {
			// If we run out of characters the rest must be added
			alignment[j-1] = -1;
			j--;
		} else if(*imat_uint32_t(cost, i, j) == *imat_uint32_t(cost, i-1, j-1) + *imat_uint32_t(weight, a[i-1], b[j-1])) {
			alignment[j-1] = i-1;
			i--;
			j--;
		} else if(*imat_uint32_t(cost, i, j) == *imat_uint32_t(cost, i-1,   j) + *imat_uint32_t(weight, a[i-1],      0)) {
			i--;
		} else if(*imat_uint32_t(cost, i, j) == *imat_uint32_t(cost, i  , j-1) + *imat_uint32_t(weight,      0, b[j-1])) {
			alignment[j-1] = -1;
			j--;
		} else {
			abort();
		}
	}
}

void constrained_tree_distance(
	const struct Tree a,
	const struct Tree b,
	const mat_uint32_t cost, // The cost matrix to map a node from a (x-axis) to a node from b (y-axis)
	mat_uint32_t cost_n, // The resulting computed cost matrixes node and forest. Size a.len x b.len
	mat_uint32_t cost_f,
	mat_uint32_t cost_s // Scratch space to calculate the edit distance between subtrees. Size a.adj.stride x b.adj.stride.
) {
	*imat_uint32_t(cost_n, 0, 0) = 0;
	*imat_uint32_t(cost_f, 0, 0) = 0;

	// Outer edges is creating/deleting the node
	for(size_t i = a.len; i > 0; i--) {
		*imat_uint32_t(cost_f, i, 0) = 0;
		for(size_t j = 0; j < a.adj.stride && *imat_nid(a.adj, j, i-1) != 0; j++) {
			*imat_uint32_t(cost_f, i, 0) += *imat_uint32_t(cost_n, *imat_nid(a.adj, j, i-1), 0);
		}
		*imat_uint32_t(cost_n, i, 0) = *imat_uint32_t(cost_f, i, 0) + *imat_uint32_t(cost, i, 0);
	}

	for(size_t i = b.len; i > 0; i--) {
		*imat_uint32_t(cost_f, 0, i) = 0;
		for(size_t j = 0; j < b.adj.stride && *imat_nid(b.adj, j, i-1) != 0; j++) {
			*imat_uint32_t(cost_f, 0, i) += *imat_uint32_t(cost_n, 0, *imat_nid(b.adj, j, i-1));
		}
		*imat_uint32_t(cost_n, 0, i) = *imat_uint32_t(cost_f, 0, i) + *imat_uint32_t(cost, 0, i);
	}

	for(size_t j = b.len; j > 0; j--) {
		size_t b_adj_len = 0;
		while(b_adj_len < b.adj.stride && *imat_nid(b.adj, b_adj_len, j-1) != 0) b_adj_len++;

		for(size_t i = a.len; i > 0; i--) {
			size_t a_adj_len = 0;
			while(a_adj_len < a.adj.stride && *imat_nid(a.adj, a_adj_len, i-1) != 0) a_adj_len++;

			string_edit_distance(imat_nid(a.adj, 0, i-1), a_adj_len, imat_nid(b.adj, 0, j-1), b_adj_len, cost_n, cost_s);
			uint32_t min_cost = *imat_uint32_t(cost_s, a_adj_len, b_adj_len);

			if(a_adj_len > 0) {
				uint32_t temp_min = UINT32_MAX;
				for(uint32_t k = 0; k < a_adj_len; k++) {
					uint32_t cost = *imat_uint32_t(cost_f, *imat_nid(a.adj, k, i-1), j) - *imat_uint32_t(cost_f, *imat_nid(a.adj, k, i-1), 0);
					if(temp_min > cost)
						temp_min = cost;
				}
				min_cost = min(min_cost, *imat_uint32_t(cost_f, i, 0) + temp_min);
			}

			if(b_adj_len > 0) {
				uint32_t temp_min = UINT32_MAX;
				for(uint32_t k = 0; k < b_adj_len; k++) {
					uint32_t cost = *imat_uint32_t(cost_f, i, *imat_nid(b.adj, k, j-1)) - *imat_uint32_t(cost_f, 0, *imat_nid(b.adj, k, j-1));
					if(temp_min > cost)
						temp_min = cost;
				}
				min_cost = min(min_cost, *imat_uint32_t(cost_f, 0, j) + temp_min);
			}

			*imat_uint32_t(cost_f, i, j) = min_cost;

			min_cost = *imat_uint32_t(cost_f, i, j) + *imat_uint32_t(cost, i, j);

			if(a_adj_len > 0) {
				uint32_t temp_min = UINT32_MAX;
				for(uint32_t k = 0; k < a_adj_len; k++) {
					uint32_t cost = *imat_uint32_t(cost_n, *imat_nid(a.adj, k, i-1), j) - *imat_uint32_t(cost_n, *imat_nid(a.adj, k, i-1), 0);
					if(temp_min > cost)
						temp_min = cost;
				}
				min_cost = min(min_cost, *imat_uint32_t(cost_n, i, 0) + temp_min);
			}

			if(b_adj_len > 0) {
				uint32_t temp_min = UINT32_MAX;
				for(uint32_t k = 0; k < b_adj_len; k++) {
					uint32_t cost = *imat_uint32_t(cost_n, i, *imat_nid(b.adj, k, j-1)) - *imat_uint32_t(cost_n, 0, *imat_nid(a.adj, k, j-1));
					if(temp_min > cost)
						temp_min = cost;
				}
				min_cost = min(min_cost, *imat_uint32_t(cost_n, 0, j) + temp_min);
			}

			*imat_uint32_t(cost_n, i, j) = min_cost;
		}
	}
}

void constrained_tree_alignment (
	const struct Tree a,
	const struct Tree b,
	const mat_uint32_t cost,
	const mat_uint32_t cost_n, // The resulting computed cost matrixes node and forest. Size a.len x b.len
	const mat_uint32_t cost_f,
	mat_uint32_t cost_s,
	uint32_t *adj_alignment,
	mat_uint32_t alignment
) {

	mat_uint32_t to_compute = {
		.data = malloc((2 * (a.len * b.len)) * sizeof(uint32_t)),
		.stride = 2
	};
	*imat_uint32_t(to_compute, 0, 0) = 1;
	*imat_uint32_t(to_compute, 1, 0) = 1;
	size_t to_compute_head = 1;

	while(to_compute_head > 0) {
		to_compute_head--;
		uint32_t i = *imat_uint32_t(to_compute, 0, to_compute_head);
		uint32_t j = *imat_uint32_t(to_compute, 1, to_compute_head);

        if(i == -1) {
			size_t remain = 1;
			size_t cursor = j-1;
			while(remain > 0) {
				log("ADD %ld", cursor);

				size_t adj_len = 0;
				while(adj_len < b.adj.stride && *imat_nid(b.adj, adj_len, cursor) != 0) adj_len++;
				remain += adj_len;

				cursor++;
				remain--;
			}
			continue;
		} else if(j == -1) {
			size_t remain = 1;
			size_t cursor = i-1;
			while(remain > 0) {
				log("REMOVE %ld", cursor);

				size_t adj_len = 0;
				while(adj_len < a.adj.stride && *imat_nid(a.adj, adj_len, cursor) != 0) adj_len++;
				remain += adj_len;

				cursor++;
				remain--;
			}
			continue;
		}

		size_t a_adj_len = 0;
		while(a_adj_len < a.adj.stride && *imat_nid(a.adj, a_adj_len, i-1) != 0) a_adj_len++;

		size_t b_adj_len = 0;
		while(b_adj_len < b.adj.stride && *imat_nid(b.adj, b_adj_len, j-1) != 0) b_adj_len++;

		uint32_t min_cost_a = UINT32_MAX;
		ssize_t min_a = -1;
		if(a_adj_len > 0) {
			for(uint32_t k = 0; k < a_adj_len; k++) {
				uint32_t cost = *imat_uint32_t(cost_n, *imat_nid(a.adj, k, i-1), j) - *imat_uint32_t(cost_n, *imat_nid(a.adj, k, i-1), 0);
				if(min_cost_a > cost) {
					min_cost_a = cost;
					min_a = k;
				}
			}
		}

		uint32_t min_cost_b = UINT32_MAX;
		ssize_t min_b = -1;
		if(b_adj_len > 0) {
			for(uint32_t k = 0; k < b_adj_len; k++) {
				uint32_t cost = *imat_uint32_t(cost_n, i, *imat_nid(b.adj, k, j-1)) - *imat_uint32_t(cost_n, 0, *imat_nid(b.adj, k, j-1));
				if(min_cost_b > cost) {
					min_cost_b = cost;
					min_b = k;
				}
			}
		}

		if(*imat_uint32_t(cost_n, i, j) == *imat_uint32_t(cost_f, i, j) + *imat_uint32_t(cost, i, j)) {
			log("MATCH %d %d", i, j);
			// Compare the forests underneath this node
			string_edit_distance(imat_nid(a.adj, 0, i-1), a_adj_len, imat_nid(b.adj, 0, j-1), b_adj_len, cost_n, cost_s);
			assert(*imat_uint32_t(cost_s, a_adj_len, b_adj_len) == *imat_uint32_t(cost_f, i, j));
			string_edit_alignment(imat_nid(a.adj, 0, i-1), a_adj_len, imat_nid(b.adj, 0, j-1), b_adj_len, cost_n, cost_s, adj_alignment);

			ssize_t a_cursor = a_adj_len-1;
			ssize_t b_cursor = b_adj_len-1;

			while(b_cursor >= 0) {
				if(adj_alignment[b_cursor] == -1) {
					uint32_t *slot = imat_uint32_t(to_compute, 0, to_compute_head);
					to_compute_head++;
					slot[0] = -1;
					slot[1] = *imat_nid(b.adj, b_cursor, j-1);

					b_cursor--;
					continue;
				}

				while(a_cursor > adj_alignment[b_cursor]) {
					uint32_t *slot = imat_uint32_t(to_compute, 0, to_compute_head);
					to_compute_head++;
					slot[0] = *imat_nid(a.adj, a_cursor, i-1);
					slot[1] = -1;
					a_cursor--;

					continue;
				}

				uint32_t *slot = imat_uint32_t(to_compute, 0, to_compute_head);
				to_compute_head++;
				slot[0] = *imat_nid(a.adj, a_cursor, i-1);
				slot[1] = *imat_nid(b.adj, b_cursor, j-1);

				b_cursor--;
				a_cursor--;
			}
		} else if(a_adj_len > 0 && *imat_uint32_t(cost_n, i, j) == *imat_uint32_t(cost_n, i, 0) + min_cost_a) {
			// Remove this node and replace it with one of its children
			log("REMOVE %ld %ld", i, j);
			for(size_t x = 0; x < a_adj_len; x++) {
				uint32_t *slot = imat_uint32_t(to_compute, 0, to_compute_head);
				to_compute_head++;
				if(x == min_a) {
					slot[0] = *imat_nid(a.adj, x, i-1);
					slot[1] = -1;
				} else {
					slot[0] = *imat_nid(a.adj, x, i-1);
					slot[1] = i;
				}
			}
		} else if(b_adj_len > 0 && *imat_uint32_t(cost_n, i, j) == *imat_uint32_t(cost_n, 0, j) + min_cost_b) {
			// Inject a node here, moving the current node (from a) into the child forest.
			log("ADD %ld %ld", i, j);
			for(size_t x = 0; x < b_adj_len; x++) {
				uint32_t *slot = imat_uint32_t(to_compute, 0, to_compute_head);
				to_compute_head++;
				if(x == min_b) {
					slot[0] = i;
					slot[1] = *imat_nid(b.adj, x, j-1);
				} else {
					slot[0] = -1;
					slot[1] = *imat_nid(b.adj, x, j-1);
				}
			}
		} else {
			abort();
		}
	}

	log("END");
	free(to_compute.data);
}
