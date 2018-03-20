/*
	get search-word
	return stemmed-word (lower case)
*/


static char * stem_word(char * word) {

	#define INC 10
	int lim = INC;
	sb_symbol * b = (sb_symbol *) malloc(lim * sizeof(sb_symbol));

	int i = 0;
	while(1) {
		if (i == strlen(word)) {
			const sb_symbol * stemmed = sb_stemmer_stem(Stemmer, b, i);
			free(b);
			return (char *) stemmed;
		}
		int ch = word[i];
		if (i == lim) {
			sb_symbol * newb;
			newb = (sb_symbol *)
			realloc(b, (lim + INC) * sizeof(sb_symbol));
			b = newb;
			lim = lim + INC;
		}
        /* force lower case: */
		if (isupper(ch)) ch = tolower(ch);
		b[i] = ch;
		i++;
	}

}


void init_stemmer () {

	char * charenc = NULL;

	Stemmer = sb_stemmer_new("english", charenc);

}

void del_stemmer () {

	sb_stemmer_delete(Stemmer);
}

