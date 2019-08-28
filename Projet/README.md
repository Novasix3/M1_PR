#projet HPC

Ceci est le projet de parallélisation d'un path-tracer, réalisé par Antoine MAS et Yoann VALERI.

Il est composé de 4 parties (OpenMPI, OpenMPI + OPenMP, SIMD par OpenMP, SIMD par AVX), mais la dernière ne fonctionne pas parfaitement, le rendu est mauvais.

Les fichiers sources sont dans le dossier "src", les mesures de temps effectuées sur les machines de la PPTI sont dans le dossier "time" (mais nous n'avons pas pu faire les tests pour les deux dernières parties, par manque de temps et car les machines étaient occupées). Les configurations de nos machines sont dans le dossier "config", le rapport dans le dossier "rapport".

## faire des test
Il est nécessaire de définir un fichier "hostfile", au moins vide, dans le dossier principal, pour que les tests fonctionnent.
Les commandes possibles sont :
 - "make partie", avec partie étant "seq", "mpi", "hybride", "simd", "avx";
 - "make type {nb=X} {samples=Y}", avec type étant "small", "mid", "big", "nb" étant le nombre de processus (2 si rien précisé), et "samples" le nombre de samples (200 si rien précisé).

 Quelques exemples :
  - "make mpi", "make small nb=4 samples=400"
  - "make hybride", "make mid nb=2 samples=200"
  - "make simd", "make big nb=2 samples=200"
  - "make avx", "make small nb=4"
  - "make hybride", "make mid"

Après utilisation de ces commandes, l'image résultante est disponible dans le dossier "image".