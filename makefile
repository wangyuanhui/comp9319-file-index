include mkinc.c
CFLAGS=-Ilibstemmer_c/include
all: tar libstemmer.o a3search
tar:
	tar -xf stem.tar
libstemmer.o: $(snowball_sources:.c=.o)
	$(AR) -cru $@ $^
a3search: a3search.cpp libstemmer.o
	g++ -std=c++11 a3search.cpp libstemmer.o -o a3search
clean:
	rm -rdf *.o a3search libstemmer_c
