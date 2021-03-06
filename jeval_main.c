#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
//#include <semaphore.h>

#include "model_api.h"

FILE *f;

volatile char *curbuf = 0;
volatile char **substrs = 0;

pthread_mutex_t stack_mutex;

// used for batch mode
char *lastbuf = 0;

int batch = 0;

thread_state_t current_task;

int wait_pos = -1;

struct timespec sleep_interval = { .tv_sec = 0, .tv_nsec = 1 };

int thread_state = 0;

int curbuf_len()
{
	if(!curbuf)
		return -1;
	else
		return strlen(curbuf);
}

void curbuf_append_char(char c)
{
	if(!curbuf)
	{
		curbuf = (char*)calloc(2, 1);
		curbuf[0] = c;
	}
	else
	{
		curbuf = realloc(curbuf, strlen(curbuf) + 2);
		curbuf[strlen(curbuf) + 1] = '\0';
		curbuf[strlen(curbuf)] = c;
	}
}

void curbuf_delete_char()
{
	if(!curbuf || strlen(curbuf) == 0)
		return;
	
	else
	{
		int s = strlen(curbuf);
		curbuf = realloc(curbuf, s);
		curbuf[s-1] = '\0';
	}
}

void tokenize()
{
	if(substrs != 0)
	{
		int i = 0;

		while(substrs[i] != NULL)
		{
			free(substrs[i]);
			substrs[i] = 0;
			i++;
		}
	}

	char *curbufcp = strdup(curbuf);

	free(substrs);

	substrs = (char**)calloc(2, sizeof(char*));

	substrs[0] = strtok(curbufcp, " ");
	if(substrs[0]) substrs[0] = strdup(substrs[0]);
	int i = 0;
	while(substrs[i] != 0)
	{
		++i;
		substrs = (char**)realloc(substrs, (i + 2) * sizeof(char*));
		//substrs[i+1] = NULL;
		substrs[i] = strtok(NULL, " ");
		if(substrs[i]) substrs[i] = strdup(substrs[i]);
	}

	free(curbufcp);
}

void rebuildstr()
{
	free(curbuf);

	curbuf = 0;
	int i =0;

	while(substrs[i] != NULL)
	{
		if(curbuf == 0)
			curbuf = (char*)calloc(1, strlen(substrs[i]) + 1);
		else
		{
			curbuf = (char*)realloc(curbuf, strlen(curbuf) + strlen(substrs[i]) + 2);
		}

		strcat(curbuf, " ");
		strcat(curbuf, substrs[i]);
		i++;
	}
}

void write_tokens(WINDOW *win)
{
	wclear(win);
	box(win, 0, 0);

	mvwprintw(win, 0, 2, "Tokens...");

	static char *tst[] = { "NONE", "WAITING", "OK", "RESPONSE", "CANCEL" };

	mvwprintw(win, 0, 20, "[ thread state : '%s' ]", tst[current_task.state]);	

	if(!substrs)
	{
		mvwprintw(win, 1, 2, "No subtokens!");
	}
	else
	{
		int i =0;
		while(substrs[i] != 0)
		{
			mvwprintw(win, i+1, 2, "%d: '%s'", i, substrs[i]);
			++i;
		}
	}
	wrefresh(win);
}

char* get_token_from_end(int i)
{
	if(pthread_mutex_lock(&stack_mutex) == 0)
	{
	if(!substrs) return pthread_mutex_unlock(&stack_mutex), NULL;

	int last = 0; for( ; substrs[last] ; ++last); --last;

	if(last - i < 0)
		return pthread_mutex_unlock(&stack_mutex), NULL;

	else return pthread_mutex_unlock(&stack_mutex), substrs[last - i];
	}
	else
	{
		fprintf(stderr, "pop_last_token: Error locking mutex!\n");
		abort();
	}
}

int get_token_last_no()
{
	if(pthread_mutex_lock(&stack_mutex) == 0)
	{

	if(!substrs)
		return pthread_mutex_unlock(&stack_mutex), -1;
	int last = 0; for( ; substrs[last]; ++last); return pthread_mutex_unlock(&stack_mutex), (last - 1);
	
	}
	else
	{
		fprintf(stderr, "pop_last_token: Error locking mutex!\n");
		abort();
	}
}

void pop_last_token()
{
	if(pthread_mutex_lock(&stack_mutex) == 0)
	{
	fprintf(f, "pop_last_token\n");

	int i = 0;
	while(substrs[i] != 0)
	{
		fprintf(f, "%s ", substrs[i]);
		++i;
	}
	fprintf(f, "\n");

	int last = get_token_last_no();
	if(last >= 0)
		free(substrs[last]), substrs[last] = NULL;
	pthread_mutex_unlock(&stack_mutex);
	}
	else
	{
		fprintf(stderr, "pop_last_token: Error locking mutex!\n");
		abort();
	}
}

void push_token(char *tok)
{
	if(pthread_mutex_lock(&stack_mutex) == 0)
	{
	fprintf(f, "push_token(%s)\n", tok);

	int i = 0;
	while(substrs[i] != 0)
	{
		fprintf(f, "%s ", substrs[i]);
		++i;
	}
	fprintf(f, "\n");

	int currsize = get_token_last_no() + 2;

	tok = strdup(tok);

	if(substrs)
		substrs = realloc(substrs, (currsize + 1) * sizeof(char*));
	else
		substrs = malloc((currsize + 1) * sizeof(char*));

	substrs[currsize] = 0;
	substrs[currsize - 1] = tok;
	pthread_mutex_unlock(&stack_mutex);
	}
	else
	{
		fprintf(stderr, "push_token: Error locking mutex!\n");
		abort();
	}
}

