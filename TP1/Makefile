.PHONY : all clean check
.ONESHELL:

CHECK_PATCH=./checkpatch.pl --no-tree

all : exo1 exo2 ex_send ex_ssend ex_ssend_correcte

%.o : %.c %.h
	gcc -g -Wall -c $<

exo1 : exo1.c exo1.o
	gcc -g -Wall $^ -o $@

exo2 : exo2.c exo2.o
	gcc -g -Wall $^ -o $@

ex_send : ex_send.c ex_send.o
	gcc -g -Wall $^ -o $@

ex_ssend : ex_ssend.c ex_ssend.o
	gcc -g -Wall $^ -o $@

ex_ssend_corecte : ex_ssend_correcte.c ex_ssend_correcte.o
	gcc -g -Wall $^ -o $@

check : 
	for f in *.c *.h ; do
		$(CHECK_PATCH) -f $$f
	done

clean :
	rm -rf *.o exo1 exo2 ex_send ex_ssend ex_ssend_correcte
