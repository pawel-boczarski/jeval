Jeval - immediate expression evaluator

Jeval works in ncurses and batch mode.

To call ncurses mode type:
./jeval

Regardless of mode, jeval uses reverse Polish notation, with immediate evaluation result.
It means expression is scanned char-by-char, and is immediately evaluated if the last token
(spaces separate tokens) can be evaluated, possibly adding some new tokens to the end, and/or
removing others.

SIMPLE (LEFT HAND) OPERATION

When you type in ncurses mode: 

> 2 3 +

and then space, the expression is immediately evaluated, and only 5 remains on stack.
(Space is necessary, because it tells Jeval that the operator/token name was fully input).

Say, you wish to count (5 + 3) * (5 + 2),

when typing 5 3 + 5 2 + *, actually the input will change as follows:

'5'
'5 '
'5 3'
'5 3 '
'5 3 +'
'8 '
'8 '
'8 5'
'8 5 '
'8 5 2'
'8 5 2 '
'8 5 2 +'
'8 7 '
'8 7 *'
// space or enter
'56' or '56 '

BIDIRECTIONAL OPERATORS

It's true, that if current token is an operator, it's evaluated immediately after we press space that terminates it.

BUT, what's interesting, it's still possible to write operators, that use both left- and right- handed arguments,
due to waiting mechanism. This is possible, because operator is called in a separate thread and falls back to the loop,
if it needs additional arguments not yet present (namingly right-hand ones).

(This is actually necessary when an operator is argument to other operator - remember immediate evaluation rule!)


For instance, we have an operator '++', that works just like '+' in normal math.

When typing:
> 2 ++ 3

in jeval ncurses mode, we will see that "thread state" value in the window changes to "WAITING" after the second space.
After writing remaining '3' and space, expression is evaluated!

USEFUL OPERATORS

OPERATOR: +
SYNTAX: op op +
replaces two last tokens and itself with its sum

OPERATORS: - , * , /

Work similarily to +, but perform subtraction, multiplication, division.

OPERATOR: addall
SYNTAX : left_sq_bracket op op op ... op right_sq_bracket addall,
         where left_sq_bracket is [
	and right_sq_bracket is ]
counts sum of tokens between nearest [ and ] tokens. The ] token need to immediately preceed addall !!

Sample
[ 10 20 30 40 50 ] addall
=> yields 150

OPERATOR: ++
SYNTAX : op ++ op

Like + in normal math

Sample
2 ++ 3
=> yields 5

OPERATOR: nelem
SYNTAX: nelem

Returns on stack number of tokens already on stack

Sample:
1 2 3 4 nelem
==> yields 1 2 3 4 4

OPERATOR: dup
SYNTAX: op1 dup
        op1 - token to duplicate

The dup operator duplicates last token on stack.
This operation may be necessary to perform some operation on token while still preserving
this token, as most operations destroy tokens they operate on.

Example:
0 1 2 3 4 5 dup
=> yields 0 1 2 3 4 5 5

10 dup + 
=> yields 20

3 dup dup
=> yields 3 3 3

1 2 dup 5 +
=> yields 1 2 7

Compare the last to example without dup:
1 2 5 +
=> yields 1 7

In the first case token '2' was copied before operation '+' that destroys operands!

OPERATOR: rep

Syntax: op op op op ... opn rep op2
	where opn - number of repetitions
              op2 - operator to repete

OR

	op op op op ... opn rep [ op1 op2 ... opn ]
	where opn - number of repetitions
	op1 op2 ... opn - tokens (some of them operators) to repeat

Sample:
1 2 3 4 5 4 rep *
=> yields 120

10 rep nelem
=> yields 0 1 2 3 4 5 6 7 8 9

10 20 30 2 rep +
=> yields 60

10 rep nelem 9 rep +
=> yields 45

and this one is tricky:

prog: 10 rep nelem 9 rep *
=> yields dummy_token: 3628800

Why it works? We just added a dummy token 'prog:' as a placeholder, so that first nelem returns 1, not zero.
We could just put anything else here, but that would be way too clumsy.

Or even better:

11 rep nelem 9 rep * +

=> it produces zero on beginning, but + "eats" it.

There are even more possibility to use rep operator in bracket form:

10 rep [ 2 nelem * ] 

=> yields 2 4 6 8 10 12 14 16 18 20

10 rep [ nelem rep * ]
=> yields 0 1 4 9 16 25 36 49 64 81

10 rep [ nelem rep * ] 9 rep +
=> yields sum of squares of numbers from 0 to 9


And this one is not so easy, it displays 10 first triangle numbers.
N-th triangle number is a sum t(n) = 1 + 2 + ... + n.

Voila: 
10 rep [ 0 nelem rep [ dup 1 + ] dup rep + ]
=> yields 1 3 6 10 15 21 28 36 45 55

I am not saying it's easy or even readable, but can do this even though we still don't have
variables defined!!! (I wrote the correct expression after several fails :)).

