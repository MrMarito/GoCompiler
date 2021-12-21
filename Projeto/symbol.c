#include "SYMBOL.h"

//Functions
extern int currentLinha, currentColuna, erroSemantico;
char* scope = "";
bool flag_anota = false;


Simbolo* novoSimbolo(char* name, char* parameter, char* type, bool isParameter){
    Simbolo* aux = (Simbolo*)malloc(sizeof(Simbolo));
    aux->name = name;
    aux->parameter = parameter;
    aux->type = type;
    aux->isParameter = isParameter;
    aux->next = NULL;
    aux->last = NULL;
    aux->ramification = NULL;
    aux->linha = 0;
    aux->coluna = 0;
    aux->numeroParams = 0;
    aux->used = 0;
    return aux;
}


Simbolo* insertSimbolo(Simbolo* head, Simbolo* novo){
    Simbolo* aux = head;
    while(aux->next){
        aux = aux->next;
    }
    aux->next = novo;
    novo->last = aux;
    return head;
}

Simbolo* insertSimboloRamification(Simbolo* head, Simbolo* novo){
    Simbolo* aux = head;
    while(aux->next){
        aux = aux->next;
    }
    aux->ramification = novo;
    return head;
}

Simbolo* insertSimboloRamification2(Simbolo* head, Simbolo* novo){//colocar dentro de uma ramification o next
    Simbolo* aux = head;
    while(aux->next){
        aux = aux->next;
    }
    aux = aux->ramification;
    while(aux->next)
        aux = aux->next;
    aux->next = novo;
    novo->last = aux;
    return head;
}


Simbolo* insertFuncContent(Node* func, Simbolo* head){//inserir na tabela da funcao os parametros
    Node* aux = (Node*) malloc(sizeof(Node));
    aux = func->child->child->brother;
    Node* aux3 = (Node*) malloc(sizeof(Node));
    aux3 = aux;
    while(aux3->brother)//while para chegar ao irmao FuncParams
        aux3 = aux3->brother;
    if(aux3->child){//se tiver Parametros, ou seja, se ParamDecl tiver filho
        aux3 = aux3->child;
        while(aux3->brother){//ir a todos os ParamDecl
            Simbolo* novo = novoSimbolo(aux3->child->brother->token, "", lowercase(aux3->child->id), true);
            if(searchSimbolo2(head, novo)==1)
                printf("Line %d, column %d: Symbol %s already defined\n", aux3->child->brother->linha, aux3->child->brother->coluna, novo->name);
            else{
                head = insertSimboloRamification2(head, novo);
            }
            aux3 = aux3->brother;
        }
        if(aux3->child){//ultimo ParamDecl
            Simbolo* novo = novoSimbolo(aux3->child->brother->token, "", lowercase(aux3->child->id), true);
            if(searchSimbolo2(head, novo)==1)
                printf("Line %d, column %d: Symbol %s already defined\n", aux3->child->brother->linha, aux3->child->brother->coluna, novo->name);
            else
                head = insertSimboloRamification2(head, novo);
        }
    }
    return head;
}


