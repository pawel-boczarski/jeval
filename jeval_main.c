#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "model_api.h"

volatile char *curbuf = 0;
volatile char **substrs = 0;

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_t current_task;

int wait_pos = -1;

struct timespec sleep_interval = { .tv_sec = 0, .tv_nsec = 1000000 };

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

	mvwprintw(win, 0, 20, "[ thread state : '%s' ]", tst[thread_state]);	

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
	if(!substrs) return NULL;

	int last = 0; for( ; substrs[last] ; ++last); --last;

	if(last - i < 0)
		return NULL;

	else return substrs[last - i];
}

int get_token_last_no()
{
	if(!substrs)
		return -1;
	int last = 0; for( ; substrs[last]; ++last); return last - 1;
}

void pop_last_token()
{
	int last = get_token_last_no();
	if(last >= 0)
	free(substrs[last]), substrs[last] = NULL;
}

void push_token(char *tok)
{
	int currsize = get_token_last_no() + 2;

	tok = strdup(tok);

	if(substrs)
		substrs = realloc(substrs, (currsize + 1) * sizeof(char*));
	else
		substrs = malloc((currsize + 1) * sizeof(char*));

	substrs[currsize] = 0;
	substrs[currsize - 1] = tok;
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

int main()
{
	WINDOW *win;
	int ch;

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

	while(ch != 27) {
		ch = getch();
		
		if(ch == 27) break;
		
		if(ch == 10 || ch == 13) {
			free(curbuf);
			curbuf = 0;
			int x, y;
			getyx(stdscr, y, x);
			move(y+1, 0);
			printw("[%d] %4d > ", ch, curbuf_len());

			continue;
		}
		else
		if(ch == KEY_BACKSPACE)
		{
			if(curbuf_len() > 0)
			{
				int x, y;
				getyx(stdscr, y, x);
				move(y, x-1);
				printw(" ");
				move(y, x-1);

						{int y,x; getyx(stdscr, y, x);
						mvprintw(14, 15, "curbuf_len(): %d", curbuf_len()); mvprintw(y,x, "");}

				if(curbuf_len() - 1 < wait_pos)
				{
					thread_state = TS_CANCEL;
					pthread_join( current_task, NULL );
					memset(&current_task, 0, sizeof(current_task));
					thread_state = TS_NONE;
				}
			}
			curbuf_delete_char();
			tokenize();
			write_tokens(win);
		}
		else if(ch == KEY_F(2) || ch == ' ')
		{
			char *tok = get_token_from_end(0);

			if(tok)
			{
			if(thread_state == TS_NONE)
			{
				void (*op)() = get_op(tok);
				if(op)
				{
					pthread_create( &current_task, NULL, op, NULL );
					while( thread_state == TS_NONE) nanosleep(&sleep_interval, NULL);
					if( thread_state == TS_OK)
					{
						pthread_join( current_task, NULL );
						memset(&current_task, 0, sizeof(current_task));
						thread_state = TS_NONE;
					}
					else if( thread_state == TS_WAITING)
					{
						// to restore on cancel
						wait_pos = curbuf_len();

						int y,x; getyx(stdscr, y, x);
						mvprintw(15, 15, "wait_pos: %d", wait_pos); mvprintw(y,x, "");
					}

					rebuildstr();
				}

			}
			else
			{
				thread_state = TS_RESPONSE;
				pthread_join( current_task, NULL );
				memset(&current_task, 0, sizeof(current_task));
				thread_state = TS_NONE;

				rebuildstr();
			}
			}
				
			int x, y;
			getyx(stdscr, y, x);

			mvprintw(y, 0, "%50s", " ");
			mvprintw(y, 0, "[%d] %4d > %s", ch, curbuf_len(), curbuf);

			write_tokens(win);

			if(ch == ' ')
			{
				curbuf_append_char(ch);
				tokenize();
				write_tokens(win);
			}
		}
		else
		{
			//printw("%c", ch);
			curbuf_append_char(ch);
			tokenize();
			write_tokens(win);
		}
		
		int x, y;
		getyx(stdscr, y, x);
		move(y, 0);
		printw("[%d] %4d > %s", ch, curbuf_len(), curbuf);
	}
	endwin();

	
	printf("curbuf: %s\n", curbuf);

	return 0;
}
