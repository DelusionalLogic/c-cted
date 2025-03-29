#include "leven.h"
#include <sys/types.h>

// Chunk offsets for each node in the tree
// chunks[nodeId] = start position of the tag in the original string
// The end position is implicitly the start of the next tag or the end of the string

int parse_string(char *str, struct Tree *tree, ssize_t **chunks);
