%option reentrant
%option prefix="iodata_"
%option bison-bridge
%option bison-locations
%option noyywrap
%option nounput
%option yylineno

%{
#include <string>
#include <iostream>
using namespace std ;
#include "iodata.h"
#include "parser.hpp"

void iodata_error(YYLTYPE* locp, iodata::parser* context, const char* format, ...) ;

#define YY_EXTRA_TYPE iodata::parser*
#define YY_INPUT(buf,result,max_size) \
  { yyextra->input.read(buf,max_size) ; \
    result = yyextra->input.gcount() ; \
  }

#define YY_USER_ACTION NXT_CHAR
#define NXT_CHAR \
  yylloc_param->first_line = yylloc_param->last_line = yylineno ; \
  yylloc_param->first_column = yycolumn ; \
  yylloc_param->last_column = yycolumn+yyleng-1 ; \
  yycolumn += yyleng ;

#define CHR (yytext[0])
#define STR(a,b) string(yytext+(a), yyleng-(a)-(b))
#define STRING(a,t,b) yylval->str = new STR(a,b) ; return t ;
#define INTEGER(a, base, sign, t) \
  yylval->unsign = sign strtoll(STR(a,0).c_str(), NULL, base) ; \
  return t ;
#define ERROR(msg...) iodata_error(yylloc_param, yyextra, msg) ; return TERROR ;
%}

%x C_COMMENT CXX_COMMENT STRING_LIT

ALPHA  [a-zA-Z_]
DEC    [0-9]
HEX    [a-fA-F0-9]
OCT    [0-7]
SINGLE [,=\][\}{\.+$|:]

IDENT  {ALPHA}({ALPHA}|{DEC})*
HEXNUM 0[xX]{HEX}+
OCTNUM 0{OCT}+
DECNUM ([1-9]{DEC}*)|0

%%

[ \t]                ;
{SINGLE}             return *yytext ;
{IDENT}              STRING(0,TIDENT,0) ;
"$"{IDENT}           STRING(1,TDOLLAR,0) ;
{HEXNUM}             INTEGER(2,16, +, TPOSITIVE) ;
{OCTNUM}             INTEGER(0, 8, +, TPOSITIVE) ;
{DECNUM}             INTEGER(0,10, +, TPOSITIVE) ;
"-"{HEXNUM}          INTEGER(3,16, -, TSIGNED) ;
"-"{OCTNUM}          INTEGER(1, 8, -, TSIGNED) ;
"-"{DECNUM}          INTEGER(1,10, -, TSIGNED) ;

<INITIAL>"/*"              BEGIN(C_COMMENT) ;
<INITIAL>"#"|"//"          BEGIN(CXX_COMMENT) ;
<INITIAL>"\""              yylval->str = new string ; BEGIN(STRING_LIT) ;

<C_COMMENT>"*/"      BEGIN(INITIAL) ;
<C_COMMENT>[^*\n]+   ; /* eat comment in chunks */
<C_COMMENT>"*"       ; /* eat the lone star */

<CXX_COMMENT>\n              ++yylineno ; BEGIN(INITIAL) ;
<CXX_COMMENT>[^\n]+          ; /* eat characters */
<INITIAL,C_COMMENT>\n        ++yylineno ;

<STRING_LIT>\n               ERROR("unterminated string literal") ;
<STRING_LIT>[^\n\"\\]+       *yylval->str += STR(0,0) ;
<STRING_LIT>"\\n"            *yylval->str += (char)'\n' ;
<STRING_LIT>"\\t"            *yylval->str += (char)'\t' ;
<STRING_LIT>"\\\\"           *yylval->str += (char)'\\' ;
<STRING_LIT>"\\\""           *yylval->str += (char)'\"' ;
<STRING_LIT>"\\x{"{HEX}{HEX}"}" {
    char x[2]={ (char)strtoll(STR(3,1).c_str(), NULL, 16), 0} ;
    *yylval->str += x ;
  }
<STRING_LIT>"\""             BEGIN(INITIAL) ; return TSTRING ;


.                       ERROR("invalid character (%x02x)", CHR) ;

%%

void iodata::parser::init_scanner()
{
  yylex_init(&scanner) ;
  yyset_extra(this, scanner) ;
}

void iodata::parser::destroy_scanner()
{
  yylex_destroy(scanner) ;
}

#include "misc.h"

void iodata_error(YYLTYPE* locp, iodata::parser* context, const char *format, ...)
{
  if (context->error_line!=-1)
    return ;

  va_list args ;
  va_start(args, format) ;

  context->error_line = locp->first_line ;
  context->error_column = locp->first_column ;
  context->error_message = str_vprintf(format, args) ;
}

