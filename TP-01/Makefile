.PHONY : all clean check
.ONESHELL:

CHECK_PATCH=./checkpatch.pl --no-tree

all : exo1

%.o : %.c %.h
	gcc -g -Wall -c $<

exo1 : exo1.c exo1.o
	gcc -g -Wall $^ -o $@

check : 
	for f in *.c *.h ; do
		$(CHECK_PATCH) -f $$f
	done

clean :
	rm -rf *.o exo1
