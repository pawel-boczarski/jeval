#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "model_api.h"

// warning - it passes wait but all internal op's right hand parameters
// need to come from stdin, not the command already typed in.
void call_op(void (*op)(thread_state_t *), thread_state_t *thrp)
{
	thread_state_t *new_thrp = calloc(1, sizeof(thread_state_t));

	new_thrp->state = TS_NONE;
	new_thrp->parent = thrp;

	pthread_create( new_thrp, NULL, op, new_thrp );

	WAIT_FOR_CHILD_FIRST_RESP(new_thrp);

	if(new_thrp->state == TS_OK)
	{	
		SIGNAL_OK(thrp);
		pthread_join( new_thrp->thread, NULL );
		free(new_thrp);
	}
	else
	while(new_thrp->state == TS_WAITING)
	{
		SIGNAL_WAIT(thrp);
		new_thrp->state = TS_RESPONSE;

		WAIT_FOR_CHILD_NEXT_RESP(new_thrp);
				
		if(new_thrp->state == TS_OK)
		{
			//SIGNAL_OK(thrp);
			pthread_join( new_thrp->thread, NULL );
			free(new_thrp);
		}
	}
}

void _plus(thread_state_t *thrp)
{
	int a, b;
	char *tok1, *tok2;

	int offs = 0;

	SIGNAL_OK(thrp);

	if( (tok1 = get_token_from_end(1)) && (tok2 = get_token_from_end(2)) )
	{
		char *tok = malloc(50);
		a = atoi(tok1);
		b = atoi(tok2);

		sprintf(tok, "%d", a + b);
		tok = realloc(tok, strlen(tok) + 1);

		pop_last_token();
		pop_last_token();
		pop_last_token();

		push_token(tok);
	}
}

