import numpy as np
import pickle as pkl
from markov_tools import *

data = pkl.load(open("genome_genes.pkl", "rb"), encoding='latin1')

Xgenes = data.get("genes")  # Les genes, une array de arrays

Genome = data.get("genome")  # le premier million de bp de Coli

Annotation = data.get("annotation")  ##l'annotation sur le genome
##0 = non codant, 1 = gene sur le brin positif

### Quelques constantes
DNA = ["A", "C", "G", "T"]
stop_codons = ["TAA", "TAG", "TGA"]

# Loi gÃ©omÃ©trique :
# longueur moyenne = 200
# probabiltÃ© de rester = 1-a, probabiltÃ© de partir = a
# 1/a = 200 => a = 1/200
a = 1. / 200.

l = 0
for x in Xgenes:
	l += len(x) / 3.  # on compte par codon, pas par bp
l /= len(Xgenes)  # longueur moyenne
b = 1 / l

Pi = np.array([1, 0, 0, 0])
A = np.array([[1 - a, a, 0, 0],
              [0, 0, 1, 0],
              [0, 0, 0, 1],
              [b, 1 - b, 0, 0]])

print(len(Genome))
print(len(Annotation))

Binter = np.ndarray((1,4))
Binter[0][0] = len(Genome[Genome == 0])/len(Genome)
Binter[0][1] = len(Genome[Genome == 1])/len(Genome)
Binter[0][2] = len(Genome[Genome == 2])/len(Genome)
Binter[0][3] = len(Genome[Genome == 3])/len(Genome)

Bgene = np.ndarray((3,4))


# pour utiliser le modÃ¨le plus loin:
s, logp = viterbi(Genome, Pi, A, B)
