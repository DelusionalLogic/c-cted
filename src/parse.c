#include "parse.h"

#include <stdbool.h>
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "log.h"

enum ParsePhase {
	PHASE_COUNT,
	PHASE_BUILD,
};

struct ParseCtx {
	char * const str;
	char *cursor;
	struct Tree *tree;
	size_t tree_cursor;
	size_t *chunks;

	enum ParsePhase phase;
};

static uint64_t max(uint64_t a, uint64_t b) {
	return a > b ? a : b;
}

static bool alnum(struct ParseCtx *ctx) {
	return isalnum(ctx->cursor[0]);
}

static int read_STag(struct ParseCtx *ctx, bool *self_close, size_t nodeId) {
	// Record start position of the tag
	if (ctx->phase == PHASE_BUILD) {
		ctx->chunks[nodeId] = ctx->cursor - ctx->str;
	}

	if(*ctx->cursor != '<') return 1;
	ctx->cursor++;

	while(isalnum(*ctx->cursor) || *ctx->cursor == ' ' || *ctx->cursor == '"' || *ctx->cursor == '=') ctx->cursor++;

	// Check for self-closing tag
	if(*ctx->cursor == '/') {
		*self_close = true;
		ctx->cursor++;
	}

	if(*ctx->cursor != '>') return 1;
	ctx->cursor++;

	return 0;
}

static int read_Element(struct ParseCtx *ctx, size_t nodeId);

static int read_Content(struct ParseCtx *ctx, size_t nodeId) {
	int err;
	uint64_t children = 0;
	while(true) {
		if(ctx->cursor[0] == '<' && ctx->cursor[1] != '/') {
			size_t childId = ctx->tree_cursor++;

			err = read_Element(ctx, childId);
			if(err != 0) return err;

			if(ctx->phase == PHASE_BUILD)
				*imat_nid(ctx->tree->adj, children, nodeId) = childId+1;

			children++;
		} else if(alnum(ctx)) {
			ctx->cursor++;
		} else {
			break;
		}
	}

	if(ctx->phase == PHASE_COUNT)
		ctx->tree->adj.stride = max(ctx->tree->adj.stride, children);

	if(ctx->phase == PHASE_BUILD) {
		for(size_t i = children; i < ctx->tree->adj.stride; i++) {
			*imat_nid(ctx->tree->adj, i, nodeId) = 0;
		}
	}

	return 0;
}

static int read_ETag(struct ParseCtx *ctx) {
	if(*ctx->cursor != '<') return 1;
	ctx->cursor++;

	if(*ctx->cursor != '/') return 1;
	ctx->cursor++;

	while(isalnum(*ctx->cursor)) ctx->cursor++;

	if(*ctx->cursor != '>') return 1;
	ctx->cursor++;

	return 0;
}

static int read_Element(struct ParseCtx *ctx, size_t nodeId) {
	int err;
	bool self_close = false;

	err = read_STag(ctx, &self_close, nodeId);
	if(err != 0) return err;
	
	if(!self_close) {
		err = read_Content(ctx, nodeId);
		if(err != 0) return err;
		err = read_ETag(ctx);
		if(err != 0) return err;
	} else if(ctx->phase == PHASE_BUILD) {
		for(size_t i = 0; i < ctx->tree->adj.stride; i++)
			*imat_nid(ctx->tree->adj, i, nodeId) = 0;
	}
	
	if(ctx->phase == PHASE_COUNT) ctx->tree->len++;
	return 0;
}

// This follows a 2 phase approach. First we size out the tree to figure out how
// much data we are going to store. Then we allocate the space based on that
// count pass before then doing a second pass in the BUILD phase which fill in
// those data structures.
int parse_string(char *str, struct Tree *tree, size_t **chunks) {
	assert(chunks != NULL); // Chunks parameter is required
	
	tree->adj.stride = 1;
	tree->len = 0;
	struct ParseCtx ctx = {
		.str = str,
		.cursor = str,
		.tree = tree,
		.tree_cursor = 1,
		.phase = PHASE_COUNT,
	};
	int err;

	err = read_Element(&ctx, 0);
	if(err != 0) {
		log("Parse error at %ld", ctx.cursor - ctx.str);
		return err;
	}

	tree->adj.data = malloc(tree->len * tree->adj.stride * sizeof(*tree->adj.data));
	
	// Allocate memory for chunks
	*chunks = malloc(tree->len * sizeof(size_t));
	if (*chunks == NULL) {
		free(tree->adj.data);
		return 1;
	}

	// Reset the context for the next phase
	ctx.phase = PHASE_BUILD;
	ctx.cursor = str;
	ctx.tree_cursor = 1;
	ctx.chunks = *chunks;

	err = read_Element(&ctx, 0);
	// The second phase we assume the tree is valid since it passed the first phase
	assert(err == 0);

	return 0;
}
