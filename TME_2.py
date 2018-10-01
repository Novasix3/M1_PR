import numpy as np
import matplotlib.pyplot as plt
import matplotlib.mlab as mlab
import math

#Ecrivez une fonction bernoulli : float -> int qui prend en argument un paramètre réel p compris entre 0 et 1, 
#et qui renvoie soit 1 (succès) avec la probabilité p, soit 0 (échec) avec la probabilité 1-p.

def bernoulli(p):
    return 1 if np.random.random() < p else 0 #on utilise np.random.random pour générer un float entre 0 et 1

print(bernoulli(0.5))
print(bernoulli(0.5))
print(bernoulli(0.2))
print(bernoulli(0.2))
print(bernoulli(0.8))
print(bernoulli(0.8))


#Ecrivez une fonction binomiale : int x float -> int qui prend en argument un paramètre entier n 
#et un paramètre réel p et qui renvoie un nombre tiré aléatoirement selon la distribution binomiale B(n,p).

def binomiale(n, p):
    return np.random.binomial(n, p) #on utilise la fonction binomial de numpy, qui fait la même chose que bernoulli utilisé n fois avec p comme proba de succès

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
        tab[i] *= binomiale(n, 0.5) #on utilise binomiale défini précédemment avec n épreuves et une proba de succès de 0.5
    print("Le nombre d'éléments différents est : ",len(set(tab)))
    plt.hist (tab, n)
    plt.show()

#Galton(10)
#Galton(50)
#Galton(100)

#Ecrivez une fonction normale : int x float -> float np.array qui, étant donné un paramètre entier k impair 
#et un paramètre réel sigma renvoie l'array numpy des k valeurs yi. Afin que l'array numpy soit bien symmétrique, on lèvera une exception si k est pair 

#fonction de calcul de la densité en un point x avec un écart type défini
def densite_normale(x, sigma):
    return 1/(sigma * np.sqrt(2 * np.pi)) * np.exp( - x**2 / (2 * sigma**2))

#fonction calculant la loi normale pour un échantillon de taille k et un écart type sigma, le mu étant défini comme 0 dans l'exercice
def normale (k, sigma):
    if k % 2 == 0:
        raise ValueError ( 'le nombre k doit etre impair' )
    #tab = [i for i in range(0, k)]
    #for i in range(0, len(tab)):
    #    tab[i] *= 1/(sigma * np.sqrt(2 * np.pi)) * np.exp( - (tab[i])**2 / (2 * sigma**2))
    #print(tab)
    #count, bins, ignored = plt.hist(tab, 50, density=True)
    #plt.plot(tab, linewidth=2, color='r')
    #plt.show()
    espace_points = int(sigma * 4 / k) #espace entre chaque points
    points_xi = [- 2 * sigma + espace_points * i for i in range(k)] #tableau de l'ensemble des points, formant la courbe du plot
    tab = [densite_normale(i, sigma) for i in points_xi] #tableau de l'ensemble des valeurs modifiés en utilisant la fonction densite_normale
    plt.plot(tab)
    plt.show()
    return tab
    
    
#normale(30001, 1)


#Ecrivez une fonction proba_affine : int x float -> float np.array qui, comme dans la question précédente, 
#va générer un ensemble de k points yi, i=0,...,k−1, représentant cette distribution (paramétrée par sa pente slope). 
#On supposera que l'entier k est impair.

def proba_affine (k, slope):
    if k % 2 == 0:
        raise ValueError ( 'le nombre k doit etre impair' )
    if abs ( slope  ) > 2. / ( k * k ):
        raise ValueError ( 'la pente est trop raide : pente max = ' +
        str ( 2. / ( k * k ) ) )
    tab = [i for i in range(0, k)]
    for i in range(0, len(tab)):
        tab[i] = 1/k + (i - (k - 1)/2) * slope
    plt.figure()              
    plt.plot(tab)
    plt.show()
    return tab
        
#proba_affine(21, 0.0003)

