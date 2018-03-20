
/*
	free posting
*/
void free_posting () {

	struct Post * tmp;

	while (PostHead != NULL) {
		tmp = PostHead;
		PostHead = PostHead->next;
		free(tmp);
	}
}


/*
	initialize posting
*/
void init_posting () {

	free_posting();
	PostHead = NULL;
	PostTail = NULL;
}


/*
	add item to posting
*/
void add_posting (struct Record * record) {

	struct Post * cur = (struct Post *)malloc(sizeof(struct Post));
	cur->file = record->file;
	cur->freq = record->freq;
	cur->next = NULL;

	if (PostHead == NULL) {
		PostHead = cur;
		PostTail = cur;
	}
	else {
		PostTail->next = cur;
		PostTail = cur;
	}
}


/*
	return min
*/
char * get_min_word (char * a, char * b) {

	if (strlen(a) == 0)
		return b;
	if (strlen(b) == 0)
		return a;
	int r = compare_strings(a, b);
	if (r == -1)
		return a;
	else
		return b;
}


//-------------------------------------------------------------


/*
	new item into New half Index
	format=> "word:file,freq file,freq \n"
*/
void half_write_to_new_index(char * min_word, FILE * fp) {

	int i;
	int length;

	// word
	length = strlen(min_word);
	for (i = 0; i < length; i++)
		putc(min_word[i], fp);
	putc(':', fp);

	std::string str;
	const char * file;
	const char * freq;
	struct Post * cur;
	cur = PostHead;
	while (cur != NULL){
		// file
		str = std::to_string(cur->file);
		file = str.c_str();
		length = strlen(file);
		for (i = 0; i < length; i++)
			putc(file[i], fp);
		putc(',', fp);
		// freq
		str = std::to_string(cur->freq);
		freq = str.c_str();
		length = strlen(freq);
		for (i = 0; i < length; i++)
			putc(freq[i], fp);
		putc(' ', fp);

		cur = cur->next;
	}
	putc('\n', fp);
}


/*
	get first record from small Index
	receive:	char * (word), int (j start 0, for FPS)
	return:		int (freq if 0 means end of index-file)
*/
int half_get_one_record_from_index (char * word, int j) {

	int freq = 0;
	char ch;

	int i = 0;
	while ( (ch = getc(FPS[j])) != EOF) {
		// word
		if (is_alphabet(ch)) {
			word[i++] = ch;
		}
		// word end
		else if (ch == ' ') {
			word[i] = '\0';
		}
		// num
		else if (is_number(ch)) {
			freq = freq * 10 + (ch - 48);
		}
		// newline
		else if (ch == '\n') {
			break;
		}

	}

	return freq;
}


