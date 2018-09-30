import numpy as np
import pickle as pkl
import matplotlib.pyplot as plt

fname = "dataVelib.pkl"
f= open(fname,'rb')
data = pkl.load(f)
print(len(data))

# On retire tous les champs ayant un number non compris entre 1 et 20
to_remove = []
for i in data:
    if not 1 <= i['number'] // 1000 <= 20:
        to_remove.append(i)
        
for i in to_remove:
    data.remove(i)

l = []

# Creation d'une liste pour la matrice
for i in data:
    l.append([i["address"], i["alt"], i["position"], i["number"], i["bike_stands"], i["available_bike_stands"]])

# Matrice avec 3 coordonnees geo, arrondissement, place total, place disponibles
m_geo = np.array(l)

# On somme le nombre total de ligne ayant le meme arrondissement
arrondissement = [0 for i in range(20)]
altitudes= []
pleins = []

for i in data:
    arrondissement[i["number"]//1000-1] += 1 # -1 parce que les arrondissements vont de 1 a 20 et la liste va de 0 a 19
    altitudes.append(i["alt"])
    pleins.append(1 if i['available_bike_stands'] == 0 else 0)
    
arrondissement = [arrondissement[i]/990 for i in range(len(arrondissement))] # On sort les probas 
#print(arrondissement)
#print(altitudes)
nIntervalles = 100
res = plt.hist(altitudes, nIntervalles)
catAlt = np.floor((data[:,1].max()-data[:,1].min()) / (res[1][1]-res[1][0]))


print()
#for i in range(0, len(def_interval), 2):
#   print(i)

print(res[0]) # effectif dans les intervalles
print(res[1]) # definition des intervalles (ATTENTION: 31 valeurs)
#print(altitudes)

print(len(data))
#pkl.dump(data,f) # penser à sauver les données pour éviter de refaire les opérations
f.close()