%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AST.h"
#include "SYMBOL.h"
extern char * yytext, info;
extern int yyleng;
extern int currentLinha, currentColuna, linhaAnte, colunaAnte, flagEOF, flagSTR, column, printARV;
extern struct node* program;

int yylex (void);
void yyerror(char* s);
int counter = 0, fixBlock = 0, blockA = 0, printERRO = 0;

%}

%type <node> Program
%type <node> Declarations
%type <node> VarDeclaration
%type <node> CommaId
%type <node> VarSpec
%type <node> Type
%type <node> FuncDeclaration
%type <node> CommaIdType
%type <node> Parameters
%type <node> FuncBody
%type <node> VdStmt
%type <node> VarsAndStatements
%type <node> StmtSemicolon
%type <node> StmtBlock
%type <node> Statement
%type <node> ParseArgs
%type <node> CommaExpr
%type <node> FuncInvocation
%type <node> Expr

%token COMMA BLANKID LBRACE LPAR LSQ RBRACE
%token RPAR RSQ PACKAGE RETURN ELSE FOR IF VAR INT
%token FLOAT32 BOOL STRING PRINT FUNC CMDARGS SEMICOLON RESERVED
%token <informacao> ID INTLIT REALLIT STRLIT PARSEINT
%token <informacao> STAR DIV MINUS PLUS EQ GE GT LE LT MOD NE NOT OR AND ASSIGN

%right ASSIGN
%left OR
%left AND
%left EQ NE LT LE GT GE
%left PLUS MINUS
%left STAR DIV MOD
%left LPAR RPAR LSQ RSQ
%right NOT
%nonassoc UNARY

%union{
    char* string;
    struct node* node;
    struct informacao{
        char *string;
        int linha;
        int coluna;
    }informacao;
};

%%
    
Program: PACKAGE ID SEMICOLON Declarations                                      {$$ = program = newNode("Program", NULL, 0, 0); $$->child = $4;}
    ;


Declarations: VarDeclaration SEMICOLON Declarations                             {$$ = $1; if($3)addBrother($1, $3);}
    | FuncDeclaration SEMICOLON Declarations                                    {$$ = $1; if($3)addBrother($1, $3);}
    |                                                                           {$$ = NULL;}
    ;

VarDeclaration: VAR VarSpec                                                     {$$ = $2;}
    | VAR LPAR VarSpec SEMICOLON RPAR                                           {$$ = $3;}
    ;

CommaId: COMMA ID                                                               {$$ = newNode("VarDecl",NULL,0,0);$$->child=newNode("Id",$2.string,$2.linha,$2.coluna);}
    | CommaId COMMA ID                                                          {$$ = $1; struct node* aux = newNode("VarDecl", NULL, 0, 0);                                                                                               aux->child = newNode("Id", $3.string, $3.linha, $3.coluna); addBrother($$, aux);}
    ;

VarSpec: ID CommaId Type                                                        {$$ = newNode("VarDecl", NULL, 0, 0); $$->child = $3;
                                                                                 addBrother($3, newNode("Id", $1.string, $1.linha, $1.coluna)); addBrother($$, $2);
                                                                                    struct node *aux, *current = $2;
                                                                                    while(current){
                                                                                        aux = current->child;
                                                                                        current->child = newNode($3->id, NULL, 0, 0);
                                                                                        current->child->brother = aux;
                                                                                        current = current->brother;
                                                                                    }
                                                                                }
    | ID Type                                                                   {$$ = newNode("VarDecl", NULL, 0, 0); $$->child = $2;
                                                                                 addBrother($2, newNode("Id", $1.string, $1.linha, $1.coluna));}
    ;

Type: INT                                                                       {$$ = newNode("Int", NULL, 0, 0);}
    | FLOAT32                                                                   {$$ = newNode("Float32", NULL, 0, 0);}
    | BOOL                                                                      {$$ = newNode("Bool", NULL, 0, 0);}
    | STRING                                                                    {$$ = newNode("String", NULL, 0, 0);}
    ;