/*
	merge half
	start - end same as name of small index files
*/
void merge_half (char * index_dir, int start, int end) {

	int i, j;
	FILE * fp;
	const char * filename;
	std::string str;
	char * index_path = (char *)malloc(PATHLEN * sizeof(char));

	//------ open target-index-file to be created ---//
	/* HALFA/B length is 6 */
	strcpy(index_path, index_dir);
	strcat(index_path, "/");
	if (start == 0) {
		strcat(index_path, HALFA);
		index_path[(strlen(index_dir)+1+6)] = '\0';
		fp = fopen(index_path, "w");
	}
	if (start != 0) {
		strcat(index_path, HALFB);
		index_path[(strlen(index_dir)+1+6)] = '\0';
		fp = fopen(index_path, "w");
	}

	//------ malloc FPS ---//
	FPS = (FILE * *)malloc( (end-start+1) * sizeof(FILE *) );

	//------ open all small-index-file, i = index-name ---//
	j = 0;
	for (i = start; i <= end; i++) {
		
		str = std::to_string(i);
		filename = str.c_str();
		int length = ( strlen(index_dir) + 1 + strlen(filename) + 1 );
		strcpy(index_path, index_dir);
		strcat(index_path, "/");
		strcat(index_path, filename);
		index_path[length-1] = '\0';

		FPS[j++] = fopen(index_path, "r");

		if (DEBUG && FPS[j-1] == NULL) /// macOS limit 252 files now, linux 1000 files
			printf("index file %d open failed !\n", i);
	}


	//------ initialize IndexRecordArray ---//
	RecordArray = (struct Record * *)malloc( (end-start+1) * sizeof(struct Record *) );
	
	if (DEBUG && RecordArray == NULL)
		printf("RecordArray malloc failed\n");
	// Must initial Null to each Record pointer !
	j = 0;
	for (i = start; i <= end; i++) {
		RecordArray[j++] = NULL;
	}


	//------ initialeze word etc. for next part---//
	int freq;
	int null_count;
	int compare;
	char * word = (char *)malloc(WORDLEN * sizeof(char));
	char * min_word = (char *)malloc(WORDLEN * sizeof(char));

	/*------ keep reading first Record from each index
			 scan-1: find the min-word
			 scan-2: all min-word form NewRecord write into new Index
			 after scan-2 there are some Null in RecordArray,
			 read next Record from indexes ---*/

	while (true) {

		//--- default min_word
		min_word[0] = '\0';
		//--- for check if all index-files end
		null_count = 0;
		//--- generate RecordArray / get first Records from each index
		j = 0;
		for (i = start; i <= end; i++) {
			// current Record not Null, dont need read
			if (RecordArray[j] != NULL) {
				j++;
				continue;
			}
			// get freq and word
			freq = half_get_one_record_from_index(word, j);
			// no record anymore in this Index
			if (freq == 0) {
				null_count++;
			}
			// new Record into RecordArray
			else {
				RecordArray[j] = (struct Record *)malloc( sizeof(struct Record) );
				RecordArray[j]->word = strdup(word);
				RecordArray[j]->freq = freq;
				RecordArray[j]->file = i;
			}
			j++;
		}
		//--- merge done !
		if ( null_count == (end-start+1) ) {
			break;
		}
		//--- scan1: find min-word
		j = 0;
		for (i = start; i <= end; i++) {
			// current not Null: return min-word
			if (RecordArray[j] != NULL)
				strcpy(min_word,
					get_min_word(min_word, RecordArray[j]->word));
			j++;
		}
		//--- scan2: all min-word (and set Null), NewRecord > new Index
		/* at this moment we already have min_word
		   generete posting */
		// Must init posting first of course
		init_posting();
		// add to posting
		j = 0;
		for (i = start; i <= end; i++) {
			// current not Null
			if (RecordArray[j] != NULL) {
				compare = 
					compare_strings(min_word, RecordArray[j]->word);
				// this word == min_word
				if (compare == 0) {
					// add (file,freq) to posting list
					add_posting(RecordArray[j]);
					// free and set Null
					free(RecordArray[j]->word);
					free(RecordArray[j]);
					RecordArray[j] = NULL;
				}
			}
			j++;
		}
		// write word:posting into new Index
		half_write_to_new_index(min_word, fp);
	}

	//------ clean up ---//
	fclose(fp);
	j = 0;
	for (i = start; i <= end; i++) {
		fclose(FPS[j++]);
	}
	free(FPS);
	free(word);
	free(min_word);
	free(index_path);
	j = 0;
	for (i = start; i <= end; i++) {
		if (RecordArray[j] != NULL) {
			free(RecordArray[j]->word);
			free(RecordArray[j]);
		}
		j++;
	}
	free(RecordArray);
	free_posting();

}



//-----------------------------------------------------------------



/*
	write rest content of fpS into fp
*/
void final_write_rest (FILE * fpS, FILE * fp) {

	char ch;
	while ( (ch = getc(fpS)) != EOF ) {
		putc(ch, fp);
	}
}


/*
	write word and post
	both means if there are two postings to write
	format=> "word:file,freq file,freq \n"
*/
void final_write_record (FILE * fp, char * word, char * post1, bool both, char * post2) {

	int i;
	int length;
	// word
	length = strlen(word);
	for (i = 0; i < length; i++)
		putc(word[i], fp);
	putc(':',fp);
	// posting(s)
	length = strlen(post1);
	for (i = 0; i < length; i++)
		putc(post1[i], fp);
	if (both) {
		length = strlen(post2);
		for (i = 0; i < length; i++)
			putc(post2[i], fp);
	}
	putc('\n', fp);
}


