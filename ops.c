#define SIGNAL_OK() (thread_state = TS_OK)
#define SIGNAL_WAIT() do { thread_state = TS_WAITING; do { \
		nanosleep(&sleep_interval, NULL); \
		} while(thread_state == TS_WAITING); } while(0)

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