#Ecrivez une fonction Pxy : float np.array x float np.array -> float np.2D-array qui, 
#étant donné deux tableaux numpy de nombres réels à 1 dimension générés par les fonctions 
#des questions précédentes et représentant deux distributions de probabilités P(A) et P(B), 
#renvoie la distribution jointe P(A,B) sous forme d'un tableau numpy à 2 dimensions de nombres réels, 
#en supposant que A et B sont des variables aléatoires indépendantes.

def Pxy(PA, PB):
    tab = np.ones((len(PA), len(PB)))
    for i in range(0, len(PA)):
        for j in range(len(PB)):
            #print("PA[", i, "] = ", PA[i], ", PB[",j,"] = ", PB[j])
            tab[i][j] = PA[i] * PB[j]
    return tab

#PA = np.array([0.2, 0.7, 0.1])
#PB = np.array([0.4, 0.4, 0.2])
#print(Pxy(PA, PB))

#fonction définie par le professeur

def dessine (P_jointe):
    print(P_jointe)
    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')
    x = np.linspace ( -3, 3, P_jointe.shape[0] )
    y = np.linspace ( -3, 3, P_jointe.shape[1] )
    X, Y = np.meshgrid(x, y)
    ax.plot_surface(X, Y, P_jointe, rstride=1, cstride=1 )
    ax.set_xlabel('A')
    ax.set_ylabel('B')
    ax.set_zlabel('P(A) * P(B)')
    plt.show ()

#fin

#Le code ci-dessus permet d'afficher en 3D une probabilité jointe générée par la fonction précédente. 
#Exécutez-le avec une probabilité jointe résultant de la combinaison d'une loi normale 
#et d'une distribution affine. Si le contenu de la fenêtre est vide, 
#redimensionnez celle-ci et le contenu devrait apparaître. 
#Cliquez à la souris à l'intérieur de la fenêtre et bougez la souris en gardant le bouton appuyé 
#afin de faire pivoter la courbe. Observez sous différents angles cette courbe. 
#Refaites l'expérience avec une probaiblité jointe résultant de deux lois normales. 
#Essayez de comprendre ce que signifie, visuellement, l'indépendance probabiliste. 
#Vous pouvez également recommencer l'expérience avec le logarithme des lois jointes. 




#PA = normale(51, 80)
#print("aaaaaaaa",PA)
#PB = proba_affine(51, 0.0001)
#print("bbbbbbbbbbbbbbbb",PB)
#PAB = Pxy(PA, PB)
#print(PAB)
#dessine(PAB)

#PA = normale(21, 1)
#PB = normale(21, 3)
#PAB = Pxy(PA, PB)
#print(PAB)
#dessine(PAB)

#Pas réussi à trouver comment faire marcher dessine

def get_YZ(P_XYZT):
    tab = np.ones((2,2))
    for x in range(0,2):
        for y in range(0,2):
            total = 0
            for i in range(0,2):
                for j in range(0,2):
                    total += P_XYZT[i][x][y][j]
            tab[x][y] = total
            
    return tab
            
P_XYZT = np.array([[[[ 0.0192,  0.1728],
                     [ 0.0384,  0.0096]],

                    [[ 0.0768,  0.0512],
                     [ 0.016 ,  0.016 ]]],

                   [[[ 0.0144,  0.1296],
                     [ 0.0288,  0.0072]],

                    [[ 0.2016,  0.1344],
                     [ 0.042 ,  0.042 ]]]])

print(get_YZ(P_XYZT))

def get_XTcondYZ(P_XYZT):
    tab = np.ones((2,2,2,2))
    YT = get_YZ(P_XYZT)
    for x in range(0,2):
        for y in range(0,2):
            for i in range(0,2):
                for j in range(0,2):
                    tab[x][y][i][j] = P_XYZT[x][y][i][j] / YT[y][i]
            
    return tab

print(get_XTcondYZ(P_XYZT))






































