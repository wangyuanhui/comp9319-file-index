bool DEBUG = false;

// word max length
#define WORDLEN		259
#define POSTLEN		20 * 1000
#define PATHLEN		1024
#define HALFA		"indexA"
#define HALFB		"indexB"
#define INDEX		"index"
#define WORD		"word"
#define INDEXBLOCK	100

// stemmer object
struct sb_stemmer * Stemmer;

// num of terms
int TermNum;							// always

//------ index.c ------//

// file names (sorted by map)
char * * FileNames;						// always
// num of file (txt,small-index)
int FileNum;							// always
// current file : start with 0
int FileCur;							// always

// WordList for file-parse small-index-create
std::map<std::string, int> WordList;	// temp


//------ merge.c -----//

// file pointer List for half small-index
FILE * * FPS;							// temp

// IndexRecord
struct Record {							// temp
	char * word;
	int file;
	int freq;
};
// IndexRecordArray
struct Record * * RecordArray;			// temp

// post item
struct Post {							// temp
	int file;
	int freq;
	struct Post * next;
};
struct Post * PostHead;					// temp
struct Post * PostTail;					// temp

// size of main-index
unsigned long SizeMainIndex;			// always

//------ find.c -----//

// search terms
std::map<std::string, char> SearchTerms;// always

// one term search result
	// file-num, freq
std::map<int, int> TermResult;			// temp

// yet all terms result (unsorted)
	// file-num, freq
std::map<int, int> SearchResult;		// always

// final result
std::vector<std::pair<int,int>> FinalResult;	//always

// last search start pos in word-index
	// cause search terms are ordered
	// so, next term just start from previours start-position
unsigned long LastLow = 0;	
// start and end pos for main-index
unsigned long Start;
unsigned long End;
// if search-term not exits, no result at all !
	// controled by a3find.c: search_main_index()
bool HasResult = true;					// always