FuncDeclaration: FUNC ID LPAR Parameters RPAR Type FuncBody                     {$$ = newNode("FuncDecl", NULL, 0, 0);
                                                                                 $$->child = newNode("FuncHeader", NULL, 0, 0);
                                                                                 addBrother($$->child, $7);
                                                                                 $$->child->child = newNode("Id", $2.string, $2.linha, $2.coluna);
                                                                                 addBrother($$->child->child, $6);
                                                                                 addBrother($6, $4);}
    | FUNC ID LPAR Parameters RPAR FuncBody                                     {$$ = newNode("FuncDecl", NULL, 0, 0);
                                                                                 $$->child = newNode("FuncHeader", NULL, 0, 0);
                                                                                 addBrother($$->child, $6);
                                                                                 $$->child->child = newNode("Id", $2.string, $2.linha, $2.coluna);
                                                                                 addBrother($$->child->child, $4);}
    ;

CommaIdType: COMMA ID Type                                                      {$$ = newNode("ParamDecl", NULL, 0, 0); $$->child = $3;
                                                                                 addBrother($3, newNode("Id", $2.string, $2.linha, $2.coluna));}
    | CommaIdType COMMA ID Type                                                 {struct node* aux2 = newNode("ParamDecl",NULL, 0, 0);$$ = $1; addBrother($1, aux2);                                                                              aux2->child = $4; addBrother($4, newNode("Id", $3.string, $3.linha, $3.coluna));}
    ;

Parameters: ID Type CommaIdType                                                 {struct node* aux = newNode("Id", $1.string, $1.linha, $1.coluna);
                                                                                 $$=newNode("FuncParams",NULL, 0, 0);
                                                                                 $$->child = newNode("ParamDecl", NULL, 0, 0); $$->child->child = $2;
                                                                                 addBrother($2, aux); addBrother($$->child, $3);}
    | ID Type                                                                   {$$ = newNode("FuncParams", NULL, 0, 0);
                                                                                 $$->child = newNode("ParamDecl", NULL, 0, 0); $$->child->child = $2;
                                                                                 addBrother($2, newNode("Id", $1.string, $1.linha, $1.coluna));}
    |                                                                           {$$ = newNode("FuncParams", NULL, 0, 0);}
    ;

FuncBody: LBRACE VarsAndStatements RBRACE                                       {$$ = newNode("FuncBody", NULL, 0, 0); $$->child = $2;}
    ;

VdStmt: VarDeclaration                                                          {/*empty*/}
    | Statement                                                                 {$$ = $1;}
    |                                                                           {$$=NULL;}
    ;

VarsAndStatements: VarsAndStatements VdStmt SEMICOLON                           {if($1){
                                                                                    $$ = $1;
                                                                                    addBrother($1, $2);
                                                                                 }
                                                                                 else
                                                                                    $$ = $2;}
    |                                                                           {$$=NULL;}
    ;

StmtSemicolon: StmtSemicolon Statement SEMICOLON                                {if($1){
                                                                                    $$ = $1; addBrother($1, $2);
                                                                                 }
                                                                                 else
                                                                                    $$ = $2;}
    |                                                                           {$$=NULL;}
    ;
StmtBlock: StmtBlock Statement SEMICOLON                                        {if($1 && blockA == 1){
                                                                                 addBrother($$->child, $2);
                                                                                 }
                                                                                else if($1){
                                                                                    if(fixBlock == 0){$$ = newNode("Block", NULL, 0, 0);
                                                                                        $$->child = $1;blockA = 1;}fixBlock = 1; addBrother($1, $2);
                                                                                    
                                                                                }
                                                                                 else{
                                                                                     fixBlock = 0;
                                                                                     $$ = $2;blockA = 0;}}
    |                                                                           {$$=NULL;}
    ;

