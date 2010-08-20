%pure-parser
%name-prefix="iodata_"
%locations
%defines
%error-verbose
%parse-param { iodata::parser *context }
%lex-param { void *scanner }

%{
#include "iodata.h"
%}

%union {
    int tid; // token id
    std::string *str; // just a pointer to a string
    iodata::bitmask_t unsign ; // huge unsigned value uint64_t
    iodata::integer_t sshort ; // short signed value int32_t
    iodata::bytes *iobytes ;
    iodata::bitmask *iobitmask ;
    iodata::integer *iointeger ;
    iodata::item *ioitem ;
    iodata::array *ioarray ;
    iodata::record *iorecord ;
}

%{
int iodata_lex(YYSTYPE* lvalp, YYLTYPE* llocp, void* scanner) ;
void iodata_error(YYLTYPE* locp, iodata::parser* context, const char* err) ;
#define scanner context->scanner
%}


%token <str> TSTRING TDOLLAR TIDENT TERROR
%token <unsign> TPOSITIVE
%token <sshort> TSIGNED
%token <tid> TUNKNOWN

%type <iobytes> bytes
%type <iobitmask> bitmask
%type <str> sbits
%type <unsign> ibits
%type <iointeger> integer
%type <ioitem> item
%type <ioarray> array
%type <iorecord> record record_

%start config

%%

config : record '.' { context->tree = $1 }
       ;

item : '{' record '}' { $$=$2 }
     | '[' array ']' { $$=$2 }
     | '['       ']' { $$=new iodata::array }
     | integer { $$=$1 }
     | bitmask { $$=$1 }
     | bytes { $$=$1 }
     ;

record : { $$ = new iodata::record }
       | record_ { $$=$1 }
       ;

record_ : TIDENT '=' item { ($$=new iodata::record)->add(*$1,$3) ; delete $1 }
        | TIDENT '=' item ',' record_ { ($$=$5)->add(*$1,$3) ; delete $1 /* XXX replace? */ }
        ;

array : item { ($$=new iodata::array)->add($1) }
      | array ',' item { ($$=$1)->add($3) }
      ;

integer : TPOSITIVE { $$=new iodata::integer(iodata::integer_t($1)) } ;

integer : TSIGNED { $$=new iodata::integer($1) } ;

ibits : '$' TPOSITIVE { $$ = $2 } ;

sbits : TDOLLAR { $$ = $1 } ;

bitmask : ibits { ($$=new iodata::bitmask)->add($1) }
        | sbits { ($$=new iodata::bitmask)->add(*$1) ; delete $1 }
        | bitmask '|' ibits { ($$=$1)->add($3) }
        | bitmask '|' sbits { ($$=$1)->add(*$3) ;  delete $3 }
        ;

bytes : TSTRING { $$=new iodata::bytes(*$1) ; delete $1 }
      | bytes '+' TSTRING { ($$=$1)->x += *$3 ; delete $3 }
      ;


%%

