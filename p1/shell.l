%{
#include <stdio.h>
#include <strings.h>

int _numargs = 200;
char *_args[200];
int _argcount = 0;
%}

WORD [a-zA-Z0-9\/\.-]+
SPECIAL [()><|&;]

%%
  _argcount = 0; _args[0] = NULL;

{WORD}|{SPECIAL} {
  if(_argcount < _numargs-1) {
    _args[_argcount++] = (char *)strdup(yytext);
    _args[_argcount] = NULL;
  }
}

\n return (int)_args;

[ \t]+

.

%%

char **get_line() { return (char **)yylex(); }
