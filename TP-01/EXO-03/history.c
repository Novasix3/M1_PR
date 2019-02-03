#include<stdlib.h>
#include<stdio.h>

#include"history.h"

/**
  * new_history - alloue, initialise et retourne un historique.
  *
  * @name: nom de l'historique
  */
struct history *new_history(char *name)
{
  struct history* h = (struct history*) malloc(sizeof(struct history));
  h->commit_count = 0;
  h->name = name;
  h->commit_list = new_commit(0, 0, NULL);
	return h;
}

/**
  * last_commit - retourne l'adresse du dernier commit de l'historique.
  *
  * @h: pointeur vers l'historique
  */
struct commit *last_commit(struct history *h)
{
  struct commit* c = h->commit_list;
  if(c == NULL)
    return NULL;
	while(c->next != NULL && c->next->id != 0)
    c = c->next;
	return c;
}

/**
  * display_history - affiche tout l'historique, i.e. l'ensemble des commits de
  *                   la liste
  *
  * @h: pointeur vers l'historique a afficher
  */
void display_history(struct history *h)
{
  printf("Historique de '%s' : \n", h->name);
  struct commit* cur = h->commit_list;
  if(cur->id == 0 && cur->next->id == 0)
    printf("Historique vide !\n\n");
  else{
    if(cur->id == 0)
      cur = cur->next;
    while(cur->id != 0){
      display_commit(cur);
      cur = cur->next;
    }
  }
  printf("\n");
}

/**
  * infos - affiche le commit qui a pour numero de version <major>-<minor> ou
  *         'Not here !!!' s'il n'y a pas de commit correspondant.
  *
  * @major: major du commit affiche
  * @minor: minor du commit affiche
  */
void infos(struct history *h, int major, unsigned long minor)
{
  struct version* v = malloc(sizeof(struct version));
  v->major = major;
  v->minor = minor;
  struct commit* c = commitOf(v, h->commit_list);
  if(c == NULL)
    printf("Not here !!!\n");
  else
    display_commit(c);
}