How it works?
On the beginning of first repetitin of the outer loop, there is nothing on stack.
So, after zero is put, and nelem then we have:
0 nelem => 0 1
then we have rep, so 1 will be treated as count of repetitions of [ dup 1 + ]:
0 1 rep [ dup 1 + ] => 0 dup 1 + => 0 0 1 + => 0 1
Then, the last number (this is number of iteration!) is duplicated:
0 1 dup => 0 1 1
And last element treated as number of repetitions of +:
0 1 1 dup rep + => 0 1 + => 1

Good. And what if it's not the first iteration? Let's say it's fourth one.
We already have 1 3 6 (three numbers) on stack
After 0 and is put we have:
1 3 6 0
Then is nelem:
1 3 6 0 nelem => 1 3 6 0 4
Then we have rep, so 4 will be treated as count of repetitions of [ dup 1 + ]:
1 3 6 0 dup 1 + dup 1 + dup 1 + dup 1 + => 1 3 6 0 0 1 + dup 1 + dup 1 + dup 1 + =>
=> 1 3 6 0 1 dup 1 + dup 1 + dup 1 + => 1 3 6 0 1 1 1 + dup 1 + dup 1 + =>
=> 1 3 6 0 1 2 dup 1 + dup 1 + => 1 3 6 0 1 2 2 1 + dup 1 + => 1 3 6 0 1 2 3 dup 1 + =>
=> 1 3 6 0 1 2 3 3 1 + => 1 3 6 0 1 2 3 4
Then, after inner loop is dup:
1 3 6 0 1 2 3 4 dup => 1 3 6 0 1 2 3 4 4
and then is 'rep' and '+' - so the freshly duplicated number is a number of repetitions of +,
effectively counting 0 + 1 + 2 + 3 + 4:
1 3 6 0 1 2 3 4 4 rep + => 1 3 6 0 1 2 3 4 + + + + => 1 3 6 0 1 2 7 + + + => 1 3 6 0 1 9 + + =>
1 3 6 0 10 + => 1 3 6 10

Well, we came here from having three consecutive triangle numbers to having four consecutive triangle numbers!

Could you do this even easier, without unnecessary "counting from beginning", using the last number on stack?

Or, maybe, why not count n-th triangle number using a mathematical expression for n-th triangle number?
(the mathematical expression to count n-th triangle number without adding one by one is:
 t(n) = n(n+1)/2 ) ?

OPERATOR: if
SYNTAX: op_cond if op_true op_false
	op_cond - true or false value - "0" and "0.0" evaluate to false,
                  others - to true
        op_true - operand to be pushed on stack when true
        op_false - operand to be pushed on stack when false

Sample:

1 if 10 15

=> yields 10

0 if 10 15

=> yields 15

10 15 0 if * + 

=> yields 10 15 +

10 15 1.0 if * +

=> yields 10 15 *

WARNING: The operator will not be evaluated by default !!!
This is because sometimes when operation fails operator may remain on the stack,
and we would enter an infinite loop. So operators left on stack by operators
are NOT evaluated at once. They can be evaluated in two ways:

- adding second space after expression to force evaluation,
- using chain-eval operator

OPERATOR: , 	[aka chain-eval operator]
SYNTAX: op ,
	op - another operator that was not evaluated because it was left on stack
	     as a result of operator function output

Chain-eval operator simply evaluates the last operator on stack. If there is no operator function
for it, it exits. If not, it pops itself off stack and calls the preceding operator.

Example:

10 15 0 if * + ,

=> yields 25

10 15 1 if * + ,

=> yields 150




Note: using double space after an expression works exactly same as calling chain-eval operator.

OPERATOR: nil
SYNTAX: nil

Evals to nothing. This operator will simply dissapear without affecting the rest of expression.
You can use it as a placeholder in 'if' expressions, if given case (true or false) does not require
any change on the stack.

OPERATOR: distdup
SYNTAX: op0 op1 ... opn dist distdup
	dist - distance of token to duplicate

It works similar to dup, but instead duplicates a token at given distance from the operator.
If dist is 0, opn will be duplicated, and it works exactly as dup. If dist is 1, the token
right before opn will be duplicated and so on. Keep in mind that dist is distance from the last
token preceding distance count!

Examples:

0 1 2 3 4 5 2 distdup
=> yields 0 1 2 3 4 5 3

0 1 2 3 4 5 0 distdup
=> yields 0 1 2 3 4 5 5

I don't have more examples yet, but can you utilize distdup in some way ?
Feel free to experiment :). 

Enjoy :).

Note: it's not necessary to press ENTER after expression in ncurses mode, space is sufficient.
You can always back out by removing any part of expression you like with Backspace.
Anyway, evaluated operators are evaluated already.
To restart, simply remove all expression.

WARNING
Backing out with Backspace won't work if two-handed function waits for more than one argument, and at least
one of the right hand argument was already confirmed (followed by space).
(This is, it will work, but in case you wrote some right-hand arguments and confirmed with space,
the operator has received them already and can't back out).
So, to restart such operator evaluation, remove it up and including the operator.

With ++ and rep functions it works, anyway, they only take one right-hand argument, so evaluate right after
the first right-hand argument consumption.

BATCH MODE

For batch use --batch switch.
It takes expressions from stdin.
Currently it will only eval the last expression!

From the shell perform:
$ cat > ./jeval --batch
Now type your expression and press Ctrl-D twice.

