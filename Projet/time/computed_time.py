#!/usr/bin/python3

import sys

def computed_time(f):
    """
    Compute time for the first part 
    """
    sum = 0
    exe = 0
    for l in f:
        if "computed" in l:
            s = l.split()
            if float(s[0]) > exe:
                exe = float(s[0])
            sum += float(s[-1])
    return [sum, exe]

def diff_computed_time(f):
    """
    Compute time for the second part
    """
    sum1 = 0
    task1 = 0
    sum2 = 0
    task2 = 0
    for l in f:
        if "fini" in l:
            s = l.split()
            sum1 += float(s[6])
            task1 += int(s[8])
            sum2 += float(s[13])
            task2 += int(s[15])
    return [sum1, task1, sum2, task2]

if len(sys.argv) < 2:
    print("usage : " + sys.argv[0] + " <file_name>")
    exit()

f = open(sys.argv[2],"r")

if sys.argv[1] == "1":
    c = computed_time(f)
    n=int(input("Nombre de processus : "))
    print("calcul " + str(c[0]))
    print("execution " + str(c[1]))
    print("efficacite " + str(c[0] / c[1] / n))
elif sys.argv[1] == "2":
    c = diff_computed_time(f)
    d1 = c[0] / c[1]
    d2 = c[2] / c[3]
    print("time 1 : " + str(c[0]) + ", time 2 : " + str(c[2]) + ", comparaison : " + str(d1 / d2))

f.close()
