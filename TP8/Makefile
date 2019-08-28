EXECS = matmul 

all: $(EXECS)

matmul: matmul.c
	gcc -O3 -o $@ $< -fopenmp -lopenblas

clean: 
	rm -rf $(EXECS)

.phony: all
