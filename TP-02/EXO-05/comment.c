#include<stdlib.h>
#include<stdio.h>
#include<string.h>

#include"comment.h"

struct comment *new_comment(int title_size, char *title, int author_size, char *author,	int text_size, char *text){
	struct comment *c = (struct comment *) malloc(sizeof(struct comment));

	if(title_size <= 0){
		int temp = 0;
		while(*(title + temp) != '\0')
			temp++;
		c->title_size = temp+1;
	}else
		c->title_size = title_size;

	if(! (c->title = malloc(c->title_size)))
		return NULL;
	memcpy(c->title, title, c->title_size);




	if(author_size <= 0){
		int temp = 0;
		while(*(author + temp) != '\0')
			temp++;
		c->author_size = temp+1;
	}else
		c->author_size = author_size;

	if(! (c->author = malloc(c->author_size)))
		return NULL;
	memcpy(c->author, author, c->author_size);



	if(text_size <= 0){
		int temp = 0;
		while(*(text + temp) != '\0')
			temp++;
		c->text_size = temp+1;
	}else
		c->text_size = text_size;

	if(! (c->text = malloc(c->text_size)))
		return NULL;
	memcpy(c->text, text, c->text_size);

	return c;
}

void display_comment(struct comment *c)
{
	printf("'%s' from '%s' \"%s\"\n", c->title, c->author, c->text);
}

