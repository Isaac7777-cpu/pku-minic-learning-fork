%option noyywrap
%option nounput
%option noinput

%{

#include <cstdlib>
#include <string>

#include "sysy.tab.hpp"

using namespace std;

%}

%x C_COMMENT

WhiteSpace    [ \t\n\r]*
LineComment   "//".*

Identifier    [a-zA-Z_][a-zA-Z0-9_]*

Decimal       [1-9][0-9]*
Octal         0[0-7]*
Hexadecimal   0[xX][0-9a-fA-F]+

%%

{WhiteSpace}  { /* Ignore without actions */ }
{LineComment} { /* Ignore without actions */ }

"/*"                  { BEGIN(C_COMMENT); }
<C_COMMENT>"*/"       { BEGIN(INITIAL); }
<C_COMMENT>.|\n       { /* Ignore comment content */ }
<C_COMMENT><<EOF>>    { /* Unterminated comment */ return 0; }

"int"         { return INT; }
"return"      { return RETURN; }

{Identifier}  { yylval.str_val = new string(yytext); return IDENT; }

{Decimal}     { yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST; }
{Octal}       { yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST; }
{Hexadecimal} { yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST; }

.             { return yytext[0]; }

%%
