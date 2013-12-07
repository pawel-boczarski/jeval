#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "model_api.h"

void _plus()
{
	int a, b;
	char *tok1, *tok2;

	int offs = 0;

	SIGNAL_OK();

	if( (tok1 = get_token_from_end(1)) && (tok2 = get_token_from_end(2)) )
	{
		char *tok = malloc(50);
		a = atoi(tok1);
		b = atoi(tok2);

		sprintf(tok, "%d", a + b);
		realloc(tok, strlen(tok) + 1);

		pop_last_token();
		pop_last_token();
		pop_last_token();

		push_token(tok);
	}
}

void _new_plus()
{
	int a, b;
	char *tok1, *tok2;

	int offs = 0;

	tok1 = strdup(get_token_from_end(1));

	SIGNAL_WAIT();

	if(thread_state == TS_CANCEL)
		return;

	tok2 = get_token_from_end(0);

	SIGNAL_OK();

	int y, x; getyx(stdscr, y, x);
//	mvprintw(20, 20, "tok1='%s', tok2='%s'", tok1, tok2);

	if( tok1 && tok2 )
	{
		char *tok = malloc(50);
		a = atoi(tok1);
		b = atoi(tok2);

		sprintf(tok, "%d", a + b);

		tok = realloc(tok, strlen(tok) + 1);
//		mvprintw(21, 20, "tok='%s', token_last_no = %d", tok, get_token_last_no());

		pop_last_token();
		pop_last_token();
		pop_last_token();

//		mvprintw(22, 20, "token_last_no = %d", get_token_last_no());
//		mvprintw(y, x, "");

		push_token(tok);

	}
	else
	{
		pop_last_token();
		pop_last_token();
		pop_last_token();

		char *err = malloc(50);
		snprintf(err, 50, "tok1='%s',tok2='%s'", tok1, tok2);
		push_token(err);

	}

}

void _minus()
{
	int a, b;
	char *tok1, *tok2;

	SIGNAL_OK();

	if( (tok1 = get_token_from_end(1)) && (tok2 = get_token_from_end(2)) )
	{
		char *tok = malloc(50);
		a = atoi(tok1);
		b = atoi(tok2);

		sprintf(tok, "%d", a - b);
		realloc(tok, strlen(tok) + 1);

		pop_last_token();
		pop_last_token();
		pop_last_token();

		push_token(tok);
	}
}

void _mul()
{
	int a, b;
	char *tok1, *tok2;

	SIGNAL_OK();

	if( (tok1 = get_token_from_end(1)) && (tok2 = get_token_from_end(2)) )
	{
		char *tok = malloc(50);
		a = atoi(tok1);
		b = atoi(tok2);

		sprintf(tok, "%d", a * b);
		realloc(tok, strlen(tok) + 1);

		pop_last_token();
		pop_last_token();
		pop_last_token();

		push_token(tok);
	}		
}

void _div()
{
	int a, b;
	char *tok1, *tok2;

	SIGNAL_OK();

	if( (tok1 = get_token_from_end(1)) && (tok2 = get_token_from_end(2)) )
	{
		char *tok = malloc(50);
		a = atoi(tok1);
		b = atoi(tok2);

		sprintf(tok, "%d", a / b);
		realloc(tok, strlen(tok) + 1);

		pop_last_token();
		pop_last_token();
		pop_last_token();

		push_token(tok);
	}		
}

void _addall()
{
	int acc = 0;
	char *tok;

	SIGNAL_OK();

	int no_tok = 1;

	tok = get_token_from_end(1);
	int last_token_no = get_token_last_no();

	if(strcmp(tok, "]") != 0) return;

	for(no_tok = 1 ; no_tok <= last_token_no; ++no_tok)
	{
		tok = get_token_from_end(no_tok);

		if(strcmp(tok, "[") == 0)
			goto found_bra;
	}

	return;

found_bra:
	pop_last_token();	// addall
	pop_last_token();	// ]
	while(strcmp(get_token_from_end(0), "[") != 0)
	{
		tok = get_token_from_end(0);

		acc += atoi(tok);
		pop_last_token();
	}

	pop_last_token();	// [
	
	char *new_token = calloc(1, 50);

	snprintf(new_token, 50, "%d", acc);

	push_token(new_token);
}

struct op ops[] = {
	{ "+", &_plus },
	{ "-", &_minus },
	{ "*", &_mul },
	{ "-", &_div },
	{ "++", &_new_plus },
        { "addall", &_addall },
	{ NULL, NULL }
};
