#ifndef MAT_TYPE
#error MAT_TYPE must be defined
#endif

#include <stddef.h>
#include <assert.h>

#define PP_CONCAT1(x,y) x ## y
#define PP_CONCAT(x,y) PP_CONCAT1(x,y)

#ifndef MAT_H_SHARED
#define MAT_H_SHARED

#define DECL_MAT(NAME, TYPE, X, Y) \
		PP_CONCAT(mat_, TYPE) NAME = { \
			.data = (TYPE[X * Y]){}, \
			.stride = X, \
		}

#define DECL_MAT_DATA(NAME, TYPE, X, Y, ...) \
		PP_CONCAT(mat_, TYPE) NAME = { \
			.data = (TYPE[X * Y]){__VA_ARGS__}, \
			.stride = X, \
		}

#endif

#define SELF_T PP_CONCAT(mat_, MAT_TYPE)
#define F(NAME) PP_CONCAT(NAME##_, MAT_TYPE)

typedef struct {
	MAT_TYPE *data;
	size_t stride;
} SELF_T;

static inline MAT_TYPE *F(imat)(SELF_T mat, size_t x, size_t y) {
	assert(x >= 0 && x < mat.stride);
	assert(y >= 0);
	return &mat.data[x + y*mat.stride];
}

#undef SELF_T
#undef MAT_TYPE
