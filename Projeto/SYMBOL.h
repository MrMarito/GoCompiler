#ifndef SYMBOL_H
#define SYMBOL_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "AST.h"


typedef struct Simbolo{
    char *name;
    char *parameter;
    char *type;
    bool isParameter;
    struct Simbolo *next;
    struct Simbolo *last;
    struct Simbolo *ramification;
    int linha;
    int coluna;
    int numeroParams;
    int used;
}Simbolo;


Simbolo* novoSimbolo(char* name, char* parameter, char* type, bool isParameter);
Simbolo* insertSimbolo(Simbolo* head, Simbolo* novo);
Simbolo* insertSimboloRamification(Simbolo* head, Simbolo* novo);
Simbolo* insertSimboloRamification2(Simbolo* head, Simbolo* novo);
Simbolo* insertFuncContent(Node* func, Simbolo* head);
Simbolo* insertFunc(Node* func, Simbolo* head);
Simbolo* TabelaSimbolos(Node* program);
void printTabelaSimbolos(Simbolo* head);
void anotaAST(Simbolo* head, Node* node);
char* searchType(Simbolo* head, char* novo, int linha, int coluna);
void assignRecursivo(Simbolo* head, Node* node);
int verificaOperadores(char* operador);
char* lowercase(char* string);
//erros
int searchSimbolo(Simbolo* head, Simbolo* novo);
int searchSimbolo2(Simbolo* head, Simbolo* novo);
Simbolo* search(Simbolo* head, Simbolo* search);
Simbolo* searchF(Simbolo* head, Node* node);
Simbolo* erroFunc(Simbolo* head, char* novo);
void checkErrors(Simbolo* head, Node* node);
void verificaErrosOperadores(Node* node);
char* operador(char* operador);
char* type(char* type);
void errorUnused(Simbolo* head);
void freeTable(Simbolo* head);

#endif