Statement: LBRACE StmtBlock RBRACE                                              {$$ = $2;}
    | ID ASSIGN Expr                                                            {$$ = newNode("Assign", NULL, $2.linha, $2.coluna);
                                                                                 $$->child = newNode("Id", $1.string, $1.linha, $1.coluna);
                                                                                 addBrother($$->child, $3);}
    | IF Expr LBRACE StmtSemicolon RBRACE ELSE LBRACE StmtSemicolon RBRACE      {if($4 && $8 ){
                                                                                    struct node* aux = newNode("Block", NULL, 0, 0);
                                                                                    $$ = newNode("If", NULL, 0, 0);
                                                                                    $$->child = $2;
                                                                                    addBrother($$->child, newNode("Block", NULL, 0, 0));
                                                                                    $2->brother->child = $4;
                                                                                    addBrother($$->child, aux);
                                                                                    aux->child = $8;
                                                                                 }
                                                                                 else if($4){
                                                                                    $$ = newNode("If", NULL, 0, 0);
                                                                                    $$->child = $2;
                                                                                    addBrother($$->child,newNode("Block", NULL, 0, 0));
                                                                                    $$->child->brother->child = $4;
                                                                                    addBrother($$->child->brother, newNode("Block", NULL, 0, 0));
                                                                                 }
                                                                                 else if($8){
                                                                                    struct node* aux = newNode("Block", NULL, 0, 0);
                                                                                    $$ = newNode("If", NULL, 0, 0);
                                                                                    $$->child = $2;
                                                                                    addBrother($$->child, aux);
                                                                                    addBrother(aux, newNode("Block", NULL, 0, 0));
                                                                                    aux->brother->child = $8;}
                                                                                 else{
                                                                                    $$ = newNode("If", NULL, 0, 0);
                                                                                    $$->child = $2;
                                                                                    addBrother($$->child, newNode("Block", NULL, 0, 0));
                                                                                    addBrother($$->child, newNode("Block", NULL, 0, 0));
                                                                                 }}

    
    | IF Expr LBRACE StmtSemicolon RBRACE                                       {if($4){
                                                                                    $$ = newNode("If", NULL, 0, 0);
                                                                                    $$->child = $2;
                                                                                    addBrother($$->child, newNode("Block", NULL, 0, 0));
                                                                                    $$->child->brother->child = $4;
                                                                                    addBrother($$->child->brother, newNode("Block", NULL, 0, 0));
                                                                                 }
                                                                                 else{
                                                                                    $$ = newNode("If", NULL, 0, 0);
                                                                                    $$->child = $2;
                                                                                    addBrother($$->child, newNode("Block", NULL, 0, 0));
                                                                                    addBrother($$->child, newNode("Block", NULL, 0, 0));
                                                                                 }}
    
    
    | FOR Expr LBRACE StmtSemicolon RBRACE                                      {if($4){
                                                                                    $$ = newNode("For", NULL, 0, 0);
                                                                                    $$->child = $2;
                                                                                    $$->child->brother = newNode("Block", NULL, 0, 0);
                                                                                    $2->brother->child = $4;
                                                                                 }
                                                                                 else{
                                                                                    $$ = newNode("For", NULL, 0, 0);
                                                                                    $$->child = $2;
                                                                                    $$->child->brother = newNode("Block", NULL, 0, 0);
                                                                                 }}
    
    | FOR LBRACE StmtSemicolon RBRACE                                           {if($3){
                                                                                    $$ = newNode("For", NULL, 0, 0);
                                                                                    $$->child = newNode("Block", NULL, 0, 0);
                                                                                    $$->child->child = $3;
                                                                                 }
                                                                                 else{
                                                                                    $$ = newNode("For", NULL, 0, 0);
                                                                                    $$->child = newNode("Block", NULL, 0, 0);
                                                                                 }}
    
    | RETURN Expr                                                               {$$ = newNode("Return", NULL, 0, 0); $$->child = $2;}
    | RETURN                                                                    {$$ = newNode("Return", NULL, 0, 0);}
    | FuncInvocation                                                            {/*empty*/}
    | ParseArgs                                                                 {/*empty*/}
    | PRINT LPAR Expr RPAR                                                      {$$ = newNode("Print", NULL, 0, 0), $$->child = $3;}
    | PRINT LPAR STRLIT RPAR                                                    {$$ = newNode("Print", NULL, 0, 0), $$->child = newNode("StrLit",$3.string, 0, 0);}
    | error                                                                     {printERRO = 1; $$ = newNode(NULL, NULL, 0, 0);}
    ;

ParseArgs: ID COMMA BLANKID ASSIGN PARSEINT LPAR CMDARGS LSQ Expr RSQ RPAR  {$$ = newNode("ParseArgs", NULL, $5.linha, $5.coluna);
                                                                             $$->child = newNode("Id", $1.string, $1.linha, $1.coluna); addBrother($$->child, $9);}
    | ID COMMA BLANKID ASSIGN PARSEINT LPAR error RPAR                      {printERRO = 1; $$ = newNode(NULL, NULL, 0, 0);}
    ;

CommaExpr: COMMA Expr                       {$$ = $2;}
    | CommaExpr COMMA Expr                  {$$ = $1; addBrother($$, $3);}
    ;

