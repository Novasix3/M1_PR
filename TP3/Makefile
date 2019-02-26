.PHONY : all clean check
.ONESHELL:

CHECK_PATCH=./checkpatch.pl --no-tree

all : mandel

%.o : %.c %.h
	gcc -g -Wall -c $< -lm 

mandel : mandel.o
	gcc -g -Wall $^ -o $@ -lm 

check : 
	for f in *.c *.h ; do
		$(CHECK_PATCH) -f $$f
	done

clean :
	rm -rf *.o mandel
