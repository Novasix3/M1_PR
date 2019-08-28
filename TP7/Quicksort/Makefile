EXECS = QuickSort_seq QuickSort_omp
CFLAGS= -O3 -fopenmp

all: $(EXECS)

QuickSort_seq: QuickSort_seq.c
	gcc $(CFLAGS) -o $@ $<

QuickSort_omp: QuickSort_omp.c
	gcc $(CFLAGS) -o $@ $<


clean: 
	rm -rf $(EXECS) *~

.phony: all
