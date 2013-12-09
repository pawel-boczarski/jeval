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

OPERATOR: rep

Syntax: op op op op ... op1 rep op2
	where op1 - number of repetitions
              op2 - operator to repete

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

Enjoy :).

Note: it's not necessary to press ENTER after expression in ncurses mode, space is sufficient.
You can always restart by removing any part of expression you like with Backspace,
but it won't work if two-handed function waits for more than one argument !
With ++ and rep functions it works, anyway.

BATCH MODE

For batch use --batch switch.
It takes expressions from stdin.
Currently it will only eval the last expression!

