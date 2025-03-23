#include "parse.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "log.h"

#define fail() fail_impl(__LINE__);

int fail_impl(int line) {
	log("%d: Test fail!\n", line);
	return 1;
}

int main(int argc, char **argv) {
	printf("Parser Tests\n");

	{
		struct Tree a;
		size_t *chunks = NULL;
		if(parse_string("<root></root>", &a, &chunks) != 0) return fail();
		if(a.adj.stride != 1) return fail();
		if(a.len != 1) return fail();
		nid expected[] = {
			0,
		};
		if(memcmp(a.adj.data, expected, sizeof(expected)) != 0) return fail();
		
		// Verify chunk offsets
		if(chunks[0] != 0) return fail();
		
		free(chunks);
		free(a.adj.data);
	}

	{
		struct Tree a;
		size_t *chunks = NULL;
		if(parse_string("<root><a></a><b></b></root>", &a, &chunks) != 0) return fail();
		if(a.adj.stride != 2) return fail();
		if(a.len != 3) return fail();
		nid expected[] = {
			2, 3,
			0, 0,
			0, 0,
		};
		if(memcmp(a.adj.data, expected, sizeof(expected)) != 0) return fail();
		
		// Verify chunk offsets
		if(chunks[0] != 0) return fail();
		if(chunks[1] != 6) return fail();
		if(chunks[2] != 13) return fail();
		
		free(chunks);
		free(a.adj.data);
	}

	{
		struct Tree a;
		size_t *chunks = NULL;
		if(parse_string("<root><a><c></c></a><b></b></root>", &a, &chunks) != 0) return fail();
		if(a.adj.stride != 2) return fail();
		if(a.len != 4) return fail();
		nid expected[] = {
			2, 4,
			3, 0,
			0, 0,
			0, 0,
		};
		if(memcmp(a.adj.data, expected, sizeof(expected)) != 0) return fail();
		
		// Verify chunk offsets
		if(chunks[0] != 0) return fail();
		if(chunks[1] != 6) return fail();
		if(chunks[2] != 9) return fail();
		if(chunks[3] != 20) return fail();
		
		free(chunks);
		free(a.adj.data);
	}

	// We forgot to close the open tag
	{
		struct Tree a;
		size_t *chunks = NULL;
		fflush(stdout);
		if(parse_string("<root</root>", &a, &chunks) != 1) return fail();
		fflush(stdout);
	}

	// Test self-closing tag
	{
		struct Tree a;
		size_t *chunks = NULL;
		fflush(stdout);
		if(parse_string("<root><a /></root>", &a, &chunks) != 0) return fail();
		if(a.adj.stride != 1) return fail();
		if(a.len != 2) return fail();
		nid expected[] = {
			2,
			0,
		};
		if(memcmp(a.adj.data, expected, sizeof(expected)) != 0) return fail();
		
		// Verify chunk offsets
		if(chunks[0] != 0) return fail();
		if(chunks[1] != 6) return fail();

		free(chunks);
		free(a.adj.data);
	}

	return 0;
}
