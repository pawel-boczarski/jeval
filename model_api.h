#ifndef MODEL_API
#define MODEL_API

#include <pthread.h>
#include <time.h>

extern struct timespec sleep_interval;

//extern int thread_state;

typedef struct _thread_state_t {
	pthread_t thread;
	int state;
	void *parent;
} thread_state_t;


struct op
{
	char *name;
	void (*value)(thread_state_t *);
};

extern struct op ops[];

// op -> main signalling
#define SIGNAL_OK(thrp) do { thrp->state = TS_OK; } while(0)

#define SIGNAL_WAIT(thrp) do { thrp->state = TS_WAITING; do { \
		nanosleep(&sleep_interval, NULL); \
		} while(thrp->state == TS_WAITING); } while(0)


// main -> op signamlling

#define WAIT_FOR_CHILD_FIRST_RESP(thrp)	\
	do { while( thrp->state == TS_NONE) nanosleep(&sleep_interval, NULL); } while(0)

#define WAIT_FOR_CHILD_NEXT_RESP(thrp)	\
	do { while( thrp->state == TS_RESPONSE) nanosleep(&sleep_interval, NULL); } while(0)


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

/*
 gets operator function by name.
 Used by those operators, that operate on other operators
*/
void * get_op(char *name);


/*
 sets a variable
*/
int set_var(char *name, char *value);

/*
 gets a variable
*/
char* get_var(char *name);

/*
 destroys a variable
*/
int unset_var(char *name);

#endif
