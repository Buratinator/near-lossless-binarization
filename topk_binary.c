#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASHSIZE   10000
#define MAXLENWORD 64

struct nlist
{
	struct nlist *next; /* next element in linked list */
	char *word;
	long index;
};

static struct nlist *hashtab[HASHSIZE]; /* hashtab composed of linked lists */
static long word_index = 0;             /* index of vector associated to word */
static int n_bits = 0, n_long = 0;      /* #bits per vector, #long per array */

/* hash: form hash value for string s */
unsigned int hash(const char *s)
{
	unsigned int hashval;

	for (hashval = 0; *s != '\0'; ++s)
		hashval = *s + 31 * hashval;
	return hashval % HASHSIZE;
}

/* get_index: return vector index of word s, -1 if not found */
long get_index(const char *s)
{
	struct nlist *np;

	/* look for string s in hashtab */
	for (np = hashtab[hash(s)]; np != NULL; np = np->next)
		if (strcmp(np->word, s) == 0) /* found, return its index */
			return np->index;
	return -1;
}

/* add_word: add word s to hashtab (only if not present) */
void add_word(const char *s)
{
	struct nlist *np;
	unsigned int hashval = hash(s);

	if (get_index(s) > -1) /* already in hashtab */
		return;

	/* word not in hashtab, need to add it */
	if ((np = malloc(sizeof *np)) == NULL ||
	    (np->word = malloc(strlen(s) + 1)) == NULL)
		return;

	strcpy(np->word, s);
	np->next = hashtab[hashval];
	np->index = word_index++;
	hashtab[hashval] = np;
}

/* load_vectors: read the vector file, add each word to hashtab, save each
 *               vector as an array of `long`, so to represent a vector of 256
 *               bits it requires an array of 4 `long`. */
unsigned long **load_vectors(char *name)
{
	int i;
	long index, n_words;
	FILE *fp;                 /* to open vector file */
	char word[MAXLENWORD];    /* to read the word of each line in file */
	unsigned long **vec;      /* to store the binary embeddings */

	if ((fp = fopen(name, "r")) == NULL)
	{
		fprintf(stderr, "load_vectors: can't open %s\n", name);
		exit(1);
	}

	if (fscanf(fp, "%ld %d", &n_words, &n_bits) <= 0)
	{
		fprintf(stderr, "load_vectors: can't read number of bits\n");
		exit(1);
	}

	n_long = n_bits / (sizeof(long) * 8);
	if ((vec = calloc(n_words, sizeof *vec)) == NULL)
		return NULL;

	while (fscanf(fp, "%s", word) > 0)
	{
		/* add word to hashtab; its index is word_index because never
		 * seen before (so not already in hashtab)  */
		index = word_index;
		add_word(word);

		if ((vec[index] = calloc(n_long, sizeof **vec)) == NULL)
			continue;

		for (i = 0; i < n_long; ++i)
			fscanf(fp, "%lu", vec[index]+i);
	}

	fclose(fp);
	return vec;
}

int main(void)
{
	unsigned long **embedding;

	embedding = load_vectors("out.txt");

	return 0;
}
