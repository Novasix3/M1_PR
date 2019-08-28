#include<stdlib.h>
#include<stdio.h>

#include"commit.h"

static int nextId = 0;

struct commit_ops *construct_minor()
{
	struct commit_ops *c =
	    (struct commit_ops *)malloc(sizeof(struct commit_ops));
	c->display = &display_minor_commit;
	c->extract = &extract_minor;
	return c;
}

struct commit_ops *construct_major()
{
	struct commit_ops *c =
	    (struct commit_ops *)malloc(sizeof(struct commit_ops));
	c->display = &display_major_commit;
	c->extract = &extract_major;
	return c;
}

/**
  * new_commit - alloue et initialise une structure commit correspondant aux
  *              parametres
  *
  * @major: numero de version majeure
  * @minor: numero de version mineure
  * @comment: pointeur vers une chaine de caracteres contenant un commentaire
  *
  * @return: retourne un pointeur vers la structure allouee et initialisee
  */
struct commit *new_commit(unsigned short major, unsigned long minor,
			  char *comment)
{
	struct version v = {.major = major,.minor = minor,.flags = 0 };
	struct commit *c = (struct commit *)malloc(sizeof(struct commit));
	c->id = nextId++;
	c->version = v;
	c->comment = comment;
	INIT_LIST_HEAD(&(c->lh));
	INIT_LIST_HEAD(&(c->major_list));
	c->major_parent = c;
	return c;
}

/**
  * insert_commit - insere sans le modifier un commit dans la liste doublement
  *                 chainee
  *
  * @from: commit qui deviendra le predecesseur du commit insere
  * @new: commit a inserer - seuls ses champs next et prev seront modifies
  *
  * @return: retourne un pointeur vers la structure inseree
  */
static struct commit *insert_commit(struct commit *from, struct commit *new)
{
	list_add(&(new->lh), &(from->lh));
	return new;
}

/**
  * add_minor_commit - genere et insere un commit correspondant a une version
  *                    mineure
  *
  * @from: commit qui deviendra le predecesseur du commit insere
  * @comment: commentaire du commit
  *
  * @return: retourne un pointeur vers la structure inseree
  */
struct commit *add_minor_commit(struct commit *from, char *comment)
{
	struct commit *c =
	    new_commit(from->version.major, from->version.minor + 1, comment);
	c->major_parent = from->major_parent;
	c->ops = construct_minor();
	return insert_commit(from, c);
}

/**
        * add_major_commit - genere et insere un commit correspondant a une version
  *                    majeure
  *
  * @from: commit qui deviendra le predecesseur du commit insere
  * @comment: commentaire du commit
  *
  * @return: retourne un pointeur vers la structure inseree
  */
struct commit *add_major_commit(struct commit *from, char *comment)
{
	struct commit *c = new_commit(from->version.major + 1, 0, comment);
	c->major_parent = c;
	c->ops = construct_major();
	list_add(&(c->major_list), &(from->major_parent->major_list));
	return insert_commit(from, c);
}

/**
  * del_commit - extrait le commit de l'historique
  *
  * @victim: commit qui sera sorti de la liste doublement chainee
  *
  * @return: retourne un pointeur vers la structure extraite
  */
struct commit *del_commit(struct commit *victim)
{
	victim->ops->extract(victim);
	return victim;
}

/**
  * extract_major - extrait le commit majeur de l'historique, ainsi que les mineurs partageant le même majeur
  *
  * @victim: commit qui sera sorti de la liste doublement chainee
  *
  * @return: retourne un pointeur vers la structure extraite
  */
void extract_major(struct commit *victim)
{
	struct list_head *pos, *n;
	struct commit *c = NULL;
	list_for_each_safe(pos, n, &(victim->lh)) {
		c = container_of(pos, struct commit, lh);
		if (c->version.major != victim->version.major)
			break;
		freeCommit(c);
	}
	freeCommit(victim);
}

/**
  * extract_minor - extrait le commit mineur de l'historique
  *
  * @victim: commit qui sera sorti de la liste doublement chainee
  *
  * @return: retourne un pointeur vers la structure extraite
  */
void extract_minor(struct commit *victim)
{
	freeCommit(victim);
}

/**
  * display_commit - affiche un commit : "2:  0-2 (stable) 'Work 2'"
  *
  * @c: commit qui sera affiche
  */
void display_commit(struct commit *c)
{
	c->ops->display(c);
}

/**
  * display_minor_commit - affiche un commit : "2:  0-2 (stable) 'Work 2'"
  *
  * @c: commit qui sera affiche
  */
void display_minor_commit(struct commit *c)
{
	printf("%lu: ", c->id);
	display_version(&c->version, &is_unstable_bis);
	printf("'%s'\n", c->comment);
}

/**
  * display_major_commit - affiche un commit : "### version 1 : 'Realse 1' ###"
  *
  * @c: commit qui sera affiche
  */
void display_major_commit(struct commit *c)
{
	printf("%lu: ### version ", c->id);
	display_version(&c->version, &is_unstable_bis);
	printf(" : '%s' ###\n", c->comment);
}

/**
  * commitOf - retourne le commit qui contient la version passee en parametre
  *
  * @version: pointeur vers la structure version dont on recherche le commit
  *
  * @return: un pointeur vers la structure commit qui contient 'version'
  *
  * Note:      cette fonction continue de fonctionner meme si l'on modifie
  *            l'ordre et le nombre des champs de la structure commit.
  */
struct commit *commitOf(struct version *version)
{
	return container_of(version, struct commit, version);
}

void freeCommit(struct commit *c)
{
	list_del(&c->lh);
	free(c->ops);
	free(c);
}
