#pragma once

#include <stddef.h>
#include <stdint.h>

#define MAT_TYPE uint32_t
#include "mat.h"

typedef struct {
	uint32_t *data;
	size_t stride[2];
} mat3_uint32_t;

static inline uint32_t* imat3_uint32_t(mat3_uint32_t mat, size_t x, size_t y, size_t z) {
	assert(x >= 0);
	assert(y >= 0 && y < mat.stride[0]);
	assert(z >= 0 && z < mat.stride[1]);
	return &mat.data[x + y*mat.stride[0] + z*(mat.stride[0]*mat.stride[1])];
}
#define DECL_MAT3(NAME, T, X, Y, Z) \
	mat3_uint32_t NAME = { \
		.data = (T[X * Y * Z]){}, \
		.stride = {X, Y}, \
	} \

typedef uint16_t nid;

#define MAT_TYPE nid
#include "mat.h"

void string_edit_distance(const nid *a, size_t la, const nid *b, size_t lb, const mat_uint32_t weight, mat_uint32_t cost);
void string_edit_alignment(const nid *a, size_t la, const nid *b, size_t lb, const mat_uint32_t weight, const mat_uint32_t cost, uint32_t *alignment);

struct Tree {
	mat_nid adj;
	size_t len;
};

void constrained_tree_distance(
	struct Tree a,
	struct Tree b,
	mat_uint32_t cost,
	mat_uint32_t cost_n,
	mat_uint32_t cost_f,
	mat_uint32_t cost_s
);

void constrained_tree_alignment(
	const struct Tree a,
	const struct Tree b,
	const mat_uint32_t cost,
	const mat_uint32_t cost_n,
	const mat_uint32_t cost_f,
	mat_uint32_t cost_s,
	uint32_t *adj_alignment,
	mat_uint32_t alignment
);