/*
	get one record from Half A/B
	record => word, posting
	return false means ok, return true means already end
*/
bool final_get_one_record_from_index (char * word, char * post, FILE * fp) {

	int n = 0;
	char ch;

	while ( (ch = getc(fp)) != EOF ) {
		if (is_alphabet(ch)) {
			word[n] = ch;
		}
		else if (ch == ':'){
			word[n] = '\0';
			n = 0;
			continue;
		}
		else if (ch == '\n') {
			post[n] = '\0';
			break;
		}
		else {
			post[n] = ch;
		}
		n++;
	}

	bool end;
	if (n == 0) end = true;
	else end = false;
	return end;
}


/*
	final merge : halfA halfB
	format=> "word:file,freq file,freq \n"
*/
void merge_final (char * index_dir) {

	FILE * fpA, * fpB, * fp;
	std::string str;
	char * index_path = (char *)malloc(PATHLEN * sizeof(char));

	//------ open half A/B , final------//
	/* HALFA/B length is 6 */
	strcpy(index_path, index_dir);
	strcat(index_path, "/");
	strcat(index_path, HALFA);
	index_path[(strlen(index_dir)+1+6)] = '\0';
	fpA = fopen(index_path, "r");

	strcpy(index_path, index_dir);
	strcat(index_path, "/");
	strcat(index_path, HALFB);
	index_path[(strlen(index_dir)+1+6)] = '\0';
	fpB = fopen(index_path, "r");

	strcpy(index_path, index_dir);
	strcat(index_path, "/");
	strcat(index_path, INDEX);
	index_path[(strlen(index_dir)+1+5)] = '\0';
	fp = fopen(index_path, "w");

	/*------ keep reading first record from HalfA/B
			 write the smaller word (one OR two)
			 if both reach end then done ---*/
	
	// "apple"
	char * wordA = (char *)malloc(WORDLEN * sizeof(char));
	char * wordB = (char *)malloc(WORDLEN * sizeof(char));
	// "file1,freq file2,freq"
	char * postA = (char *)malloc(POSTLEN * sizeof(char));
	char * postB = (char *)malloc(POSTLEN * sizeof(char));
	bool endA = false;
	bool endB = false;
	bool readA = true;
	bool readB = true;
	int com;

	//-- keep reading
	while (true) {
		if (readA)
			endA = final_get_one_record_from_index(wordA, postA, fpA);
		if (readB)
			endB = final_get_one_record_from_index(wordB, postB, fpB);
		//-- both end, done !
		if (endA && endB)
			break;
		//-- write rest of B
		else if (endA) {
			// write current B
			final_write_record(fp, wordB, postB, false, NULL);
			// write rest
			final_write_rest(fpB, fp);
			break;
		}
		//-- write rest of A
		else if (endB) {
			// write current A
			final_write_record(fp, wordA, postA, false, NULL);
			// write rest
			final_write_rest(fpA, fp);
			break;
		}
		//-- write min
		else {
			com = compare_strings(wordA, wordB);
			// if A min: write A, readA true, readB false
			if (com == -1) {
				final_write_record(fp, wordA, postA, false, NULL);
				readA = true;
				readB = false;
			}
			// if B min: write B, readB true, readA false
			else if (com == 1) {
				final_write_record(fp, wordB, postB, false, NULL);
				readB = true;
				readA = false;
			}
			// if ==: merge write, both true
			else if (com == 0) {
				final_write_record(fp, wordA, postA, true, postB);
				readA = true;
				readB = true;
			}
		}
	}


	//------ clean up ------//
	free(index_path);
	fclose(fpA);
	fclose(fpB);
	fclose(fp);
	free(wordA);
	free(wordB);
	free(postA);
	free(postB);

}



//-----------------------------------------------------------------


/*
	write record into word-index
	word-index format: "word:pos\n" 259+1+9+1
	eg app12345 : 'a' 'p' 'p' '1' '2' '3' '4' '5' ' ' ' ' ...... '\n'
*/
void word_write_index (FILE * fp, char * word, unsigned long pos) {

	int length;
	int empty;
	int i;

	const char * number;
	std::string str;
	str = std::to_string(pos);
	number = str.c_str();

	length = strlen(word) + strlen(number);
	// 1 for '\n'
	empty = 270 - length - 1 - 1;

	for (i = 0; i < strlen(word); i++)
		putc(word[i], fp);

	putc(':', fp);

	for (i = 0; i < strlen(number); i++)
		putc(number[i], fp);

	for (i = 0; i < empty; i++)
		putc(' ', fp);

	putc('\n', fp);

}


