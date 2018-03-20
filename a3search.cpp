#include <iostream>
#include <map>
#include <utility>
#include <vector>
#include <algorithm>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "libstemmer_c/include/libstemmer.h"

#include "a3data.c"
#include "a3stem.c"
#include "a3help.c"
#include "a3index.c"
#include "a3merge.c"
#include "a3find.c"


int main(int argc, char *argv[]) {

	//---------- initialization --------//
	init_stemmer();


	// varibles for this fun
	int i, j;
	DIR * fd;
	struct dirent * dir;


	//-------- receive arguments ---------//
	char * file_dir = argv[1];
	char * index_dir = argv[2];


	//-------- get txt-file-names (sort using map) -------//

	// get sorted-names and Num
	i = 0;
	std::map<std::string, char> FileNamesMap;
	fd = opendir(file_dir);
	while ((dir = readdir(fd))) {
		//-- avoid current and parent dir
		if (!strcmp (dir->d_name, "."))
			continue;
		if (!strcmp (dir->d_name, ".."))    
			continue;
		FileNamesMap[dir->d_name] = 'f';
		i ++;
	}
	FileNum = i;
	closedir(fd);

	// store into array
	FileNames = (char * *)malloc(FileNum * sizeof(char *));
	i = 0;
	std::map<std::string, char>::iterator it;
	for (it = FileNamesMap.begin(); it != FileNamesMap.end(); ++it) {
		FileNames[i] = strdup(it->first.c_str());
		i++;
	}

	// clean up
	FileNamesMap.clear();



	//-------- index check -----------//
	fd = opendir(index_dir);
	// no index
	if (fd == NULL || DEBUG) {
		if (DEBUG)
			printf("need to build index: \n");
		// parse, create each-index
		build_index(index_dir, file_dir);
		// merge all indexes
		merge_index(index_dir);
	}


	//-------- search terms ----------//
	j = 0;
	char * word;
	for (i = 3; i < argc; i ++) {
		// -c
		if (argv[i][0] == '-')
			continue;
		// 0.X
		if (argv[i][1] == '.')
			continue;
		// stem word
		word = stem_word(argv[i]);
		if (strlen(word) == 0){
			continue;
		}
		// add search terms
		add_terms(word);
		j++;
	}
	// num of terms
	TermNum = j;
	// search
	do_search(index_dir);
	// print result
	if (HasResult) {
		// print
		std::vector <std::pair<int,int>>::iterator It;
		for (It = FinalResult.begin(); It != FinalResult.end(); ++It) {
			if (DEBUG)
				printf("freq: %d file: %d\n", It->first, It->second);
			printf("%s\n", FileNames[It->second]);
		}
	}
	else {
		printf("\n");
	}


	//----------- clean up ----------//
	del_stemmer();
	free(FileNames);
	SearchTerms.clear();
	TermResult.clear();
	SearchResult.clear();
	FinalResult.clear();


	return 0;
}