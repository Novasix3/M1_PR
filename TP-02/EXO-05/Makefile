.PHONY : all clean check
.ONESHELL:

CHECK_PATCH=./checkpatch.pl --no-tree

all : testHistory testComment

%.o : %.c %.h
	gcc -g -Wall -c $<

testHistory : testHistory.c version.o commit.o history.o
	gcc -g -Wall $^ -o $@

testComment : testComment.c comment.o
	gcc -g -Wall $^ -o $@

check : 
	for f in *.c *.h ; do
		$(CHECK_PATCH) -f $$f
	done

clean :
	rm -rf *.o testHistory testComment