Simbolo* insertFunc(Node* func, Simbolo* head){
    char params[100];
    char par[1000];
    char type[100];
    int numeroParams = 0;
    Node* aux = (Node*) malloc(sizeof(Node));
    aux = func->child->child->brother;//sera igual ao tipo se tiver ou igual a FuncParams
    if(strcmp(aux->id, "FuncParams")!=0){//verificar se a funcao tem tipo
        strcpy(type, lowercase(aux->id));
    }
    else{
        sprintf(type, "none");
    }
    Node* aux3 = (Node*) malloc(sizeof(Node));
    aux3 = aux;
    while(aux3->brother)//while para chegar ao irmao FuncParams
        aux3 = aux3->brother;
    if(aux3->child){//se tiver Parametros, ou seja, se ParamDecl tiver filho
        aux3 = aux3->child;
        sprintf(params, "(");
        while(aux3->brother){//ir a todos os ParamDecl
            sprintf(par, "%s,", lowercase(aux3->child->id));
            strcat(params, par);
            numeroParams++;
            aux3 = aux3->brother;
        }
        if(aux3->child){//ultimo ParamDecl
            sprintf(par, "%s)", lowercase(aux3->child->id));
            strcat(params, par);
            numeroParams++;
        }
    }
    else{
        sprintf(params, "()");
    }
    char* aux4, *aux5;
    aux4 = strdup(params);
    aux5 = strdup(type);
    Simbolo* aux2 = novoSimbolo(func->child->child->token, aux4, aux5, false);
    aux2->numeroParams = numeroParams;
    if(search(head, aux2)!=NULL || searchSimbolo(head, aux2)==1){//erro de dupla definicao de funcoes
        erroSemantico = 1;
        printf("Line %d, column %d: Symbol %s already defined\n", func->child->child->linha, func->child->child->coluna, aux2->name);
    }
    else{
        aux2->linha = func->child->child->linha;
        aux2->coluna = func->child->child->coluna;
        head = insertSimbolo(head, aux2);
        head = insertSimboloRamification(head, novoSimbolo("return", "", aux5, false));
        head = insertFuncContent(func, head);
        Node* aux7 = (Node*) malloc(sizeof(Node));
        aux7 = func->child->brother;
        if(aux7->child){//entrar no FuncBody
            Node* aux9 = (Node*) malloc(sizeof(Node));
            aux9 = aux7->child;
            while(aux9){
                if(strcmp(aux9->id, "VarDecl")==0){
                    Simbolo* aux8 = novoSimbolo(aux9->child->brother->token, "", lowercase(aux9->child->id), false);
                    aux8->linha = aux9->child->brother->linha;
                    aux8->coluna = aux9->child->brother->coluna;
                    if(searchSimbolo2(head, aux8)==1){//erro de definicao de uma variavel dentro de uma funcao duas vezes
                        printf("Line %d, column %d: Symbol %s already defined\n", aux9->child->brother->linha, aux9->child->brother->coluna, aux8->name);
                        erroSemantico = 1;
                    }
                    else
                        head = insertSimboloRamification2(head, aux8);
                }
                aux9 = aux9->brother;
            }
        }
    }
    return head;
}

Simbolo* TabelaSimbolos(Node* program){
    Simbolo* head = novoSimbolo("", "", "", false);
    Node* func = (Node*) malloc(sizeof(Node));
    func = program->child;
    while(func){
        if(strcmp(func->id, "VarDecl")==0){
            Simbolo* aux = novoSimbolo(func->child->brother->token, "", lowercase(func->child->id), false);
            if(searchSimbolo(head, aux)==1 || search(head, aux)!=NULL){//corresponde ao erro de multipla definicao de variaveis globais
                printf("Line %d, column %d: Symbol %s already defined\n", func->child->brother->linha, func->child->brother->coluna, aux->name);
                erroSemantico = 1;
            }
            else
                head = insertSimbolo(head, aux);
        }
        else if(strcmp(func->id, "FuncDecl")==0){
            head = insertFunc(func, head);
        }
        func = func->brother;
    }
    return head;
}


void printTabelaSimbolos(Simbolo* head){
    printf("===== Global Symbol Table =====\n");
    Simbolo* aux = head;
    while(aux){
        printf("%s\t%s\t%s\n", aux->name, aux->parameter, aux->type);
        aux = aux->next;
    }
    printf("\n");
    while(head){
        if(head->ramification){
            printf("===== Function %s%s Symbol Table =====\n", head->name, head->parameter);
            while(head->ramification){
                if(head->ramification->isParameter==0)
                    printf("%s\t%s\t%s\n", head->ramification->name, head->ramification->parameter, head->ramification->type);
                else
                    printf("%s\t%s\t%s\tparam\n", head->ramification->name, head->ramification->parameter, head->ramification->type);
                head->ramification = head->ramification->next;
            }
            printf("\n");
        }
        head = head->next;
    }
}