void _new_plus(thread_state_t *thrp)
{
	int a, b;
	char *tok1, *tok2;

	int offs = 0;

	tok1 = strdup(get_token_from_end(1));

	SIGNAL_WAIT(thrp);

	if(thrp->state == TS_CANCEL)
		return;

	tok2 = get_token_from_end(0);

	SIGNAL_OK(thrp);

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

/*void _reverse_plus()
{
	char *tok1;
	char *tok2;

	SIGNAL_WAIT(thrp);
	SIGNAL_WAIT(thrp);

	tok1 = get_token_from_end(0);
	tok2 = get_token_from_end(1);

	SIGNAL_OK(thrp);

	if(tok1 && tok2)
	{
		char *tok = malloc(50);

		snprintf(tok, 50, "%d", atoi(tok1) + atoi(tok2));


		pop_last_token();
		pop_last_token();
		pop_last_token();

		push_token(tok);
	}
}*/

void _minus(thread_state_t *thrp)
{
	int a, b;
	char *tok1, *tok2;

	SIGNAL_OK(thrp);

	if( (tok1 = get_token_from_end(1)) && (tok2 = get_token_from_end(2)) )
	{
		char *tok = malloc(50);
		a = atoi(tok1);
		b = atoi(tok2);

		sprintf(tok, "%d", a - b);
		tok = realloc(tok, strlen(tok) + 1);

		pop_last_token();
		pop_last_token();
		pop_last_token();

		push_token(tok);
	}
}

void _mul(thread_state_t *thrp)
{
	int a, b;
	char *tok1, *tok2;

	SIGNAL_OK(thrp);

	if( (tok1 = get_token_from_end(1)) && (tok2 = get_token_from_end(2)) )
	{
		char *tok = malloc(50);
		a = atoi(tok1);
		b = atoi(tok2);

		sprintf(tok, "%d", a * b);
		tok = realloc(tok, strlen(tok) + 1);

		pop_last_token();
		pop_last_token();
		pop_last_token();

		push_token(tok);
	}		
}

void _div(thread_state_t *thrp)
{
	int a, b;
	char *tok1, *tok2;

	SIGNAL_OK(thrp);

	if( (tok1 = get_token_from_end(1)) && (tok2 = get_token_from_end(2)) )
	{
		char *tok = malloc(50);
		a = atoi(tok1);
		b = atoi(tok2);

		sprintf(tok, "%d", a / b);
		tok = realloc(tok, strlen(tok) + 1);

		pop_last_token();
		pop_last_token();
		pop_last_token();

		push_token(tok);
	}		
}

void _mod(thread_state_t *thrp)
{
	int a, b;
	char *tok1, *tok2;

	SIGNAL_OK(thrp);

	if( (tok1 = get_token_from_end(1)) && (tok2 = get_token_from_end(2)) )
	{
		char *tok = malloc(50);
		a = atoi(tok1);
		b = atoi(tok2);

		sprintf(tok, "%d", b % a);
		tok = realloc(tok, strlen(tok) + 1);

		pop_last_token();
		pop_last_token();
		pop_last_token();

		push_token(tok);
	}		
}

void _addall(thread_state_t *thrp)
{
	int acc = 0;
	char *tok;

	SIGNAL_OK(thrp);

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

void _nelem(thread_state_t *thrp)
{
	int n = get_token_last_no();

	SIGNAL_OK(thrp);

	char *new_token = calloc(1, 50);

	snprintf(new_token, 50, "%d", n);

	pop_last_token(); // nelem

	push_token(new_token);
}

void _rep(thread_state_t *thrp)
{
	SIGNAL_WAIT(thrp);

	if(thrp->state == TS_CANCEL)
		return;

	if(!get_token_from_end(2)) {
		SIGNAL_OK(thrp);
		return;
	}

	int repcount = atoi(get_token_from_end(2));

	int noper = 0, bra_not = 0;

	char *oper = get_token_from_end(0);
	//void (**op_tbl)(thread_state_t *);
	char **oper_tbl = calloc(1, sizeof(char*));

	if(strcmp(oper, "[") == 0)
	{
		noper = 0;
		bra_not = 1;

		do
		{
			SIGNAL_WAIT(thrp);
			oper = get_token_from_end(0);

			if(strcmp(oper, "]") == 0)
			break;

			noper++;

			oper_tbl = realloc(oper_tbl, noper * sizeof(char*));
			oper_tbl[noper - 1] = strdup(oper);
		} while(1);

	}
	else
	{
		noper = 1;
		oper_tbl[0] = strdup(oper);
	}

	if(noper == 0)
	{
		SIGNAL_OK(thrp);
		return;
	}

	//SIGNAL_OK(thrp);

	//void (*op)(thread_state_t *) = get_op(oper);

//	if(!op)
//		return;

	int j;
	for(j = 0; j < noper + (bra_not ? 2 : 0); j++)
		pop_last_token();

	//extern FILE *f;
	//fprintf(f, "after loop\n");

	pop_last_token();
	pop_last_token();

	for( ; repcount > 0 ; --repcount)
	{
		for(j = 0; j < noper; j++)
		{
			void (*op)(thread_state_t *) = get_op(oper_tbl[j]);
			char *oper_token = strdup(oper_tbl[j]);

			push_token(oper_token);

			if(op)
			{
				//call_op(op, thrp);
			//{
				thread_state_t *new_thrp = calloc(1, sizeof(thread_state_t));
			
				new_thrp->state = TS_NONE;
				new_thrp->parent = thrp;

				pthread_create( new_thrp, NULL, op, new_thrp );

				WAIT_FOR_CHILD_FIRST_RESP(new_thrp);

				if(new_thrp->state == TS_OK)
				{	
					SIGNAL_OK(thrp);
					pthread_join( new_thrp->thread, NULL );
					free(new_thrp);
				}
				else
				while(new_thrp->state == TS_WAITING)
				{
					// SIGNAL_WAIT(thrp);
					// need to supply next token from oper loop
					if(repcount > 0)
					{
						if(++j == noper)
						{
							j = 0;
							repcount --;
						}
					}
					if(repcount > 0)
					{
						char *oper_token_2 = strdup(oper_tbl[j]);
						push_token(oper_token_2);

						new_thrp->state = TS_RESPONSE;
					}
					else /* loop ended - wait for missing token from stdin */
					{
						SIGNAL_WAIT(thrp);
						new_thrp->state = TS_RESPONSE;	
					}

					WAIT_FOR_CHILD_NEXT_RESP(new_thrp);
	
					if(new_thrp->state == TS_OK)
					{
						//SIGNAL_OK(thrp);
						pthread_join( new_thrp->thread, NULL );
						free(new_thrp);
					}
				}
			//}
			}
		}
		//char *oper_token = strdup(oper);
		//push_token(oper);
		//call_op(op, thrp);
	}

	for(j = 0; j < noper; j++)
		free(oper_tbl[j]);
	free(oper_tbl);

	SIGNAL_OK(thrp);

}

void _if(thread_state_t *thrp)
{
	char *last_tok = strdup(get_token_from_end(1));

	if(!last_tok) {
		SIGNAL_OK(thrp);
		return;
	}
	else
	{
		SIGNAL_WAIT(thrp);
		SIGNAL_WAIT(thrp);

		char *tok_true = get_token_from_end(1);
		char *tok_false = get_token_from_end(0);
		char *tok_push=  NULL;

		if((strcmp(last_tok, "0") == 0) || (strcmp(last_tok, "0.0") == 0))
		{
			tok_push = strdup(tok_false);	
		}
		else
		{
			tok_push = strdup(tok_true);
		}

		SIGNAL_OK(thrp);

		pop_last_token();
		pop_last_token();
		pop_last_token();
		pop_last_token();

		push_token(tok_push);

		free(last_tok);
	}
}

void _nil(thread_state_t *thrp)
{
	SIGNAL_OK(thrp);

	pop_last_token();
}

void _chain_eval(thread_state_t *thrp)
{
	if(get_token_last_no() == 0)
	{
		SIGNAL_OK(thrp);
		return;
	}

	else
	{
		char *oper = get_token_from_end(1);

		if(oper)
		{
			pop_last_token();

			void *(*op)(thread_state_t *thrp) = get_op(oper);

			if(!op)
			{
				SIGNAL_OK(thrp);
				return;
			}
			else
				op(thrp); // note op is down to signal now
		}
	}
}

struct op ops[] = {
	{ "+", &_plus },
	{ "-", &_minus },
	{ "*", &_mul },
	{ "/", &_div },
	{ "mod", &_mod },
	{ "++", &_new_plus },
//	{ "r+", &_reverse_plus },
        { "addall", &_addall },
	{ "nelem", &_nelem },
	{ "rep", &_rep },
	{ "if", &_if },
	{ "nil", &_nil },
	{ ",", &_chain_eval },
	{ NULL, NULL }
};
