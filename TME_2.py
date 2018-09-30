import numpy as np
import pickle as pkl
import matplotlib.pyplot as plt
from lib2to3.tests.data.infinite_recursion import ptrdiff_t

#Ecrivez une fonction bernoulli : float -> int qui prend en argument un paramètre réel p compris entre 0 et 1, 
#et qui renvoie soit 1 (succès) avec la probabilité p, soit 0 (échec) avec la probabilité 1-p.

def bernoulli(p):
    return 1 if np.random.random() < p else 0

print(bernoulli(0.5))
print(bernoulli(0.5))
print(bernoulli(0.2))
print(bernoulli(0.2))
print(bernoulli(0.8))
print(bernoulli(0.8))


#Ecrivez une fonction binomiale : int x float -> int qui prend en argument un paramètre entier n 
#et un paramètre réel p et qui renvoie un nombre tiré aléatoirement selon la distribution binomiale B(n,p).

def binomiale(n, p):
    return np.random.binomial(n, p)

print(binomiale(10,0.5))
print(binomiale(10,0.5))
print(binomiale(50,0.2))
print(binomiale(50,0.2))
print(binomiale(100,0.8))
print(binomiale(100,0.8))

#La répartition des billes dans les boites suit donc une loi binomiale B(n,0.5). 
#Ecrivez un script qui crée un array numpy de 1000 cases dont le contenu correspond à 1000 instanciations de la loi binomiale B(n,0.5). 
#Afin de voir la répartition des billes dans la planche de Galton, tracez l'histogramme de ce tableau.

def Galton(n):
    tab = np.ones(1000) 
    for i in range(0, len(tab)):
        tab[i] *= binomiale(n, 0.5)
    print("Le nombre d'éléments différents est : ",len(set(tab)))
    plt.hist (tab, n)
    plt.show()

#Galton(10)
#Galton(50)
#Galton(100)

#Ecrivez une fonction normale : int x float -> float np.array qui, étant donné un paramètre entier k impair 
#et un paramètre réel sigma renvoie l'array numpy des k valeurs yi. Afin que l'array numpy soit bien symmétrique, on lèvera une exception si k est pair 

def normale (k, sigma):
    if k % 2 == 0:
        raise ValueError ( 'le nombre k doit etre impair' )
    tab = np.random.normal(0, sigma, k)
    print(tab)
    count, bins, ignored = plt.hist(tab, 50, density=True)
    plt.plot(bins, 1/(sigma * np.sqrt(2 * np.pi)) * np.exp( - bins**2 / (2 * sigma**2)), linewidth=2, color='r')
    plt.show()
    
normale(30001, 1)


#Ecrivez une fonction proba_affine : int x float -> float np.array qui, comme dans la question précédente, 
#va générer un ensemble de k points yi, i=0,...,k−1, représentant cette distribution (paramétrée par sa pente slope). 
#On supposera que l'entier k est impair.




























































