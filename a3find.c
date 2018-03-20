/*
	after each term search, update up to now search-res
*/
void update_term_result () {

	int file, freq;
	std::map<int, int>::iterator it;

	if (SearchResult.size() == 0){
		for (it = TermResult.begin(); it != TermResult.end(); ++it) {
			file = it->first;
			SearchResult[file] = it->second;
		}
	}
	else {
		for (it = SearchResult.begin(); it != SearchResult.end(); ++it) {
			file = it->first;
			freq = it->second;
			if (TermResult[file] == 0 || SearchResult[file] == 0) {
				SearchResult[file] = 0;
			}
			else {
				SearchResult[file] = freq + TermResult[file];
			}
		}
	}

}


/*
	get this search term result
	posting : "file,freq file,freq "
*/
void one_term_result (char * posting) {

	int i, j, k;
	int len = strlen(posting);
	bool isfile = true;
	char * file = (char *)malloc(10 * sizeof(char));
	char * freq = (char *)malloc(20 * sizeof(char));
	int nfile = 0;
	int nfreq = 0;

	j = 0;
	for (i = 0; i < len; i++) {
		if (posting[i] == ',') {
			file[j] = '\0';
			for (k = 0; k < strlen(file); k++){
				nfile *= 10;
				nfile += (file[k]- 48);
			}
			isfile = false;
			j = 0;
			nfreq = 0;
		}
		else if (posting[i] == ' ') {
			freq[j] = '\0';
			for (k = 0; k < strlen(freq); k++){
				nfreq *= 10;
				nfreq += (freq[k] - 48);
			}
			TermResult[nfile] = nfreq;
			isfile = true;
			j = 0;
			nfile = 0;
		}
		else {
			if (isfile) {
				file[j++] = posting[i];
			} else {
				freq[j++] = posting[i];
			}
		}
	}

	free(file);
	free(freq);
}



//----------------------------------------------------------------------------


/*
	search main-index to find word , sewarch range(Start, End)
	if no result set HasResult -> false !!!!!!
*/
void search_main_index(char * term, FILE * fp, char * posting) {

	char * word = (char *)malloc(WORDLEN * sizeof(char));

	fseek(fp, Start, SEEK_SET);

	bool find = false;
	char ch;
	int i = 0;
	while ( (ch = getc(fp)) != EOF ) {
		if (ftell(fp) > End)
			break;
		if (is_alphabet(ch)) {
			word[i] = ch;
			i++;
		}
		if (ch == ':'){
			word[i] = '\0';
			if (compare_strings(word, term) == 0)
				find = true;
			i = 0;
			continue;
		}
		if (find) {
			if (ch == '\n') {
				posting[i] = '\0';
				break;
			}
			posting[i] = ch;
			i++;
		}
	}

	// this term not appears in main-index, so all the search no result
	if ( ! find )
		HasResult = false;

	free(word);
}


//---------------------------------------------------------------------


/*
	get word-data from Word-index at a given address(pos)
*/
void get_word_from_word_index (FILE * fp, char * word, unsigned long pos) {

	fseek(fp, pos, SEEK_SET);

	char ch;
	int i = 0;
	while ( (ch = getc(fp)) != EOF ) {
		if (is_alphabet(ch)) {
			word[i] = ch;
			i++;
		}
		if (ch == ':') {
			word[i] = '\0';
			break;
		}
	}
}

/*
	get pos-data from Word-index at a given address(pos)
*/
unsigned long get_pos_from_word_index (FILE * fp, unsigned long pos) {

	unsigned long res = 0;
	char * number = (char *)malloc(10 * sizeof(char));

	fseek(fp, pos, SEEK_SET);

	char ch;
	int i = 0;
	while ( (ch = getc(fp)) != EOF ) {
		if (is_number(ch)) {
			number[i] = ch;
			i++;
		}
		if (ch == ' ') {
			number[i] = '\0';
			break;
		}
	}

	int len = strlen(number);
	for (i = 0; i < len; i++) {
		res = res * 10;
		res = res + (number[i] - 48);
	}

	free(number);

	return res;
}