void anotaAST(Simbolo* head, Node* node){
    if(strcmp(node->id, "FuncDecl")==0){
        if(searchF(head, node)==NULL){
            flag_anota = true;
            node->wasChecked = true;
        }
        else
            flag_anota = false;
    }
    if(!node->wasChecked){
        if(strcmp(node->id, "FuncHeader") == 0){
            scope = strdup(node->child->token);
        }
        if(strcmp(node->id, "Print")==0){
            assignRecursivo(head, node->child);
            if(node->child->annotation){
                if(strcmp(node->child->annotation, " - undef")==0){
                    erroSemantico = 1;
                    if(strcmp(node->child->id, "Call")==0)
                        printf("Line %d, column %d: Incompatible type %s in %s statement\n", node->child->child->linha, node->child->child->coluna, type(node->child->annotation), operador(node->id));
                    else
                        printf("Line %d, column %d: Incompatible type %s in %s statement\n", node->child->linha, node->child->coluna, type(node->child->annotation), operador(node->id));
                }
                if(strcmp(node->child->annotation, "")==0){
                    erroSemantico = 1;
                    printf("Line %d, column %d: Incompatible type undef in %s statement\n", node->child->child->linha, node->child->child->coluna, operador(node->id));
                }
            }
        }
        if(strcmp(node->id, "If")==0 || strcmp(node->id, "For")==0 || strcmp(node->id, "Else")==0){
            assignRecursivo(head, node->child);
            if(strcmp(node->child->id, "Block")!=0){//casos em que nao tem expressao
                if(strcmp(node->child->annotation, " - bool")!=0){
                    erroSemantico = 1;
                    printf("Line %d, column %d: Incompatible type %s in %s statement\n", node->child->linha, node->child->coluna, type(node->child->annotation), lowercase(node->id));
                }
            }
        }
        if(strcmp(node->id, "Assign")==0 || strcmp(node->id, "Call")==0 || strcmp(node->id, "ParseArgs")==0){
            assignRecursivo(head, node);
        }
        if(strcmp(node->id, "Return")==0){
            assignRecursivo(head, node);
            char auxString[100];
            if(node->child){
                Simbolo* aux = erroFunc(head, scope);
                sprintf(auxString, " - ");
                strcat(auxString, aux->ramification->type);
                if(strcmp(node->child->annotation, auxString)!=0){
                    erroSemantico = 1;
                    printf("Line %d, column %d: Incompatible type %s in %s statement\n", node->child->linha, node->child->coluna, type(node->child->annotation), lowercase(node->id));
                }
            }
        }
    }
    if(node->child && flag_anota==false)      anotaAST(head, node->child);
    if(node->brother)                         anotaAST(head, node->brother);
}

char* searchType(Simbolo* head, char* novo, int linha, int coluna){
    Simbolo* aux = head;
    Simbolo* aux2 = head;
    
    while(aux2){
        if(aux2->ramification && strcmp(aux2->name, scope) == 0){
            aux2 = aux2->ramification;
            while(aux2){
                if(strcmp(aux2->name, novo) == 0){
                    if(aux2->linha<linha || (aux2->linha == linha && aux2->coluna<coluna)){
                        aux2->used = 1;
                        return aux2->type;
                    }
                }
                aux2 = aux2->next;
            }
            break;
        }
        aux2 = aux2->next;
    }
    
    while(aux){
        if(!aux->ramification && strcmp(aux->name, novo) == 0){
            aux->used = 1;
            return aux->type;
        }
        aux = aux->next;
    }
    
    //erro
    return "undef";
}


