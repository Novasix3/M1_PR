#ifndef COMMIT_H
#define COMMIT_H

#include "version.h"
#include "list.h"

struct commit;

struct commit_ops {
	void (*display)(struct commit*);
	void (*extract)(struct commit*);
};

struct commit {
	unsigned long id;
	struct version version;
	char *comment;
	struct list_head lh;
	struct list_head major_list;
	struct commit* major_parent;
	struct commit_ops* ops;
};

struct commit_ops *construct_minor();

struct commit_ops *construct_major();

struct commit *new_commit(unsigned short major, unsigned long minor, char *comment);

struct commit *add_minor_commit(struct commit *from, char *comment);

struct commit *add_major_commit(struct commit *from, char *comment);

struct commit *del_commit(struct commit *victim);

void extract_minor(struct commit* victim);

void extract_major(struct commit* victim);

void display_commit(struct commit *c);

void display_minor_commit(struct commit *c);

void display_major_commit(struct commit *c);

struct commit *commitOf(struct version *version);

void freeCommit(struct commit* c);

#endif
