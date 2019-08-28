EXECS =  ProduitTermeATerme_scal ProduitTermeATerme_omp ProduitTermeATerme_avx
CFLAGS= -O3 -ffast-math -fopenmp-simd -mavx2
OUT=output.txt 

all: $(EXECS)

ProduitTermeATerme_scal: ProduitTermeATerme.c
	gcc -S $(CFLAGS) -o $@.s $< ; grep -i mul $@.s
	gcc $(CFLAGS) -o $@ $<

ProduitTermeATerme_omp: ProduitTermeATermeOmp.c
	gcc -S $(CFLAGS) -o $@.s $< ; grep -i mul $@.s
	gcc $(CFLAGS) -o $@ $<

ProduitTermeATerme_avx: ProduitTermeATermeAvx.c
	gcc -S $(CFLAGS) -o $@.s $< ; grep -i mul $@.s
	gcc $(CFLAGS) -o $@ $<

exec: all 
	rm -rf $(OUT)
	(for e in $(EXECS) ; do echo "Running $$e ..." ; echo "" >> $(OUT) ; echo "Running $$e ..." >> $(OUT) ; ./$$e $$N >> $(OUT) ; done)

clean: 
	rm -rf $(EXECS) *~ *.s 

.phony: all
