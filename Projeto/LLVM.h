#ifndef LLVM_H
#define LLVM_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "SYMBOL.h"


typedef struct llvm{
	char* name;
	char* type;
	int numero;
	int numeroAlloca;
	int numeroLoad;
	int numeroParam;
	int numeroString;
	bool global;
	struct llvm* next;
	struct llvm* last;
}llvm;

llvm* newLlvmFunc(char* name, bool global);
llvm* insertLlvm(llvm* head, llvm* novo);
llvm* insertString(llvm* head, llvm* novo);
llvm* searchLlvm(llvm* head, char* name);
llvm* getLastLlvm(llvm* head);
char* type2(char* type);
void globalVariable(Simbolo* head, char* name);
void defineFunc(Simbolo* head, Node* node, llvm* fun);
char* getParams(Node* node, llvm* fun);
llvm* assign(Node* node, llvm* fun);
void funcReturn(Node* node, llvm* fun);
void funcBody(Simbolo* head, Node* node, llvm* fun);
void recursive(Node* node, llvm* fun);
void stringCheck(Node* node, llvm* fun);
void convert(Simbolo* head, Node* node);


#endif
