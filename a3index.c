
/*
	build wordList
*/
int update_WordList (char * key) {

	WordList[key] += 1;

	return 0;
}



/*
	create a index-file for current txt-file using current WordList
*/
int create_this_index (char * index_dir) {

	const char * filename;
	std::string str;
	char * index_path;
	
	str = std::to_string(FileCur);
	filename = str.c_str();
	int length = ( strlen(index_dir) + 1 + strlen(filename) + 1 );

	//-- form index-file-path
	index_path = (char *)malloc(length * sizeof(char));

	strcpy(index_path, index_dir);
	strcat(index_path, "/");
	strcat(index_path, filename);
	index_path[length-1] = '\0';

	//-- create index-file
	FILE * fp;
	fp = fopen(index_path, "w");

	//-- write WordList
	int i;
	const char * word;
	const char * freq;
	std::map<std::string, int>::iterator it;
	for (it = WordList.begin(); it != WordList.end(); ++it) {
		word = it->first.c_str();
		str = std::to_string(it->second);
		freq = str.c_str();
		for (i = 0; i < strlen(word); i++)
			putc(word[i], fp);
		putc(' ', fp);
		for (i = 0; i < strlen(freq); i++)
			putc(freq[i], fp);
		putc('\n', fp);
	}


	//-- clean up
	fclose(fp);
	free(index_path);

	return 0;
}




/*
	parse a file and generate Index List
*/
void parse_file (FILE * fp) {

	char ch;
	// read word
	char * word = (char *)malloc(WORDLEN * sizeof(char));
	char * stem = (char *)malloc(WORDLEN * sizeof(char));

	if (DEBUG && word == NULL)
		printf("parse_file(): word mem fail\n");
	if (DEBUG && stem == NULL)
		printf("parse_file(): stem mem fail\n");

	int i = 0;
	int first_split = 0;

	while ( (ch = getc(fp)) != EOF ) {

		//std::cout << ch;

		// is alphabet
		if (is_alphabet(ch)) {
			word[i++] = ch;
			first_split = 1;
		}
		// is unwanted symbol
		else {
			// deal with word
			if (first_split) {
				first_split = 0;
				word[i] = '\0';
				// at least 3 charactors
				if (strlen(word) >= 3) {
					// stemmer
					strcpy(stem, stem_word(word));
//strcpy(stem, word);
					// update WordList
					if (strlen(stem) > 0) {
						update_WordList(stem);
						/////// printf("%s\n", stem);
					}
					else if(DEBUG) {
						printf("stem null: %s\n", word);
					}
				}
				i = 0;
			}
		}

	}

	free(word);
	free(stem);

}


/*
	main function of build index
*/
void build_index (char * index_dir, char * file_dir) {

	FILE * fp;
	int i;

	//----- create index dir -----//
	mkdir(index_dir, 0777);


	//----- parse all text files ------//
	//----- and create index for each txt file -----//
	/*
	each file a List, when merge open 1000 per time
	*/
	FileCur = 0;
	char * file_path;
	file_path = (char *)malloc( PATHLEN * sizeof(char) );
	char * filename;
	int length = 0;

	printf("%d\n", FileNum);

	for (i = 0; i < FileNum; i++) {

		filename = FileNames[i];

		// form file-path
		length = ( strlen(file_dir) + 1 + strlen(filename) + 1 );
		strcpy(file_path, file_dir);
		strcat(file_path, "/");
		strcat(file_path, filename);
		file_path[length-1] = '\0';

		// open this file
		fp = fopen(file_path, "r");

		if ( DEBUG && ! fp) {
			printf("file open failed: %d | %s\n", i, file_path);
			exit(1);
		}

		// parse text in this file and add to dict
		parse_file(fp);
		/////// printf("txt-file-path: %s\n", file_path);

//////		printf("done\n");

		// write WordList to file
		create_this_index(index_dir);

//////		printf("-- %d\n", i);

		// free WordList
		WordList.clear();

		fclose(fp);

		FileCur++;
	}

	free(file_path);

//////	printf("check \n");

}