FuncInvocation: ID LPAR Expr CommaExpr RPAR {$$ = newNode("Call", NULL, 0, 0);
                                             $$->child = newNode("Id", $1.string, $1.linha, $1.coluna); addBrother($$->child,$3); addBrother($3, $4);}
    | ID LPAR Expr RPAR                     {$$ = newNode("Call", NULL, 0, 0); $$->child = newNode("Id", $1.string, $1.linha, $1.coluna); addBrother($$->child, $3);}
    | ID LPAR RPAR                          {$$ = newNode("Call", NULL, 0, 0); $$->child = newNode("Id", $1.string, $1.linha, $1.coluna);}
    | ID LPAR error RPAR                    {printERRO = 1; $$ = newNode(NULL, NULL, 0, 0);}
    ;

Expr: Expr OR Expr                          {$$ = newNode("Or", NULL, $2.linha, $2.coluna); $$->child = $1; addBrother($1, $3);}
    | Expr AND Expr                         {$$ = newNode("And", NULL, $2.linha, $2.coluna); $$->child = $1; addBrother($1, $3);}
    | Expr LT Expr                          {$$ = newNode("Lt", NULL, $2.linha, $2.coluna); $$->child = $1; addBrother($1, $3);}
    | Expr GT Expr                          {$$ = newNode("Gt", NULL, $2.linha, $2.coluna); $$->child = $1; addBrother($1, $3);}
    | Expr EQ Expr                          {$$ = newNode("Eq", NULL, $2.linha, $2.coluna); $$->child = $1; addBrother($1, $3);}
    | Expr NE Expr                          {$$ = newNode("Ne", NULL, $2.linha, $2.coluna); $$->child = $1; addBrother($1, $3);}
    | Expr LE Expr                          {$$ = newNode("Le", NULL, $2.linha, $2.coluna); $$->child = $1; addBrother($1, $3);}
    | Expr GE Expr                          {$$ = newNode("Ge", NULL, $2.linha, $2.coluna); $$->child = $1; addBrother($1, $3);}
    | Expr PLUS Expr                        {$$ = newNode("Add", NULL, $2.linha, $2.coluna); $$->child = $1; addBrother($1, $3);}
    | Expr MINUS Expr                       {$$ = newNode("Sub", NULL, $2.linha, $2.coluna); $$->child = $1; addBrother($1, $3);}
    | Expr STAR Expr                        {$$ = newNode("Mul", NULL, $2.linha, $2.coluna); $$->child = $1; addBrother($1, $3);}
    | Expr DIV Expr                         {$$ = newNode("Div", NULL, $2.linha, $2.coluna); $$->child = $1; addBrother($1, $3);}
    | Expr MOD Expr                         {$$ = newNode("Mod", NULL, $2.linha, $2.coluna); $$->child = $1; addBrother($1, $3);}
    | NOT Expr                              {$$ = newNode("Not", NULL, $1.linha, $1.coluna); $$->child = $2;}
    | MINUS Expr     %prec NOT              {$$ = newNode("Minus", NULL, $1.linha, $1.coluna); $$->child = $2;}
    | PLUS Expr      %prec NOT              {$$ = newNode("Plus", NULL, $1.linha, $1.coluna); $$->child = $2;}
    | INTLIT                                {$$ = newNode("IntLit", $1.string, $1.linha, $1.coluna);}
    | REALLIT                               {$$ = newNode("RealLit", $1.string, $1.linha, $1.coluna);}
    | ID                                    {$$ = newNode("Id", $1.string, $1.linha, $1.coluna);}
    | FuncInvocation                        {/*empty*/}
    | LPAR Expr RPAR                        {$$ = $2;}
    | LPAR error RPAR                       {printERRO = 1; $$ = newNode(NULL, NULL, 0, 0);}
    ;
    

%%

void yyerror(char* s) {
    printERRO = 1;
    if(yytext[0]=='\n'){
        printf("Line %d, column %d: %s: %s\n", linhaAnte, colunaAnte, s, yytext);
    }
    else if(flagSTR==1 && (strcmp(yytext, "\\n")==0)){
        printf("Line %d, column %d: %s: %s\n", currentLinha+flagEOF, column, s, yytext);
        flagSTR = 0;
    }
    else if(flagSTR==1){
        printf("Line %d, column %d: %s: \"%s\n", currentLinha+flagEOF, column, s, yytext);
        flagSTR = 0;
    }
    else{
        if(currentColuna-yyleng == 0) printf("Line %d, column 1: %s: %s\n", currentLinha+flagEOF, s, yytext);
        else printf("Line %d, column %d: %s: %s\n", currentLinha+flagEOF, currentColuna-yyleng, s, yytext);
    }
    
}
