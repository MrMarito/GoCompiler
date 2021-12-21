#include "AST.h"

//Functions
Node * newNode(char* id, char* token, int linha, int coluna){
    Node* auxNode = (Node*) malloc(sizeof(Node));
    auxNode->id = id;
    auxNode->type = NULL;
    auxNode->token = token;
    auxNode->child = NULL;
    auxNode->brother = NULL;
    auxNode->linha = linha;
    auxNode->coluna = coluna;
    auxNode->annotation = NULL;
    auxNode->wasChecked = false;
    auxNode->numero = 0;

    return auxNode;
}

void addBrother(Node* node1, Node* node2){
    Node* temp;
    temp = node1;
    while(temp->brother != NULL){
        temp=temp->brother;
    }
    temp->brother=node2;
}

void printAST(Node* node, int alt){
    char dots[alt*2];
    for (int i = 0; i< alt*2; i++){
        dots[i]='.';
    }
    dots[2*alt]='\0';
    if(strcmp(node->id, "StrLit")==0){
        if(node->annotation)
            printf("%s%s(\"%s)%s\n", dots, node->id, node->token, node->annotation);
        else
            printf("%s%s(\"%s)\n", dots, node->id, node->token);
    }
    else if(node->token!=NULL)
        if(node->annotation)
            printf("%s%s(%s)%s\n", dots, node->id, node->token, node->annotation);
        else
            printf("%s%s(%s)\n", dots, node->id, node->token);
    else{
        if(node->annotation)
            printf("%s%s%s\n", dots, node->id, node->annotation);
        else
            printf("%s%s\n", dots, node->id);
    }

    if (node->child)    printAST(node->child, alt+1);
    if (node->brother)  printAST(node->brother, alt);

    free(node);
}

void freeAST(Node* node){
    if (node->child)    freeAST(node->child);
    if (node->brother)  freeAST(node->brother);

    free(node);
}