/*
	search word-index set Start End for main-index search 
	this is binary search, search range for this can start from previous low
	word-index format: "word:pos  \n" 259+1+9+1
*/
void search_word_index (char * term, FILE * fp) {

	char ch;
	unsigned long size;

	//--- check if empty ---//
	fseek (fp, 0, SEEK_END);
	size = ftell(fp);
	if (size == 0){
		Start = 0;
		End = SizeMainIndex;
		return;
	}

	char * word = (char *)malloc(WORDLEN * sizeof(char));

	//------ binary ------//

	unsigned long high, low, mid;
	// terms are ordered, so low can start from previous-one
	low = LastLow;
	high = (size / 270) - 1;
	mid = (high + low) / 2;

	while (true) {
		get_word_from_word_index(fp, word, mid * 270);
		if (compare_strings(term, word) == -1) {
			high = mid;
			if (high - low == 1)
				break;
			mid = (high + low) / 2;
		}
		else if (compare_strings(term, word) == 1) {
			low = mid;
			if (high - low == 1)
				break;
			mid = (high + low) / 2;
		}
		else if (compare_strings(term, word) == 0) {
			high = mid + 1;
			low = mid;
			break;
		}
	}

	// use high low set Start End for main-index

	if (low == 0) {
		// cause in word-index 1st record is not 1st word in main-index
		Start = 0;
		End = get_pos_from_word_index (fp, high * 270);
	}
	else if (high == ((size / 270) - 1) ) {
		// cause in word-index last record isn't last one in main-index
		Start = get_pos_from_word_index (fp, low * 270);
		End = SizeMainIndex;
	}
	else {
		Start = get_pos_from_word_index (fp, low * 270);
		End = get_pos_from_word_index (fp, high * 270);
	}

	// remeber this start position
	LastLow = low;


	free(word);
}



//-------------------------------------------------------------------------



/*
	search this term, find postings, and update FileFilter
*/
void search_this_word (char * term, char * index_dir) {

	FILE * fpi, *fpw;
	char * index_path = (char *)malloc(PATHLEN * sizeof(char));

	//------ open index, word ------//
	strcpy(index_path, index_dir);
	strcat(index_path, "/");
	strcat(index_path, INDEX);
	index_path[(strlen(index_dir)+1+5)] = '\0';
	fpi = fopen(index_path, "r");

	strcpy(index_path, index_dir);
	strcat(index_path, "/");
	strcat(index_path, WORD);
	index_path[(strlen(index_dir)+1+4)] = '\0';
	fpw = fopen(index_path, "r");

	//------ get size of main-index ------//
	fseek(fpi, 0, SEEK_END);
	SizeMainIndex = ftell(fpi);
	fseek(fpi, 0, SEEK_SET);

	//------ do the search ------//
	char * posting = (char *)malloc(POSTLEN * sizeof(char));

	// get range(Start End)
	search_word_index(term, fpw);
	// search in range(Start End), control global-flag: HasResult
	search_main_index(term, fpi, posting);

	if (HasResult && DEBUG)
		printf("result %s\n", posting);

	//------ update FileFilter ------//
	if (HasResult) {
		// clear previours res
		TermResult.clear();
		// get this term res
		one_term_result(posting);
		// update up to now search-res
		update_term_result();
	}


	//------ clean up ------//
	fclose(fpi);
	fclose(fpw);
	free(index_path);
	free(posting);
}





//--------------------------------------------------------------------





/*
	do the search
*/
void do_search (char * index_dir) {

	char * term = (char *)malloc(WORDLEN * sizeof(char));

	// get terms
	std::map<std::string, char>::iterator it;
	for (it = SearchTerms.begin(); it != SearchTerms.end(); ++it) {
		strcpy(term, it->first.c_str());
		if (HasResult) search_this_word(term, index_dir);
	}

	// get final result vector
	int file, freq;
	std::map<int, int>::iterator it2;
	for (it2 = SearchResult.begin(); it2 != SearchResult.end(); ++it2) {
		file = it2->first; freq = it2->second;
		if (freq != 0) {
			FinalResult.push_back(std::make_pair(freq, file));
		}
	}
	// check if has result
	if (FinalResult.size() == 0)
		HasResult = false;

	if (HasResult) {
		// sort
		std::sort(FinalResult.begin(), FinalResult.end(), [](const std::pair<int,int>&x,const std::pair<int,int>&y){
			return x.first != y.first ? x.first>y.first : x.second < y.second;});
	}

	// clean up
	free(term);
}



/*
	get and store search terms
	receive: stemmed-word
*/
void add_terms (char * term) {

	char * word = (char *)malloc(WORDLEN * sizeof(char));

	strcpy(word, term);

	SearchTerms[word] = 't';

	free(word);
}