void assignRecursivo(Simbolo* head, Node* node){
    node->wasChecked = true;
    if (strcmp(node->id, "Call") != 0){
        if(node->child){
            if (strcmp(node->child->id, "Id") == 0){
                char annotation[100];
                sprintf(annotation, " - %s", searchType(head, node->child->token, node->child->linha, node->child->coluna));
                node->child->annotation = strdup(annotation);
                if(strcmp(annotation, " - undef")==0){
                    erroSemantico = 1;
                    printf("Line %d, column %d: Cannot find symbol %s\n", node->child->linha, node->child->coluna, node->child->token);
                }
            }
            else if(strcmp(node->child->id, "Minus")==0 || strcmp(node->child->id, "Plus")==0){
                assignRecursivo(head, node->child->child);
                node->child->annotation = node->child->child->annotation;
                if(strcmp(node->child->child->annotation, " - bool")==0 || strcmp(node->child->child->annotation, " - undef")==0){
                    erroSemantico = 1;
                    printf("Line %d, column %d: Operator %s cannot be applied to type %s\n", node->child->child->linha, node->child->child->coluna, operador(node->child->id), type(node->child->child->annotation));
                }
            }
            else if(strcmp(node->child->id, "IntLit") == 0) node->child->annotation = " - int";
            else if(strcmp(node->child->id, "RealLit") == 0) node->child->annotation = " - float32";
            else
                assignRecursivo(head, node->child);
            if(node->child->brother){
                if (strcmp(node->child->brother->id, "Id") == 0){
                    char annotation[100];
                    sprintf(annotation, " - %s", searchType(head, node->child->brother->token, node->child->brother->linha, node->child->brother->coluna));
                    node->child->brother->annotation = strdup(annotation);
                    if(strcmp(annotation, " - undef")==0){
                        erroSemantico = 1;
                        printf("Line %d, column %d: Cannot find symbol %s\n", node->child->brother->linha, node->child->brother->coluna, node->child->brother->token);
                    }
                }
                else if(strcmp(node->child->brother->id, "Minus")==0 || strcmp(node->child->brother->id, "Plus")==0){
                    assignRecursivo(head, node->child->brother->child);
                    node->child->brother->annotation = node->child->brother->child->annotation;
                    if(strcmp(node->child->brother->child->annotation, " - bool")==0 || strcmp(node->child->brother->child->annotation, " - undef")==0){
                        erroSemantico = 1;
                        printf("Line %d, column %d: Operator %s cannot be applied to type %s\n", node->child->brother->child->linha, node->child->brother->child->coluna, operador(node->child->brother->id), type(node->child->brother->child->annotation));
                    }
                }
                else if(strcmp(node->child->brother->id, "IntLit") == 0) node->child->brother->annotation = " - int";
                else if(strcmp(node->child->brother->id, "RealLit") == 0) node->child->brother->annotation = " - float32";
                else
                    assignRecursivo(head, node->child->brother);
                if(node->child->annotation && node->child->brother->annotation){
                    if (strcmp(node->child->annotation, node->child->brother->annotation) == 0){
                        if(verificaOperadores(node->id)==1){
                            node->annotation = " - bool";
                            if(strcmp(node->id, "And")==0 && strcmp(node->child->annotation, " - bool")!=0){//erro de Operador And com tipos iguais q n sao bool
                                erroSemantico = 1;
                                printf("Line %d, column %d: Operator %s cannot be applied to types %s, %s\n", node->linha, node->coluna, operador(node->id), type(node->child->annotation), type(node->child->brother->annotation));
                            }
                            if((strcmp(node->id, "Lt")==0 || strcmp(node->id, "Le")==0 || strcmp(node->id, "Gt")==0 || strcmp(node->id, "Ge")==0) && strcmp(node->child->annotation, " - bool")==0){
                                erroSemantico = 1;
                                printf("Line %d, column %d: Operator %s cannot be applied to types %s, %s\n", node->linha, node->coluna, operador(node->id), type(node->child->annotation), type(node->child->brother->annotation));
                            }
                            if(strcmp(node->child->annotation, " - undef")==0){
                                erroSemantico = 1;
                                printf("Line %d, column %d: Operator %s cannot be applied to types %s, %s\n", node->linha, node->coluna, operador(node->id), type(node->child->annotation), type(node->child->brother->annotation));
                            }
                            if(strcmp(node->child->annotation, "")==0){
                                erroSemantico = 1;
                                printf("Line %d, column %d: Operator %s cannot be applied to types undef, undef\n", node->linha, node->coluna, operador(node->id));
                            }
                        }
                        else if((strcmp(node->id, "Mod")==0) && strcmp(node->child->annotation, " - int")!=0){
                            erroSemantico = 1;
                            printf("Line %d, column %d: Operator %s cannot be applied to types %s, %s\n", node->linha, node->coluna, operador(node->id),type(node->child->annotation), type(node->child->brother->annotation));
                            node->annotation = " - undef";
                            
                        }
                        else if((strcmp(node->id, "Add")==0 || strcmp(node->id, "Sub")==0 || strcmp(node->id, "Mul")==0 || strcmp(node->id, "Div")==0) && strcmp(node->child->annotation, " - bool")==0){
                            node->annotation = " - undef";
                            erroSemantico = 1;
                            printf("Line %d, column %d: Operator %s cannot be applied to types %s, %s\n", node->linha, node->coluna, operador(node->id), type(node->child->annotation), type(node->child->brother->annotation));
                        }
                        else if(strcmp(node->id, "ParseArgs")==0 && strcmp(node->child->annotation, " - int")!=0){
                            node->annotation = " - undef";
                            erroSemantico = 1;
                            printf("Line %d, column %d: Operator %s cannot be applied to types %s, %s\n", node->linha, node->coluna, operador(node->id), type(node->child->annotation), type(node->child->brother->annotation));
                        }
                        else if(strcmp(node->child->annotation, " - undef")==0){
                            node->annotation = " - undef";
                            erroSemantico = 1;
                            printf("Line %d, column %d: Operator %s cannot be applied to types %s, %s\n", node->linha, node->coluna, operador(node->id), type(node->child->annotation), type(node->child->brother->annotation));
                        }
                        else if(strcmp(node->child->annotation, "")==0){
                            node->annotation = " - undef";
                            erroSemantico = 1;
                            printf("Line %d, column %d: Operator %s cannot be applied to types undef, undef\n", node->linha, node->coluna, operador(node->id));
                        }
                        else
                            node->annotation = strdup(node->child->annotation);
                    }
                    else if(verificaOperadores(node->id)==1){
                        node->annotation = " - bool";
                        erroSemantico = 1;
                        printf("Line %d, column %d: Operator %s cannot be applied to types %s, %s\n", node->linha, node->coluna, operador(node->id), type(node->child->annotation), type(node->child->brother->annotation));
                    }
                    else if(strcmp(node->child->annotation, "")==0){
                        node->annotation = " - undef";
                        erroSemantico = 1;
                        printf("Line %d, column %d: Operator %s cannot be applied to types undef, %s\n", node->linha, node->coluna, operador(node->id), type(node->child->brother->annotation));
                    }
                    else if(strcmp(node->child->brother->annotation, "")==0){
                        node->annotation = " - undef";
                        erroSemantico = 1;
                        printf("Line %d, column %d: Operator %s cannot be applied to types %s, undef\n", node->linha, node->coluna, operador(node->id), type(node->child->annotation));
                    }
                    else{
                        node->annotation = " - undef";
                        erroSemantico = 1;
                        printf("Line %d, column %d: Operator %s cannot be applied to types %s, %s\n", node->linha, node->coluna, operador(node->id), type(node->child->annotation), type(node->child->brother->annotation));
                    }
                }
            }
            if(strcmp(node->id, "Not")==0){//erro tipo errado em relacao ao operador NOT
                node->annotation = " - bool";
                if(strcmp(node->child->annotation, " - bool")!=0){
                    erroSemantico = 1;
                    printf("Line %d, column %d: Operator %s cannot be applied to type %s\n", node->linha, node->coluna, operador(node->id), type(node->child->annotation));
                }
            }
        }
        else{//casos de if ou for ou while em que a expr e so uma variavel
            if (strcmp(node->id, "Id") == 0){
                char annotation[100];
                sprintf(annotation, " - %s", searchType(head, node->token, node->linha, node->coluna));
                node->annotation = strdup(annotation);
                if(strcmp(annotation, " - undef")==0){
                    erroSemantico = 1;
                    printf("Line %d, column %d: Cannot find symbol %s\n", node->linha, node->coluna, node->token);
                }
            }
            else if(strcmp(node->id, "IntLit") == 0) node->annotation = " - int";
            else if(strcmp(node->id, "RealLit") == 0) node->annotation = " - float32";
        }
    }
    else{
        Simbolo* aux = erroFunc(head, node->child->token);//devolve a funcao que estamos a dar call
        char params[100]; //Quase de certeza que temos de trocar isto para mallocs
        int countParams = 0;
        char verificaTipos[1000];
        char params2[1000];
        params[0] = '\0';
        node->annotation = "";
        if(aux){
            sprintf(params, " - %s", aux->parameter);
            node->child->annotation = strdup(params);
            Simbolo* aux2 = aux->ramification;
            if(strcmp(aux2->type, "none")==0)
                params[0] = '\0';
            else
                sprintf(params, " - %s", aux2->type);
            node->annotation = strdup(params);
            params[0] = '\0';
            int erroParams = 0;
            if(node->child->brother){//se tu chamares sem parametro mas ela tiver podemos apanhar este erro com o else deste if!!!!!!!!
                Node* aux3 = node->child->brother;
                sprintf(params, "(");
                while(aux2){
                    if(aux2->isParameter){//procura params dentro da tabela de simbolos da funcao
                        countParams++;
                        if(strcmp(aux3->id, "Add")==0 || strcmp(aux3->id, "Sub")==0 || strcmp(aux3->id, "Mul")==0 || strcmp(aux3->id, "Div")==0 || strcmp(aux3->id, "Mod")==0 || strcmp(aux3->id, "Minus")==0 || strcmp(aux3->id, "Plus")==0 || strcmp(aux3->id, "Call")==0 || verificaOperadores(aux3->id)==1)
                            assignRecursivo(head, aux3);
                        else{
                            if (strcmp(aux3->id, "Id") == 0){
                                char annotation[100];
                                sprintf(annotation, " - %s", searchType(head, aux3->token, aux3->linha, aux3->coluna));
                                aux3->annotation = strdup(annotation);
                                if(strcmp(annotation, " - undef")==0){
                                    erroSemantico = 1;
                                    printf("Line %d, column %d: Cannot find symbol %s\n", aux3->linha, aux3->coluna, aux3->token);
                                }
                            }
                            else if(strcmp(aux3->id, "IntLit") == 0)  aux3->annotation = " - int";
                            else if(strcmp(aux3->id, "RealLit") == 0) aux3->annotation = " - float32";
                        }
                        if(strcmp(aux3->id, "Call") == 0){
                            Simbolo* aux4 = erroFunc(head, aux3->child->token);
                            sscanf(aux4->ramification->type, "%s", params2);
                            sscanf(aux4->ramification->type, "%s", verificaTipos);
//                            aux3->annotation = aux4->ramification->type;
                        }
                        else{
                            sscanf(aux3->annotation, " - %s", params2);
                            sscanf(aux3->annotation, " - %s", verificaTipos);
                        }
                        strcat(params, params2);
                        strcat(params, ",");
                        if(strcmp(aux2->type, verificaTipos)!=0){
                            erroParams = 1;
                        }
                        if(aux3->brother){//testa se chamaste com o numero certo de argumentos, se chamares com menos dava segfault
                            aux3 = aux3->brother;
                        }
                        else
                            break;
                    }
                    aux2 = aux2->next;
                }
                if(aux3->brother || (aux3 && countParams==0)){//se for chamada com mais parametros do que devia
                    while(aux3){
                        countParams++;
                        if(strcmp(aux3->id, "Add")==0 || strcmp(aux3->id, "Sub")==0 || strcmp(aux3->id, "Mul")==0 || strcmp(aux3->id, "Div")==0 || strcmp(aux3->id, "Mod")==0 || strcmp(aux3->id, "Minus")==0 || strcmp(aux3->id, "Plus")==0 || strcmp(aux3->id, "Call")==0 || verificaOperadores(aux3->id)==1)
                            assignRecursivo(head, aux3);
                        else{
                            if (strcmp(aux3->id, "Id") == 0){
                                char annotation[100];
                                sprintf(annotation, " - %s", searchType(head, aux3->token, aux3->linha, aux3->coluna));
                                aux3->annotation = strdup(annotation);
                                if(strcmp(annotation, " - undef")==0){
                                    erroSemantico = 1;
                                    printf("Line %d, column %d: Cannot find symbol %s\n", aux3->linha, aux3->coluna, aux3->token);
                                }
                            }
                            else if(strcmp(aux3->id, "IntLit") == 0)  aux3->annotation = " - int";
                            else if(strcmp(aux3->id, "RealLit") == 0) aux3->annotation = " - float32";
                        }
                        if(strcmp(aux3->id, "Call") == 0){
                            Simbolo* aux4 = erroFunc(head, aux3->child->token);
                            sscanf(aux4->ramification->type, "%s", params2);
                            sscanf(aux4->ramification->type, "%s", verificaTipos);
                            aux3->annotation = aux4->ramification->type;
                        }
                        else{
                            sscanf(aux3->annotation, " - %s", params2);
                            sscanf(aux3->annotation, " - %s", verificaTipos);
                        }
                        strcat(params, params2);
                        strcat(params, ",");
                        if(aux3->brother){
                            aux3 = aux3->brother;
                        }
                        else
                            break;
                    }
                }
            }
            else{//chamar sem parametros qnd tem
                
            }
            if(aux->numeroParams!=countParams || erroParams==1){
                erroSemantico = 1;
                node->annotation = " - undef";
                if(node->child->brother){
                    params[strlen(params)-1] = ')';
                    printf("Line %d, column %d: Cannot find symbol %s%s\n", node->child->linha, node->child->coluna, node->child->token, params);
                }
                else{
                    sprintf(params, "()");
                    printf("Line %d, column %d: Cannot find symbol %s%s\n", node->child->linha, node->child->coluna, node->child->token, params);
                }
            }
            
        }
        //se tiver masi brother e pq foi chamada com demasiados parametros e podemos verificar em cima
        else{//anota uma chamada de uma funcao que nao foi definida para poder dar catch ao erro de chamadas de funcoes nao definidas
            char par[1000];
            node->annotation = " - undef";
            if(node->child->brother){//vai anotar a chamada de funcoes que nao estao definidas
                Node* aux4 = node->child->brother;
                while(aux4){
                    if(strcmp(aux4->id, "Add")==0 || strcmp(aux4->id, "Sub")==0 || strcmp(aux4->id, "Mul")==0 || strcmp(aux4->id, "Div")==0 || strcmp(aux4->id, "Mod")==0 || strcmp(aux4->id, "Minus")==0 || strcmp(aux4->id, "Plus")==0 || strcmp(aux4->id, "Call")==0 || verificaOperadores(aux4->id)==1)
                        assignRecursivo(head, aux4);
                    else{
                        if (strcmp(aux4->id, "Id") == 0){
                            char annotation[100];
                            sprintf(annotation, " - %s", searchType(head, aux4->token, aux4->linha, aux4->coluna));
                            aux4->annotation = strdup(annotation);
                            if(strcmp(annotation, " - undef")==0){
                                erroSemantico = 1;
                                printf("Line %d, column %d: Cannot find symbol %s\n", aux4->linha, aux4->coluna, aux4->token);
                            }
                        }
                        else if(strcmp(aux4->id, "IntLit") == 0) aux4->annotation = " - int";
                        else if(strcmp(aux4->id, "RealLit") == 0) aux4->annotation = " - float32";
                    }
                    if(aux4->brother)
                        aux4 = aux4->brother;
                    else
                        break;
                }
            }
            else
                sprintf(params, "()");
            if(node->child->brother){//depois de a chamada estar anotada vai guardar no params o tipo dos parametros anotados
                Node* aux5 = node->child->brother;
                sprintf(params, "(");
                while(aux5){
                    sscanf(aux5->annotation, " - %s", par);
                    strcat(params, par);
                    strcat(params, ",");
                    if(aux5->brother)
                        aux5 = aux5->brother;
                    else
                        break;
                }
                params[strlen(params)-1] = ')';
            }
            printf("Line %d, column %d: Cannot find symbol %s%s\n", node->child->linha, node->child->coluna, node->child->token, params);
            erroSemantico = 1;
        }
    }
}


