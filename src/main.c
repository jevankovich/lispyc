#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

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
 * @param len the length of the word (not including null byte)
 */
void push_lex(struct lexarr *arr, const char * restrict term, size_t len) {
	// If len is zero, do nothing
	if (len == 0 || term == NULL) {
		return;
	}
	
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
	dest[len] = '\0';

	arr->offs[arr->off_occupied++] = arr->buf_occupied;
	arr->buf_occupied += len + 1;
}

void print_lex(struct lexarr *arr) {
	for (int i = 0; i < arr->off_occupied; i++) {
		printf("%s, ", arr->buf + arr->offs[i]);
	}
	printf("\n");
}

void lex(FILE *f, struct lexarr *arr) {
	const size_t BUF_SIZE = 63;
	size_t real_size = BUF_SIZE + 1;
	char *buf = malloc(BUF_SIZE + 1);
	int c;
	size_t i = 0;
	while ((c = fgetc(f)) != EOF) {
		// expand buf if needed
		if (i == BUF_SIZE) {
			char *new = realloc(buf, real_size * 2);
			if (!new) {
				fprintf(stderr, "OOM\n");
				exit(1);
			}
			real_size *= 2;
			buf = new;
		}
		// buf is now big enough for the new character

		char cc = c;

		switch (cc) {
			case ')':
				push_lex(arr, buf, i);
				push_lex(arr, ")", 1);
				i = 0;
				break;
			case '(':
				push_lex(arr, buf, i);
				push_lex(arr, "(", 1);
				i = 0;
				break;
			default:
				if (isspace(cc)) {
					push_lex(arr, buf, i);
					i = 0;
				} else {
					buf[i++] = cc;
				}
		}
	}

	free(buf);
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
	print_lex(&arr);
	destroy_lexarr(&arr);

	fclose(infile);
}