void * get_op(char *name)
{
	struct op *opsp = ops;

	while(opsp->name != NULL && strcmp(opsp->name, name) != 0)
	{
		opsp++;
	}

	return opsp->value;
}

int main(int argc, char *argv[])
{
	f = fopen("debug.txt", "w+");

	WINDOW *win;
	int ch;
	int batch = 0;
	int endflag = 0;
	int no_chain_eval = 0;
	int i;
	for(i = 0; i < argc; i++)
	{
		if(strcmp(argv[i], "--batch") == 0)
			batch = 1;

		if(strcmp(argv[i], "--no-chain-eval") == 0)
			no_chain_eval = 1;
	}

	pthread_mutexattr_t ops;

	pthread_mutexattr_init(&ops);

	pthread_mutexattr_settype(&ops, PTHREAD_MUTEX_RECURSIVE);

	if(0 != pthread_mutex_init(&stack_mutex, &ops))
	{
		fprintf(stderr, "Error initializing stack semaphore!\n");
		abort();
	}

	if(!batch)
	{
	initscr();
	raw();
	keypad(stdscr, TRUE);
	noecho();

	refresh();

	win = newwin(LINES / 3, COLS - 2, LINES - LINES / 3, 1);
	//win = newwin(10, 10, 10, 10);
	box(win, 0, 0);

	wrefresh(win);

	printw("%4d > ", curbuf_len());

	mvwprintw(win, 0, 2, "Tokens...");
	}

	while((!batch && ch != 27) || (batch && ch != 13 && ch != 10 && !endflag ) ) {

		if(!batch)
			ch = getch();
		else
		{
			int val = fgetc(stdin);
			if(val == EOF) endflag = 1;
			ch = (char)val;
		}
		
		if(ch == 27) break;
		
		if(ch == 10 || ch == 13) {
			free(lastbuf);
			lastbuf = 0;
			if(curbuf)
				lastbuf = strdup(curbuf);
			free(curbuf);
			curbuf = 0;
			if(!batch)
			{
				int x, y;
				getyx(stdscr, y, x);
				move(y+1, 0);
				printw("[%d] %4d > ", ch, curbuf_len());
			}

			continue;
		}
		else
		if(!batch && ch == KEY_BACKSPACE)
		{
			if(curbuf_len() > 0)
			{
				int x, y;

				if(!batch)
				{
					getyx(stdscr, y, x);
					move(y, x-1);
					printw(" ");
					move(y, x-1);

						{int y,x; getyx(stdscr, y, x);
						mvprintw(14, 15, "curbuf_len(): %d", curbuf_len()); mvprintw(y,x, "");}
				}

				if(curbuf_len() - 1 < wait_pos)
				{
					current_task.state = TS_CANCEL;
					pthread_join( current_task.thread, NULL );
					memset(&current_task, 0, sizeof(current_task));
					current_task.state = TS_NONE;
				}
			}
			curbuf_delete_char();
			tokenize();
			if(!batch) write_tokens(win);
		}
		else if((!batch && ch == KEY_F(2)) || (batch && endflag) || ch == ' ')
		{
			char *tok = get_token_from_end(0);

			if(tok)
			{
			if(current_task.state == TS_NONE)
			{
				void (*op)(thread_state_t *) = get_op(tok);
				if(op)
				{
					current_task.state = TS_NONE;
					current_task.parent = NULL;

					pthread_create( &current_task, NULL, op, &current_task );
					while( current_task.state == TS_NONE) nanosleep(&sleep_interval, NULL);
					if( current_task.state == TS_OK)
					{
						pthread_join( current_task.thread, NULL );
						memset(&current_task, 0, sizeof(current_task));
						//current_task.state = TS_NONE;
					}
					else if( current_task.state == TS_WAITING)
					{
						// to restore on cancel
						wait_pos = curbuf_len();

						if(!batch)
						{
							int y,x; getyx(stdscr, y, x);
							mvprintw(15, 15, "wait_pos: %d", wait_pos); mvprintw(y,x, "");
						}
					}

					rebuildstr();
				}

			}
			else /*TS_WAITING */
			{
				current_task.state = TS_RESPONSE;
				while( current_task.state == TS_RESPONSE) nanosleep(&sleep_interval, NULL);
				if( current_task.state == TS_OK)
				{
					pthread_join( current_task.thread, NULL );
					memset(&current_task, 0, sizeof(current_task));
					current_task.state = TS_NONE;
				}

				rebuildstr();
			}
			}
				
			if(!batch)
			{
				int x, y;
				getyx(stdscr, y, x);

				mvprintw(y, 0, "%50s", " ");
				mvprintw(y, 0, "[%d] %4d > %s", ch, curbuf_len(), curbuf);

				write_tokens(win);
			}

			if(ch == ' ')
			{
				curbuf_append_char(ch);
				tokenize();
				if(!batch) write_tokens(win);
			}
			else
			{
			}
		}
		else
		{
			//printw("%c", ch);
			curbuf_append_char(ch);
			tokenize();
			if(!batch) write_tokens(win);
		}

		if(!batch)
		{		
			int x, y;
			getyx(stdscr, y, x);
			move(y, 0);
			printw("[%d] %4d > %s", ch, curbuf_len(), curbuf);
		}
	}
	
	if(!batch) endwin();
	
	if(lastbuf)
		printf("lastbuf: %s\n", lastbuf);

	if(curbuf)
		printf("curbuf: %s\n", curbuf);

	if(!lastbuf && !curbuf)
		printf("lastbuf and curbuf empty\n");

	fclose(f);

	return 0;
}