int verificaOperadores(char* operador){
    if(strcmp(operador, "Or")==0  ||
       strcmp(operador, "Lt")==0  ||
       strcmp(operador, "Gt")==0  ||
       strcmp(operador, "Eq")==0  ||
       strcmp(operador, "Ne")==0  ||
       strcmp(operador, "Le")==0  ||
       strcmp(operador, "Ge")==0  ||
       strcmp(operador, "Not")==0 ||
       strcmp(operador, "And")==0){
        return 1;
    }
    return 0;
}


char* lowercase(char* string){
    char* aux = (char*)malloc(strlen(string) + 1);
    for(int i = 0; i<strlen(string); i++){
        aux[i] = tolower(string[i]);
    }
    aux[strlen(string)]='\0';
    return aux;
}


//erros
int searchSimbolo(Simbolo* head, Simbolo* novo){
    Simbolo* aux = head;
    while(aux){
        if(!aux->ramification && strcmp(aux->name, novo->name)==0)
            return 1;
        aux = aux->next;
    }
    return 0;
}


int searchSimbolo2(Simbolo* head, Simbolo* novo){//verificar se nao sao definidas duas variaveis iguais numa funcao
    Simbolo* aux = head;
    while(aux->next){
        aux = aux->next;
    }
    aux = aux->ramification;
    while(aux){
        if(!aux->ramification && strcmp(aux->name, novo->name)==0)
            return 1;
        aux = aux->next;
    }
    return 0;
}


