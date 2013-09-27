all:	librootcursor.so	\
	test_rootcursor

CC = gcc -g -Wall -Wextra -O3

CPP = g++ -g -Wall -Wextra -O3
CPPFLAGS = `root-config --cflags`

LIBS = `root-config --libs`

rootcursor = rootcursor.h rootcursor.cpp

librootcursor.so: $(rootcursor)
	$(CPP) $(rootcursor) $(CPPFLAGS) -fPIC -shared -o $@

test_rootcursor: librootcursor.so test_rootcursor.c
	$(CC) test_rootcursor.c -L. -lrootcursor $(LIBS) -o $@

clean:
	rm -f librootcursor.so
	rm -f test_rootcursor

