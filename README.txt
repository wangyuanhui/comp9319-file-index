1.
code files: a3search.cpp, a3data.c, a3help.c, a3stem.c,
			a3index.c, a3merge.c, a3find.c
main function is in a3search.cpp

2.
code-walkthrough

a3search.cpp:
(this is the entrance of program)
include all libraries
include all other c code
receive user input arguments
get all text-files' names and sort them
(in program each text-file is represented in number from 0)
create index files if needed
searching
output result

a3data.c
contains all the global variables

a3help.c
contains all the helper functions

a3index.c
(this is the first step building indexes, create index for each text-file)
while reading each text-file, parse the contents of this file
(extract all the words and store word and its frequency into a word-list, each word is stemmed, and word-list is ordered)
write word-list into the index-file of current text-file

a3merge.c
(this is the second step building indexes, 
first, merge two halves of all the indexes into indexA and indexB.
then, merge these two into a final index called main-index.
finally, create a word-index for main-index, which store address of words in main-index)

a3find.c
sort all search terms.
for each term, first, lookup word-index to find the start and end position in main-index
(search word-index using binary search.
After each term, the next search will start at last start-point,
because terms are sorted and records in word-index is also sorted)
get start word/address and end word/address for main-index
search main-index in this range
if there is no result for this term, set flag HasResult to false, and no more further searches, final result is none
otherwise, sort all result, 
first by frequency in decresing order and by file-name in increasing order


3.
index detail:

main-index: 
record fomart:	"word:file,freq file,freq \n"
				e.g. "apple:2,3 3,5 9,1 \n"
all record are in ascending order by word

word-index:
record fomart:	"word:address \n"
				e.g. "apple:37268      \n"
				(each record has 270 bytes, 259 for word, 9 for address)
all record are in ascending order of word
address is the start position of word in main-index
record every 100th word of main-index
so words are 101th, 201th, 301th ...... of main-index