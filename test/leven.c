#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "leven.h"
#include "log.h"

DECL_MAT(weight, uint32_t, 256, 256);

static void simple_distance() {
	for(uint32_t i = 0; i < 256 * 256; i++) {
		weight.data[i] = 2;
	}

	for(uint32_t i = 1; i < 256; i++) {
		// The identity values
		*imat_uint32_t(weight, i, i) = 0;

		// Adding a character
		*imat_uint32_t(weight, i, 0) = 1;
		// Removing a character
		*imat_uint32_t(weight, 0, i) = 1;
	}
}

static size_t nidlen(nid *a) {
	size_t l = 0;
	while(a[l] != 0) l++;
	return l;
}

static uint32_t* run_leven(nid *a, nid *b) {
	size_t la = nidlen(a);
	size_t lb = nidlen(b);

	mat_uint32_t dist = {
		.data = malloc((la+1) * (lb+1) * sizeof(uint32_t)),
		.stride = la+1,
	};
	uint32_t *alignment = malloc(lb * sizeof(uint32_t));

	string_edit_distance(a, la, b, lb, weight, dist);
	string_edit_alignment(a, la, b, lb, weight, dist, alignment);

	free(dist.data);
	return alignment;
}

int main(int argc, char **argv) {

	log("Levenshtein tests");

	// Initialize the weight with simple weights
	simple_distance();
	{
		uint32_t* alignment = run_leven((nid[]){1, 0}, (nid[]){2, 0});
		uint32_t expected[] = {
			0,
		};
		if(memcmp(alignment, expected, sizeof(expected)/sizeof(*expected)) != 0) {
			return 1;
		}
		free(alignment);
	}

	{
		uint32_t* alignment = run_leven((nid[]){1, 0}, (nid[]){2, 1, 0});
		uint32_t expected[] = {
			-1, 0,
		};
		if(memcmp(alignment, expected, sizeof(expected)/sizeof(*expected)) != 0) {
			return 1;
		}
		free(alignment);
	}

	{
		uint32_t* alignment = run_leven((nid[]){1, 2, 3, 3, 4, 5, 0}, (nid[]){7, 3, 3, 4, 5, 3, 8, 5, 0});
		logb("Alignment: ");
		for(size_t i = 0; i < 8; i++) {
			logc("%d ", alignment[i]);
		}
		loge();
		uint32_t expected[] = {
			1, 2, 3, 4, -1, -1, -1, 5,
		};
		if(memcmp(alignment, expected, sizeof(expected)) != 0) {
			return 1;
		}
		free(alignment);
	}

	// Turning an 1 into 2 now costs 3
	*imat_uint32_t(weight, 1, 2) = 3;
	{
		uint32_t* alignment = run_leven((nid[]){1, 0}, (nid[]){2, 0});
		// So it prefers to remove the a an add a b
		uint32_t expected[] = {
			-1,
		};
		if(memcmp(alignment, expected, sizeof(expected)/sizeof(*expected)) != 0) {
			return 1;
		}
		free(alignment);
	}
	// Reset the cost of 1 into 2
	*imat_uint32_t(weight, 1, 2) = 2;
}