Simbolo* search(Simbolo* head, Simbolo* search){
    Simbolo* aux = head;
    while(aux){
        if(aux->ramification && strcmp(aux->name, search->name)==0)
            return aux;
        aux = aux->next;
    }
    return NULL;
}


Simbolo* searchF(Simbolo* head, Node* node){
    Simbolo* aux = head;
    while(aux){
        if(aux->ramification && strcmp(aux->name, node->child->child->token)==0){
            if(aux->linha == node->child->child->linha && aux->coluna == node->child->child->coluna)
                return aux;
        }
        aux = aux->next;
    }
    return NULL;
}


//verificar se uma funcao pode ser declarada, ou seja, se ja n foi declarada antes !!!ATENCAO PODE FALTAR VERIFICAR FUNCAO CHAMADAS DENTRO DE OUTRAS FUNCOES
Simbolo* erroFunc(Simbolo* head, char* novo){//recebe a tabela de simbolos e o nome da funcao a verificar
    Simbolo* aux = head;
    while(aux){
        if(aux->ramification && strcmp(aux->name, novo)==0)
            return aux;
        aux = aux->next;
    }
    return NULL;
}

void checkErrors(Simbolo* head, Node* node){
    if(strcmp(node->id, "FuncHeader")==0){
        if(!erroFunc(head, node->child->token))
            printf("Line %d, column %d: Symbol %s already defined\n", node->child->linha, node->child->coluna, node->child->token);
    }
    if(strcmp(node->id, "Call")==0){
        Simbolo* aux = erroFunc(head, node->child->token);
        if(aux)
            printf("Line %d, column %d: Cannot find Symbol %s%s\n", node->child->linha, node->child->coluna, node->child->token, node->child->id);
    }
    
    if(node->child)     checkErrors(head, node->child);
    if(node->brother)   checkErrors(head, node->brother);
}