/*
	final-index : "word:file,freq file,freq \n"
	get 101th, 201th, ... word and it's start-pos
	return 0(<=100 word), pos
*/
unsigned long word_get_one_word_from_final (FILE * fp, char * word) {

	char ch;
	// word count
	long count = 0;
	// word start-pos
	unsigned long pos;
	// flag for reading word
	bool read = false;
	int i = 0;

	while ( (ch = getc(fp)) != EOF ) {
		// word +1
		if (ch == '\n') {
			count++;
			// 100
			if ( count % INDEXBLOCK == 0 ) {
				// next word is target, currently pointed, then pos
				pos = ftell(fp);
				read = true;
			}
		}
		if (read) {
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

	// this time <= 100
	if (i == 0) {
		pos = 0;
	}

	return pos;
}


/*
	create word-pos-index for final-index
	in final-index, every 100 record a block. #define INDEXBLOCK
	eg  101-word 201-word .. and thire pos write into word-index
	word-index format: "word:pos\n" 259+1+9+1
	eg app12345 : 'a' 'p' 'p' '1' '2' '3' '4' '5' ' ' ' ' ...... '\n'
	(if final-index have less than 100 word then word-index is 0-byte)
*/
void word_index (char * index_dir) {

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
	fpw = fopen(index_path, "w");

	//------ get word every 100 (101th, 201th...) write word-index ------//
	// word
	char * word = (char *)malloc(WORDLEN * sizeof(char));
	// word start position
	unsigned long pos;

	while (true) {
		// get next word and it's start point
		pos = word_get_one_word_from_final(fpi, word);
		// <=100 word left
		if (pos == 0)
			break;
		// wirte
		word_write_index(fpw, word, pos);
	}


	//------ clean up ------//
	free(index_path);
	fclose(fpi);
	fclose(fpw);
	free(word);
}



//-----------------------------------------------------------------

void delete_useless_index(char * index_dir) {

	DIR * fd;
	struct dirent * dir;
	char * index_path = NULL;
	index_path = (char *)malloc( PATHLEN * sizeof(char) );
	int length;
	int ret;

	fd = opendir(index_dir);
	while ((dir = readdir(fd))) {
		//-- avoid current and parent dir
		if (!strcmp (dir->d_name, "."))
			continue;
		if (!strcmp (dir->d_name, ".."))    
			continue;
		if (!strcmp (dir->d_name, INDEX))    
			continue;
		if (!strcmp (dir->d_name, WORD))    
			continue;
		length = ( strlen(index_dir) + 1 + strlen(dir->d_name) + 1 );
		strcpy(index_path, index_dir);
		strcat(index_path, "/");
		strcat(index_path, dir->d_name);
		index_path[length-1] = '\0';

		ret = remove(index_path);

		if (ret != 0 && DEBUG)
			printf("index delete failed\n");
	}

	closedir(fd);
	free(index_path);
}


//-----------------------------------------------------------------



/*
	main function : merger indexes -> final index-file and wordFinder
*/
void merge_index (char * index_dir) {

	//--- only one file ---//

	if (FileNum == 1) {
		if (DEBUG)
			printf("only one file found, exit...\n");
		printf("\n");
		exit(1);
	}

	//--- merge Half ---//
	
	int half_1, half_2;
	half_1 = FileNum / 2;
	half_2 = FileNum - half_1;

	if (DEBUG) {
		printf("1st half file: %d\n", half_1);
		printf("2nd half file: %d\n", half_2);
	}

	// first half
	merge_half (index_dir, 0, half_1-1);

	if (DEBUG)
		printf("merge-half-1 done\n");

	// second half
	merge_half (index_dir, half_2-1, FileNum-1);

	if (DEBUG)
		printf("merge-half-2 done\n");

	//--- merge Final ---//
	merge_final (index_dir);

	if (DEBUG)
		printf("merge-final done\n");

	//--- index for word position ---//
	word_index (index_dir);

	//--- delete others indexes ---//
	delete_useless_index(index_dir);

}