
void plus()
{
	int a = atoi(get_token_from_end(1));
	int b = atoi(get_token_from_end(2));

	char *tok = malloc(50);

	sprintf(tok, "%d", a + b);
	realloc(tok, strlen(tok) + 1);

	pop_last_token();
	pop_last_token();
	pop_last_token();

	push_token(tok);		
}

void minus()
{
	int a = atoi(get_token_from_end(1));
	int b = atoi(get_token_from_end(2));

	char *tok = malloc(50);

	sprintf(tok, "%d", a - b);
	realloc(tok, strlen(tok) + 1);

	pop_last_token();
	pop_last_token();
	pop_last_token();

	push_token(tok);		
}

void mul()
{
	int a = atoi(get_token_from_end(1));
	int b = atoi(get_token_from_end(2));

	char *tok = malloc(50);

	sprintf(tok, "%d", a * b);
	realloc(tok, strlen(tok) + 1);

	pop_last_token();
	pop_last_token();
	pop_last_token();

	push_token(tok);		
}

void div()
{
	int a = atoi(get_token_from_end(1));
	int b = atoi(get_token_from_end(2));

	if(b == 0) return;

	char *tok = malloc(50);

	sprintf(tok, "%d", a / b);
	realloc(tok, strlen(tok) + 1);

	pop_last_token();
	pop_last_token();
	pop_last_token();

	push_token(tok);		
}
