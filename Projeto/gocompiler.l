        /*Mário Lemos 2019216792*/
        /*António Correia 2019216646*/



%{
#include "y.tab.h"
#include "AST.h"
#include "SYMBOL.h"
#include "LLVM.h"
int yyparse(void);
void informacao(char *string, int linha, int coluna);
%}


letra                                                        ([a-zA-Z]|"_")
alfanumerico                                                 ([a-zA-Z]|"_"|[0-9])*
numero                                                       [0-9]+
hexadecimal                                                  ([0-9]|[a-fA-F])
escape                                                       [\\"|\\f|\\n|\\r|\\t|\\\\]
reserved                                                     ("break"|"case"|"chan"|"const"|"continue"|"default"|"defer"|"fallthrough"|"go"|"goto"|"import"|"interface"|"map"|"range"|"select"|"struct"|"switch"|"type"|"++"|"--")                                                        
%{
    int currentLinha = 1, currentColuna = 1, current = 1, line = 1, column = 1, flag = 0, semicolon = 0, spaghetti = 0, count = 0, linhaAnte = 0, colunaAnte = 0, flagEOF = 0, flagSTR = 0, printARV = 0;
    int countSpaces = 0, erroStr = 0;
    extern int printERRO;
    struct node* program;
    int erroSemantico = 0;
    int erroLexical = 0;
%}
%X MULTILINECOMMENT COMMENT STR ERRO
%%
\r                                                           {;}
"."                                                          {currentColuna+=yyleng+countSpaces; countSpaces = 0; erroLexical = 1; printf("Line %d, column %d: illegal character (%s)\n", currentLinha, currentColuna-(int)yyleng, yytext);semicolon = 0;}
    /*REALLIT*/
{numero}?"."{numero}?(("e"|"E")("-"|"+")?)?{numero}         {if(flag==1) printf("REALLIT(%s)\n", yytext);semicolon = 1;currentColuna+=yyleng+countSpaces; countSpaces=0;
                                                             if(flag==2){informacao(yylval.string = strdup(yytext), currentLinha, currentColuna+countSpaces-yyleng); return REALLIT;}}
{numero}("e"|"E")("-"|"+")?{numero}                         {if(flag==1) printf("REALLIT(%s)\n", yytext);semicolon = 1;currentColuna+=yyleng+countSpaces;  countSpaces=0;
                                                             if(flag==2){informacao(yylval.string = strdup(yytext), currentLinha, currentColuna+countSpaces-yyleng); return REALLIT;}}
{numero}"."                                                 {if(flag==1) printf("REALLIT(%s)\n", yytext);semicolon = 1;currentColuna+=yyleng+countSpaces;  countSpaces=0;
                                                             if(flag==2){informacao(yylval.string = strdup(yytext), currentLinha, currentColuna+countSpaces-yyleng); return REALLIT;}}
    /*INTLIT*/
"0"("x"|"X"){hexadecimal}+                                  {currentColuna+=yyleng+countSpaces;  countSpaces=0; if(flag==1)printf("INTLIT(%s)\n", yytext);semicolon = 1;
                                                             if(flag==2){informacao(yylval.string = strdup(yytext), currentLinha, currentColuna+countSpaces-yyleng); return INTLIT;}}
"0"{numero}                                                 {  int erro = 0;
                                                                currentColuna+=yyleng+countSpaces;  countSpaces=0;
                                                                if(yytext[1]!='x' && yytext[1]!='X'){
                                                                    for(int i = 0; i<yyleng; i++){
                                                                        if(yytext[i]>='8'){
                                                                            i=yyleng;
                                                                            printf("Line %d, column %d: invalid octal constant (%s)\n", currentLinha, currentColuna-(int)yyleng, yytext);
                                                                            erro = 1;
                                                                            erroLexical = 1;
                                                                            semicolon = 0;
                                                                        }
                                                                    }
                                                                }
                                                                if(erro==0 && flag==1)printf("INTLIT(%s)\n", yytext);semicolon = 1; if(flag==2){informacao(yylval.string = strdup(yytext), currentLinha, currentColuna+countSpaces-yyleng); return INTLIT;}
                                                            }

{numero}                                                     {if(flag==1) printf("INTLIT(%s)\n", yytext);semicolon = 1;currentColuna+=yyleng+countSpaces;  countSpaces=0;
                                                              if(flag==2){informacao(yylval.string = strdup(yytext), currentLinha, currentColuna+countSpaces-yyleng); return INTLIT;}}

    /*Caracteres*/
"/*"                                                         {BEGIN MULTILINECOMMENT;currentColuna+=yyleng+countSpaces;  countSpaces=0; line = currentLinha, column = currentColuna-(int)yyleng;}
<MULTILINECOMMENT><<EOF>>                                    {BEGIN 0;erroLexical = 1;printf("Line %d, column %d: unterminated comment\n", line, column);semicolon = 0;}
<MULTILINECOMMENT>"*/"                                       {BEGIN 0;currentColuna+=yyleng+countSpaces;  countSpaces=0;}
<MULTILINECOMMENT>.                                          {currentColuna+=yyleng+countSpaces;  countSpaces=0;}
<MULTILINECOMMENT>\n                                         {if(semicolon==1 && flag==1)printf("SEMICOLON\n");currentLinha++;currentColuna=1; if(semicolon==1){semicolon = 0; if(flag==2){return SEMICOLON;}semicolon = 0;}}

"//"                                                         {BEGIN COMMENT;}
<COMMENT>.                                                   {;}
<COMMENT>\n                                                  {BEGIN 0;if(semicolon==1 && flag==1)printf("SEMICOLON\n");currentLinha++;currentColuna=1; if(semicolon==1){semicolon = 0; if(flag==2){return SEMICOLON;}semicolon = 0;}}

"\""                                                         {BEGIN STR;count = 0; currentColuna+=yyleng+countSpaces;  countSpaces=0; line = currentLinha, column = currentColuna-(int)yyleng;}
<STR><<EOF>>                                                 {BEGIN 0; erroStr = 1; erroLexical = 1; printf("Line %d, column %d: unterminated string literal\n", line, column);semicolon = 0;spaghetti = 0;}
<STR>\n                                                      {BEGIN 0; erroStr = 1; erroLexical = 1; printf("Line %d, column %d: unterminated string literal\n", line, column);semicolon = 0;spaghetti = 0;currentLinha++;currentColuna=1;}
<STR>\\[^nfrt\"\\]                                           {erroStr=1; currentColuna += count+countSpaces; countSpaces=0; erroLexical = 1; printf("Line %d, column %d: invalid escape sequence (%s)\n", line, currentColuna, &yytext[yyleng-2]); semicolon = 0;spaghetti = 1;currentColuna+=2;count = 0;}
<STR>\"                                                      {BEGIN 0; if(spaghetti==0 && flag == 1) {printf("STRLIT(\"%s)\n", yytext);semicolon = 1;currentColuna += count+countSpaces; countSpaces=0;}spaghetti = 0;currentColuna+=1+countSpaces; countSpaces = 0; if(flag==2 && erroStr == 0){flagSTR = 1; yylval.string = strdup(yytext); erroStr = 0; return STRLIT;}else{erroStr = 0; yylval.string = NULL;}}
<STR>\\[nfrt\"\\]                                            {yymore();count+=2;}
<STR>.                                                       {yymore();count++;}
";"                                                          {if(flag==1) printf("SEMICOLON\n");semicolon = 0;currentColuna+=yyleng+countSpaces;  countSpaces=0; if(flag==2){return SEMICOLON;}}
","                                                          {if(flag==1) printf("COMMA\n");semicolon = 0;currentColuna+=yyleng+countSpaces;  countSpaces=0; if(flag==2){return COMMA;}}
"_"                                                          {if(flag==1) printf("BLANKID\n");semicolon = 0;currentColuna+=yyleng+countSpaces;  countSpaces=0; if(flag==2){return BLANKID;}}
"="                                                          {if(flag==1) printf("ASSIGN\n");if(flag==2){informacao(yylval.string = strdup(yytext), currentLinha, currentColuna+countSpaces);} semicolon = 0;currentColuna+=yyleng+countSpaces;  countSpaces=0; if(flag==2){return ASSIGN;}}
"*"                                                          {if(flag==1) printf("STAR\n");if(flag==2){informacao(yylval.string = strdup(yytext), currentLinha, currentColuna+countSpaces);} semicolon = 0;currentColuna+=yyleng+countSpaces;  countSpaces=0; if(flag==2){return STAR;}}
"/"                                                          {if(flag==1) printf("DIV\n");if(flag==2){informacao(yylval.string = strdup(yytext), currentLinha, currentColuna+countSpaces);} semicolon = 0;currentColuna+=yyleng+countSpaces;  countSpaces=0; if(flag==2){return DIV;}}
"-"                                                          {if(flag==1) printf("MINUS\n");if(flag==2){informacao(yylval.string = strdup(yytext), currentLinha, currentColuna+countSpaces);} semicolon = 0;currentColuna+=yyleng+countSpaces;  countSpaces=0; if(flag==2){return MINUS;}}
"+"                                                          {if(flag==1) printf("PLUS\n");if(flag==2){informacao(yylval.string = strdup(yytext), currentLinha, currentColuna+countSpaces);} semicolon = 0;currentColuna+=yyleng+countSpaces;  countSpaces=0; if(flag==2){return PLUS;}}
"=="                                                         {if(flag==1) printf("EQ\n");if(flag==2){informacao(yylval.string = strdup(yytext), currentLinha, currentColuna+countSpaces);} semicolon = 0;currentColuna+=yyleng+countSpaces;  countSpaces=0; if(flag==2){return EQ;}}
">="                                                         {if(flag==1) printf("GE\n");if(flag==2){informacao(yylval.string = strdup(yytext), currentLinha, currentColuna+countSpaces);} semicolon = 0;currentColuna+=yyleng+countSpaces;  countSpaces=0; if(flag==2){return GE;}}
">"                                                          {if(flag==1) printf("GT\n");if(flag==2){informacao(yylval.string = strdup(yytext), currentLinha, currentColuna+countSpaces);} semicolon = 0;currentColuna+=yyleng+countSpaces;  countSpaces=0; if(flag==2){return GT;}}
"{"                                                          {if(flag==1) printf("LBRACE\n");semicolon = 0;currentColuna+=yyleng+countSpaces;  countSpaces=0; if(flag==2){return LBRACE;}}
"<="                                                         {if(flag==1) printf("LE\n");if(flag==2){informacao(yylval.string = strdup(yytext), currentLinha, currentColuna+countSpaces);} semicolon = 0;currentColuna+=yyleng+countSpaces;  countSpaces=0; if(flag==2){return LE;}}
"("                                                          {if(flag==1) printf("LPAR\n");semicolon = 0;currentColuna+=yyleng+countSpaces;  countSpaces=0; if(flag==2){return LPAR;}}
"["                                                          {if(flag==1) printf("LSQ\n");semicolon = 0;currentColuna+=yyleng+countSpaces;  countSpaces=0; if(flag==2){return LSQ;}}
"<"                                                          {if(flag==1) printf("LT\n");if(flag==2){informacao(yylval.string = strdup(yytext), currentLinha, currentColuna+countSpaces);} semicolon = 0;currentColuna+=yyleng+countSpaces;  countSpaces=0; if(flag==2){return LT;}}
"%"                                                          {if(flag==1) printf("MOD\n");if(flag==2){informacao(yylval.string = strdup(yytext), currentLinha, currentColuna+countSpaces);} semicolon = 0;currentColuna+=yyleng+countSpaces;  countSpaces=0; if(flag==2){return MOD;}}
"!="                                                         {if(flag==1) printf("NE\n");if(flag==2){informacao(yylval.string = strdup(yytext), currentLinha, currentColuna+countSpaces);} semicolon = 0;currentColuna+=yyleng+countSpaces;  countSpaces=0; if(flag==2){return NE;}}
"!"                                                          {if(flag==1) printf("NOT\n");if(flag==2){informacao(yylval.string = strdup(yytext), currentLinha, currentColuna+countSpaces);} semicolon = 0;currentColuna+=yyleng+countSpaces;  countSpaces=0; if(flag==2){return NOT;}}
"&&"                                                         {if(flag==1) printf("AND\n");if(flag==2){informacao(yylval.string = strdup(yytext), currentLinha, currentColuna+countSpaces);} semicolon = 0;currentColuna+=yyleng+countSpaces;  countSpaces=0; if(flag==2){return AND;}}
"||"                                                         {if(flag==1) printf("OR\n");if(flag==2){informacao(yylval.string = strdup(yytext), currentLinha, currentColuna+countSpaces);} semicolon = 0;currentColuna+=yyleng+countSpaces;  countSpaces=0; if(flag==2){return OR;}}
"}"                                                          {if(flag==1) printf("RBRACE\n");semicolon = 1;currentColuna+=yyleng+countSpaces;  countSpaces=0; if(flag==2){return RBRACE;}}
")"                                                          {if(flag==1) printf("RPAR\n");semicolon = 1;currentColuna+=yyleng+countSpaces;  countSpaces=0; if(flag==2){return RPAR;}}
"]"                                                          {if(flag==1) printf("RSQ\n");semicolon = 1;currentColuna+=yyleng+countSpaces;  countSpaces=0; if(flag==2){return RSQ;}}
"package"                                                    {if(flag==1) printf("PACKAGE\n");currentColuna+=yyleng+countSpaces;  countSpaces=0; if(flag==2){return PACKAGE;}}
"return"                                                     {if(flag==1) printf("RETURN\n");semicolon = 1;currentColuna+=yyleng+countSpaces;  countSpaces=0; if(flag==2){return RETURN;}}
"else"                                                       {if(flag==1) printf("ELSE\n");semicolon = 0;currentColuna+=yyleng+countSpaces;  countSpaces=0; if(flag==2){return ELSE;}}
"for"                                                        {if(flag==1) printf("FOR\n");semicolon = 0;currentColuna+=yyleng+countSpaces;  countSpaces=0; if(flag==2){return FOR;}}
"if"                                                         {if(flag==1) printf("IF\n");semicolon = 0;currentColuna+=yyleng+countSpaces;  countSpaces=0; if(flag==2){return IF;}}
"var"                                                        {if(flag==1) printf("VAR\n");semicolon = 0;currentColuna+=yyleng+countSpaces;  countSpaces=0; if(flag==2){return VAR;}}
"int"                                                        {if(flag==1) printf("INT\n");semicolon = 0;currentColuna+=yyleng+countSpaces;  countSpaces=0; if(flag==2){return INT;}}
"float32"                                                    {if(flag==1) printf("FLOAT32\n");semicolon = 0;currentColuna+=yyleng+countSpaces;  countSpaces=0; if(flag==2){return FLOAT32;}}
"bool"                                                       {if(flag==1) printf("BOOL\n");semicolon = 0;currentColuna+=yyleng+countSpaces;  countSpaces=0; if(flag==2){return BOOL;}}
"string"                                                     {if(flag==1) printf("STRING\n");semicolon = 0;currentColuna+=yyleng+countSpaces;  countSpaces=0; if(flag==2){return STRING;}}
"fmt.Println"                                                {if(flag==1) printf("PRINT\n");semicolon = 0;currentColuna+=yyleng+countSpaces;  countSpaces=0; if(flag==2){return PRINT;}}
"strconv.Atoi"                                               {if(flag==1) printf("PARSEINT\n");if(flag==2){informacao(yylval.string = strdup(yytext), currentLinha, currentColuna+countSpaces);} semicolon = 0;currentColuna+=yyleng+countSpaces; countSpaces=0; if(flag==2){return PARSEINT;}}
"func"                                                       {if(flag==1) printf("FUNC\n");semicolon = 0;currentColuna+=yyleng+countSpaces;  countSpaces=0; if(flag==2){return FUNC;}}
"os.Args"                                                    {if(flag==1) printf("CMDARGS\n");semicolon = 0;currentColuna+=yyleng+countSpaces;  countSpaces=0; if(flag==2){return CMDARGS;}}

    /*RESERVED*/
{reserved}                                                   {if(flag==1) printf("RESERVED(%s)\n",yytext);semicolon=0;currentColuna+=yyleng+countSpaces;  countSpaces=0; if(flag==2){return                                                                    RESERVED;}}

    /*ID*/
{letra}{alfanumerico}                                        {if(flag==1) printf("ID(%s)\n", yytext);if(flag==2){informacao(yylval.string = strdup(yytext), currentLinha, currentColuna+countSpaces);} semicolon = 1;currentColuna+=yyleng+countSpaces;  countSpaces=0; if(flag==2){return ID;}}
(" "|"\t")                                                   {countSpaces+=yyleng;}
.                                                            {currentColuna+=yyleng+countSpaces; countSpaces=0; erroLexical = 1; printf("Line %d, column %d: illegal character (%s)\n", currentLinha, currentColuna-(int)yyleng, &yytext[0]);semicolon = 0; if(flag==2){return yytext[0];}}
\n                                                           {if(semicolon==1 && flag==1)printf("SEMICOLON\n");linhaAnte = currentLinha; currentLinha++; colunaAnte = currentColuna+countSpaces; countSpaces = 0; currentColuna=1; if(semicolon==1){ semicolon = 0; if(flag==2){return SEMICOLON;} semicolon = 0;}}
    <<EOF>>                                                      {flagEOF = 1;if(semicolon==1 && flag==1)printf("SEMICOLON\n"); if(semicolon==1){ semicolon=0; if(flag==2){return SEMICOLON;}}else if(flag==2)return EOF; semicolon = 0; exit(0);}

%%

int main(int argc, char *argv[]) {
    if(argc>1 && strcmp(argv[1],"-l")==0){
        flag = 1;
        yylex();
    }
    else if(argc>1 && strcmp(argv[1],"-t")==0){
        flag = 2;
        printARV = 1;
        yyparse();
    }
    else if(argc>1 && strcmp(argv[1],"-s")==0){
        flag = 2;
        yyparse();
        if(printERRO==0 && erroLexical==0){
            Node* aux = (Node*) malloc(sizeof(Node));
            aux = program;
            Simbolo* head = TabelaSimbolos(aux);
            anotaAST(head, aux);
            errorUnused(head);
            printTabelaSimbolos(head->next);
            printARV = 1; 
        }
    }
    else{
        flag = 2;
        yyparse();
        if(printERRO==0 && erroLexical==0){
            Node* aux = (Node*) malloc(sizeof(Node));
            aux = program;
            Node* aux2 = (Node*) malloc(sizeof(Node));
            aux2 = program;
            Simbolo* head = TabelaSimbolos(aux);
            Simbolo* aux3 = head;
            anotaAST(head, aux);
            errorUnused(head);
            if(erroSemantico==0){
                convert(aux3, aux2);
            }
            freeAST(program);
        }
    }
    if(printARV == 1 && printERRO == 0) {
        printAST(program, 0);
    }
    return 0;
}

void informacao(char *string, int linha, int coluna){
    yylval.informacao.string = string;
    yylval.informacao.linha = linha;
    yylval.informacao.coluna = coluna;
}

int yywrap() {
    return 1;
}
