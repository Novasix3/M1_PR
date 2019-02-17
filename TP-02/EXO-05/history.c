#include<stdlib.h>
#include<stdio.h>

#include "history.h"
#include "list.h"

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
  return container_of(c->lh.prev, struct commit, lh);
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
  if(h->commit_list->lh.prev == h->commit_list->lh.next)
    printf("Historique vide !\n\n");
  //h->commit_list->display(h->commit_list);
  struct list_head* pos;
  struct commit* c;
  list_for_each(pos, &(h->commit_list->lh)){
    c = container_of(pos, struct commit, lh);
    display_commit(c);
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
  struct list_head* pos;
  struct commit* c = NULL;
  list_for_each(pos, &(h->commit_list->major_list)){
    c = container_of(pos, struct commit, major_list);
    if(c->version.major == major)
      break;
  }
  if(c == NULL){
    printf("Not here !!!\n");
    return;
  }
  list_for_each(pos, &(c->lh)){
    c = container_of(pos, struct commit, lh);
    if(c->version.major != major)
      break;
    if(c->version.minor == minor){
      display_commit(c);
      return;
    }
  }
  printf("Not here !!!\n");
}

void freeHistory(struct history* h){
  struct list_head* pos, *n;
  struct commit* c = NULL;
  list_for_each_safe(pos, n, &(h->commit_list->lh)){
    c = container_of(pos, struct commit, lh);
    //printf("free de c = \n");
    //display_commit(c);
    freeCommit(c);
  }
  free(h->commit_list);
  free(h);
}