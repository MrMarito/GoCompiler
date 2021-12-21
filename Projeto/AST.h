#ifndef AST_H
#define AST_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

typedef struct node{
    struct node* child;
    struct node* brother;
    char* id;
    char* type;
    char* token;
    int linha;
    int coluna;
    char* annotation;
    bool wasChecked;
    int numero;
}Node;

Node* newNode(char* id, char* token, int linha, int coluna);
void addBrother(Node* node, Node* node2);
void printAST(Node* node, int alt);
void freeAST(Node* node);

#endif