char* operador(char* operador){
    if(strcmp(operador, "Or")==0)
        return "||";
    else if(strcmp(operador, "And")==0)
        return "&&";
    else if(strcmp(operador, "Lt")==0)
        return "<";
    else if(strcmp(operador, "Gt")==0)
        return ">";
    else if(strcmp(operador, "Eq")==0)
        return "==";
    else if(strcmp(operador, "Ne")==0)
        return "!=";
    else if(strcmp(operador, "Le")==0)
        return "<=";
    else if(strcmp(operador, "Ge")==0)
        return ">=";
    else if(strcmp(operador, "Add")==0)
        return "+";
    else if(strcmp(operador, "Sub")==0)
        return "-";
    else if(strcmp(operador, "Mul")==0)
        return "*";
    else if(strcmp(operador, "Div")==0)
        return "/";
    else if(strcmp(operador, "Mod")==0)
        return "%";
    else if(strcmp(operador, "Not")==0)
        return "!";
    else if(strcmp(operador, "Minus")==0)
        return "-";
    else if(strcmp(operador, "Plus")==0)
        return "+";
    else if(strcmp(operador, "Assign")==0)
        return "=";
    else if(strcmp(operador, "ParseArgs")==0)
        return "strconv.Atoi";
    else if(strcmp(operador, "Print")==0)
        return "fmt.Println";
    return NULL;
}


char* type(char* type){
    if(strcmp(type, " - int")==0)
        return "int";
    else if(strcmp(type, " - bool")==0)
        return "bool";
    else if(strcmp(type, " - float32")==0)
        return "float32";
    else if(strcmp(type, " - undef")==0)
        return "undef";
    else if(strcmp(type, " - string")==0)
        return "string";
    return NULL;
}


void errorUnused(Simbolo* head){
    Simbolo* aux2 = head;
    while(aux2){
        if(aux2->ramification){
            Simbolo* aux3 = aux2->ramification->next;
            while(aux3){
                if(!aux3->isParameter && aux3->used==0){
                    erroSemantico = 1;
                    printf("Line %d, column %d: Symbol %s declared but never used\n", aux3->linha, aux3->coluna, aux3->name);
                }
                aux3 = aux3->next;
            }
        }
        aux2 = aux2->next;
    }
}


void freeTable(Simbolo* head){
    if(head->next)              freeTable(head->next);
    if(head->ramification)      freeTable(head->ramification);
    freeTable(head);
}
