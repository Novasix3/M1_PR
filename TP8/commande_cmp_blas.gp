#set title ""
#set xtics 1 
#set size 0.5,0.5
set logscale x 2 
set logscale y 
set xlabel "N (log scale)"
set ylabel "Gflop/s (log scale)"
#set yrange[0:1]

set key top left  

set term jpeg 
set output 'courbe_cmp_blas.jpg' 

plot 'data_matmul_BLAS_NUM_THREADS1.txt' using 1:2 title "BLAS (1 coeur)" with linespoints, \
     'data_matmul_BLAS_NUM_THREADS12.txt' using 1:2 title "BLAS (12 coeurs)" with linespoints, \
     'data_matmul_CUBLAS.txt' using 1:2 title "CUBLAS (calcul & comms)" with linespoints, \
     'data_matmul_CUBLAS.txt' using 1:3 title "CUBLAS (calcul seul)" with linespoints 

