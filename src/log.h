#pragma once

#include <stdio.h>

#define logb(FORMAT, ...) fprintf(stderr, "%s:%d: " FORMAT, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__)
#define logc(FORMAT, ...) fprintf(stderr, FORMAT __VA_OPT__(,) __VA_ARGS__)
#define lognl() fprintf(stderr, "\n")
#define loge() lognl()

#define log(FORMAT, ...) do { \
	logb(FORMAT __VA_OPT__(,) __VA_ARGS__); \
	loge(); \
} while(0)
