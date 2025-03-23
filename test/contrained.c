#include "leven.h"

#include "log.h"

DECL_MAT3(no_trace, uint32_t, 0, 0, 0);

int main(int argc, char **argv) {
	log("Constrained edit tests");

	{
		struct Tree a = {
			.adj = { .data = &(nid){0},
				.stride = 1,
			},
			.len = 1,
		};

		struct Tree b = {
			.adj = {
				.data = &(nid){0},
				.stride = 1,
			},
			.len = 1,
		};

		DECL_MAT_DATA(cost, uint32_t, 2, 2, 
			0, 2,
			2, 1,
		);
		DECL_MAT(cost_n, uint32_t, 2, 2);
		DECL_MAT(cost_f, uint32_t, 2, 2);
		DECL_MAT(cost_s, uint32_t, 1, 1);

		constrained_tree_distance(
			a,
			b,
			(CTedData) {
				cost,
				cost_n,
				cost_f,
				cost_s,
			}
		);

		if(*imat_uint32_t(cost_n, 1, 1) != 1) {
			log("Test Fail");
			return 1;
		}

		DECL_MAT(alignment, uint32_t, 2, 2);
		uint32_t adj_alignment[2] = {0, 0};
		constrained_tree_alignment(
			a,
			b,
			(CTedData) {
				cost,
				cost_n,
				cost_f,
				cost_s,
			},
			adj_alignment,
			alignment
		);
	}

	{
		struct Tree a = {
			.adj = {
				.data = (nid[]){2, 0},
				.stride = 1,
			},
			.len = 2,
		};

		struct Tree b = {
			.adj = {
				.data = &(nid){0},
				.stride = 1,
			},
			.len = 1,
		};

		DECL_MAT_DATA(cost, uint32_t, 3, 2,
			0, 1, 2,
			2, 1, 1,
		);
		DECL_MAT(cost_n, uint32_t, 3, 2);
		DECL_MAT(cost_f, uint32_t, 3, 2);
		DECL_MAT(cost_s, uint32_t, 2, 2);

		constrained_tree_distance(
			a,
			b,
			(CTedData) {
				cost,
				cost_n,
				cost_f,
				cost_s,
			}
		);

		if(*imat_uint32_t(cost_n, 1, 1) != 2) {
			log("Test Fail");
			return 1;
		}

		DECL_MAT(alignment, uint32_t, 3, 2);
		uint32_t adj_alignment[2] = {0, 0};
		constrained_tree_alignment(
			a,
			b,
			(CTedData) {
				cost,
				cost_n,
				cost_f,
				cost_s,
			},
			adj_alignment,
			alignment
		);
	}

	{
		struct Tree a = {
			.adj = {
				.data = (nid[]){2, 0},
				.stride = 1,
			},
			.len = 2,
		};

		struct Tree b = {
			.adj = {
				.data = (nid[]){2, 0},
				.stride = 1,
			},
			.len = 2,
		};

		DECL_MAT_DATA(cost, uint32_t, 3, 3,
			0, 2, 2,
			2, 0, 2,
			2, 2, 0,
		);
		DECL_MAT(cost_n, uint32_t, 3, 3);
		DECL_MAT(cost_f, uint32_t, 3, 3);
		DECL_MAT(cost_s, uint32_t, 2, 2);

		constrained_tree_distance(
			a,
			b,
			(CTedData) {
				cost,
				cost_n,
				cost_f,
				cost_s,
			}
		);

		if(*imat_uint32_t(cost_n, 1, 1) != 0) {
			log("Test Fail");
			return 1;
		}

		DECL_MAT(alignment, uint32_t, 2, 2);
		uint32_t adj_alignment[2] = {0, 0};
		constrained_tree_alignment(
			a,
			b,
			(CTedData) {
				cost,
				cost_n,
				cost_f,
				cost_s,
			},
			adj_alignment,
			alignment
		);
	}

	{
		struct Tree a = {
			.adj = {
				.data = (nid[]){2, 3, 0, 0, 0, 0},
				.stride = 2,
			},
			.len = 3,
		};

		struct Tree b = {
			.adj = {
				.data = (nid[]){2, 0, 3, 4, 0, 0, 0, 0},
				.stride = 2,
			},
			.len = 4,
		};

		DECL_MAT_DATA(cost, uint32_t, 4, 5,
			2, 2, 2, 2,
			2, 2, 2, 2,
			2, 0, 2, 2,
			2, 2, 0, 2,
			2, 2, 2, 0,
		);
		DECL_MAT(cost_n, uint32_t, 4, 5);
		DECL_MAT(cost_f, uint32_t, 4, 5);
		DECL_MAT(cost_s, uint32_t, 3, 3);

		constrained_tree_distance(
			a,
			b,
			(CTedData) {
				cost,
				cost_n,
				cost_f,
				cost_s,
			}
		);

		if(*imat_uint32_t(cost_n, 1, 1) != 2) {
			log("Test Fail\n");
			return 1;
		}

		DECL_MAT(alignment, uint32_t, 2, 2);
		uint32_t adj_alignment[2] = {0, 0};
		constrained_tree_alignment(
			a,
			b,
			(CTedData) {
				cost,
				cost_n,
				cost_f,
				cost_s,
			},
			adj_alignment,
			alignment
		);
	}

	{
		struct Tree a = {
			.adj = {
				.data = (nid[]){2, 3, 0, 0, 0, 0},
				.stride = 2,
			},
			.len = 3,
		};

		struct Tree b = {
			.adj = {
				.data = (nid[]){2, 3, 0},
				.stride = 1,
			},
			.len = 3,
		};

		DECL_MAT_DATA(cost, uint32_t, 4, 4,
			0, 2, 2, 2,
			2, 0, 2, 2,
			2, 2, 0, 2,
			2, 2, 2, 0,
		);
		DECL_MAT(cost_n, uint32_t, 4, 4);
		DECL_MAT(cost_f, uint32_t, 4, 4);
		DECL_MAT(cost_s, uint32_t, 3, 2);

		constrained_tree_distance(
			a,
			b,
			(CTedData) {
				cost,
				cost_n,
				cost_f,
				cost_s,
			}
		);

		if(*imat_uint32_t(cost_n, 1, 1) != 4) {
			log("Test Fail\n");
			return 1;
		}

		DECL_MAT(alignment, uint32_t, 2, 2);
		uint32_t adj_alignment[2] = {0, 0};
		constrained_tree_alignment(
			a,
			b,
			(CTedData) {
				cost,
				cost_n,
				cost_f,
				cost_s,
			},
			adj_alignment,
			alignment
		);
	}

	{
		struct Tree a = {
			.adj = {
				.data = (nid[]){2, 0},
				.stride = 1,
			},
			.len = 2,
		};

		struct Tree b = {
			.adj = {
				.data = (nid[]){2, 3, 0},
				.stride = 1,
			},
			.len = 3,
		};

		DECL_MAT_DATA(cost, uint32_t, 3, 4,
			0, 2, 2,
			2, 0, 2,
			2, 2, 2,
			2, 2, 0,
		);
		DECL_MAT(cost_n, uint32_t, 3, 4);
		DECL_MAT(cost_f, uint32_t, 3, 4);
		DECL_MAT(cost_s, uint32_t, 3, 2);

		constrained_tree_distance(
			a,
			b,
			(CTedData) {
				cost,
				cost_n,
				cost_f,
				cost_s,
			}
		);

		if(*imat_uint32_t(cost_n, 1, 1) != 2) {
			log("Test Fail\n");
			return 1;
		}

		DECL_MAT(alignment, uint32_t, 2, 2);
		uint32_t adj_alignment[2] = {0, 0};
		constrained_tree_alignment(
			a,
			b,
			(CTedData) {
				cost,
				cost_n,
				cost_f,
				cost_s,
			},
			adj_alignment,
			alignment
		);
	}

	return 0;
}
