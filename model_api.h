#ifndef MODEL_API
#define MODEL_API

#include <time.h>

extern struct timespec sleep_interval;

extern int thread_state;

struct op
{
	char *name;
	void (*value);
};

extern struct op ops[];


// op -> main signalling
#define SIGNAL_OK() (thread_state = TS_OK)

#define SIGNAL_WAIT() do { thread_state = TS_WAITING; do { \
		nanosleep(&sleep_interval, NULL); \
		} while(thread_state == TS_WAITING); } while(0)

#define TS_NONE 0
#define TS_WAITING 1
#define TS_OK 2
#define TS_RESPONSE 3
#define TS_CANCEL 4

/*
 gets token at particular position from expression end,
 counting from zero
*/
char* get_token_from_end(int i);

/*
 returns -1 if no tokens, last token number otherwise
*/
int get_token_last_no();

/*
 pops last token from the end
*/
void pop_last_token();

/*
 pushes new token. Must be allocated.
*/
void push_token(char *tok);

#endif
