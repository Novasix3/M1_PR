##Exercice 1 : Mon premier appel système

### Q1
La fonction `syscall` permet d'invoquer un appel système à l'aide de son numéro et on passe les arguments de l'appel système à la fonction.
Un appel système est une demande de service au noyau.
La valeur de retour de la fonciton `syscall` est la valeur de retour de l'appel système. -1 indique une erreur et le code d'erreur est stocker dans errno.

## Exercice 2 (rootkit 4) : Détourner un signal

En cachant une module dans /proc il n'est plus visible de l'extérieur via /proc ni de la commande ps par exemple. Mais l'envoie d'un signal sur un processus caché dans /proc, va réussir. Dans le cas d'un PID inexistant l'information qu'il n'existe pas est renvoyer. Il suffi donc d'envoyer un signal (SIGCONT par exemple) à tous les PID possible et de comparer avec les process dans /proc pour retrouver les processus qui ont été cachés.
Nous allons donc dérouter l'appel système "kill", pour vérifié s'il n'est pas appeler sur un PID que l'on veut cacher.

Comme aucun appel système n'est exporté, nous utilisons la fonction `kallsyms_lookup_name` pour retrouver la table des syscalls. Cette table est protéger en écriture, nous changeons donc la vérification de la protection en écriture, le temps de remplacer le pointeur de fonction associé avec le syscall kill vers notre nouvelle fonction.

Le remplacement fonctionne correctement, mais l'appel à la fonction d'original ne fonctionne pas (page fault)...
De plus Greg dit que c'est pas bien :( => https://www.spinics.net/lists/newbies/msg59547.html
