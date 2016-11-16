#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct lexarr {
	// The number of offsets that can be stored
	size_t off_capacity;
	// The number of offsets that are stored
	size_t off_occupied;
	// The actual array of offsets
	size_t *offs;

	// The number of chars that can be stored
	size_t buf_capacity;
	// The number of chars that are stored
	size_t buf_occupied;
	// The packed sequence of words
	char *buf;
};

// Initialized the lex array
void init_lexarr(struct lexarr *arr) {
	arr->off_capacity = 16;
	arr->off_occupied = 0;
	arr->offs = malloc(sizeof(ptrdiff_t)*arr->off_capacity);

	arr->buf_occupied = 0;
	arr->buf_capacity = 64;
	arr->buf = malloc(arr->buf_capacity);

	if (arr->offs == NULL || arr->buf == NULL) {
		fprintf(stderr, "Out of memory... at init time... how much RAM do you have!?\n");
		exit(1);
	}
}

void destroy_lexarr(struct lexarr *arr) {
	free(arr->offs);
	free(arr->buf);
}

/**
 * @param arr an existing lex array
 * @param term the new word to add to the array
 * @param len the length of the word (including null byte)
 */
void push_lex(struct lexarr *arr, char * restrict term, size_t len) {
	if (arr->off_occupied == arr->off_capacity) {
		size_t *new = realloc(arr->offs, sizeof(size_t) * arr->off_capacity * 2);
		if (!new) {
			fprintf(stderr, "OOM\n");
			exit(1);
		}
		arr->offs = new;
		arr->off_capacity *= 2;
	}

	while (arr->buf_occupied + len > arr->buf_capacity) {
		char *new = realloc(arr->buf, arr->buf_capacity * 2);
		if (!new) {
			fprintf(stderr, "OOM\n");
			exit(1);
		}
		arr->buf = new;
		arr->buf_capacity *= 2;
	}

	char* dest = arr->buf + arr->buf_occupied;
	strncpy(dest, term, len);
	dest[len-1] = '\0';

	arr->offs[arr->off_occupied++] = arr->buf_occupied;
	arr->buf_occupied += len;
}

void print_lex(struct lexarr *arr) {
	for (int i = 0; i < arr->off_occupied; i++) {
		printf("%s, ", arr->buf + arr->offs[i]);
	}
	printf("\n");
}

void lex(FILE *f, struct lexarr *arr) {
	char* buf = calloc(2, 1);
	int c;
	while ((c = fgetc(f)) != EOF) {
		buf[0] = c;
		push_lex(arr, buf, 2);
	}
}

int main(int argv, char** argc) {
	FILE *infile;
	if (argv > 1) {
		if (strcmp("--", argc[1]) == 0) {
			infile = stdin;
		} else {
			infile = fopen(argc[1], "r");
		}
	} else {
		infile = stdin;
	}

	struct lexarr arr;
	init_lexarr(&arr);
	lex(infile, &arr);
	//print_lex(&arr);
	destroy_lexarr(&arr);
}
