#include "LLVM.h"

int countVarLocal = 0;
bool returnCheck = false;
bool printCheck = false;


llvm* newLlvmFunc(char* name, bool global){
    llvm* aux = (llvm*)malloc(sizeof(llvm));
    aux->name = name;
    aux->type = NULL;
    aux->numero = -1;
    aux->global = global;
    aux->numeroAlloca = 0;
    aux->numeroLoad = 0;
    aux->numeroParam = 0;
    aux->numeroString = -1;
    aux->next = NULL;
    aux->last = NULL;
    return aux;
}


llvm* insertLlvm(llvm* head, llvm* novo){
    llvm* aux = head;
    while(aux->next)
        aux = aux->next;
    aux->next = novo;
    novo->numero = aux->numero+1;
    novo->last = aux;
    return head;
}


llvm* insertString(llvm* head, llvm* novo){
    llvm* aux = head;
    while(aux->next)
        aux = aux->next;
    aux->next = novo;
    novo->numeroString = aux->numeroString+1;
    novo->last = aux;
    return head;
}


llvm* searchLlvm(llvm* head, char* name){
    llvm* aux = head;
    while(aux){
        if(strcmp(aux->name, name)==0)
            return aux;
        aux = aux->next;
    }
    return NULL;
}


llvm* getLastLlvm(llvm* head){
    llvm* aux = head;
    while(aux->next)
        aux = aux->next;
    return aux;
}


char* type2(char* type){
    if(strcmp(type, "int")==0)
        return "i32";
    else if(strcmp(type, "float32")==0)
        return "double";
    else if(strcmp(type, "bool")==0)
        return "i1";
    else if(strcmp(type, "string")==0)
        return "i8*";
    else
        return "";
}


void globalVariable(Simbolo* head, char* name){
    Simbolo* aux = head;
    while(aux){
        if(!aux->ramification && strcmp(aux->name, name)==0){
            if(strcmp(type2(aux->type), "")!=0)
                printf("@%s = global %s 0\n", aux->name, type2(aux->type));
            else
                printf("@%s = global %s null\n", aux->name, type2(aux->type));
        }
        aux = aux->next;
    }
}


void defineFunc(Simbolo* head, Node* node, llvm* fun){
    Simbolo* aux = head;
    while(aux){
        if(aux->ramification && strcmp(aux->name, node->token)==0){
            if(strcmp(aux->ramification->type, "none")!=0)
                printf("define %s @%s(%s) {\n", type2(aux->ramification->type), aux->name, getParams(node, fun));
            else
                printf("define void @%s(%s) {\n", aux->name, getParams(node, fun));
            break;
        }
        aux = aux->next;
    }
    //alloca e store dos params
    int auxCount = 0;
    llvm* last = getLastLlvm(fun);
    if(last->numeroParam!=-1){
        auxCount = last->numero;
        auxCount++;//para contar com o Entry:
        while(fun->next){//nao esquecer que o primeiro elemento do fun e vazio!!
            if(fun->next->numeroString==-1){//para nao contar com as definicoes de strings
                fun->next->numeroAlloca = ++auxCount;
                fun->next->numero = fun->next->numeroAlloca;
                printf("    %%%d = alloca %s\n", fun->next->numeroAlloca, fun->next->type);
                printf("    store %s %%%s, %s* %%%d\n", fun->next->type, fun->next->name, fun->next->type, fun->next->numeroAlloca);
            }
            fun = fun->next;
        }
    }
}


char* getParams(Node* node, llvm* fun){
    char params[100];
    char par[1000];
    while(node->brother)//while para chegar ao irmao FuncParams
        node = node->brother;
    if(node->child){//se tiver Parametros, ou seja, ParamDecl
        node = node->child;
        while(node->brother){//ir a todos os ParamDecl
            llvm* aux = newLlvmFunc(node->child->brother->token, false);
            fun = insertLlvm(fun, aux);
            aux->type = type2(lowercase(node->child->id));
            aux->numeroParam = aux->numero;
            aux->numero = -1;
            sprintf(par, "%s %%%s,", type2(lowercase(node->child->id)), aux->name);
            strcat(params, par);
            node = node->brother;
        }
        if(node->child){//ultimo ParamDecl
            llvm* aux = newLlvmFunc(node->child->brother->token, false);
            fun = insertLlvm(fun, aux);
            aux->type = type2(lowercase(node->child->id));
            aux->numeroParam = aux->numero;
            aux->numero = -1;
            sprintf(par, "%s %%%s", type2(lowercase(node->child->id)), aux->name);
            strcat(params, par);
        }
        char* params2 = strdup(params);
        return params2;
    }
    return "";
}


llvm* assign(Node* node, llvm* fun){
    llvm* aux = searchLlvm(fun, node->child->brother->token);//vai buscar o que queres colocar na variavel
    llvm* new = newLlvmFunc("", false);
    fun = insertLlvm(fun, new);
    new->numeroParam = new->numero;
    printf("    %%%d = load %s, %s* %%%d\n", new->numero, aux->type, aux->type, aux->numero);
    llvm* aux2 = searchLlvm(fun, node->child->token);//vai buscar a variavel que queres alterar
    printf("    store %s %%%d, %s* %%%d\n", aux->type, new->numero, aux2->type, aux2->numero);
    return fun;
}


void funcReturn(Node* node, llvm* fun){
    if (node->child){
        char type[10];
        sscanf(node->child->annotation, " - %s", type);
        strcpy(type, type2(type));
        llvm* aux = searchLlvm(fun, node->child->token);
        if (strcmp(node->child->id, "Id")==0){
            llvm* new = newLlvmFunc("", false);
            fun = insertLlvm(fun, new);
            new->numeroParam = new->numero;
            printf("    %%%d = load %s, %s* %%%d\n", new->numero, type, type, aux->numeroAlloca);
            printf("    ret %s %%%d\n", type, new->numero);
        }
        else
            printf("    ret %s %s\n", type, node->child->token);
    }
    else
        printf("    ret void\n");
}


void funcBody(Simbolo* head, Node* node, llvm* fun){
    if(strcmp(node->id, "VarDecl")==0){
        llvm* last = getLastLlvm(fun);
        if(last->numero!=-1){//a funcao tem parametros
            llvm* aux = newLlvmFunc(node->child->brother->token, false);
            fun = insertLlvm(fun, aux);
            aux->type = type2(lowercase(node->child->id));
            aux->numeroAlloca = last->numero + 1;
            aux->numero = aux->numeroAlloca;
            printf("    %%%d = alloca %s\n", aux->numero, aux->type);
        }
        else{
            llvm* aux = newLlvmFunc(node->child->brother->token, false);
            fun = insertLlvm(fun, aux);
            aux->type = type2(lowercase(node->child->id));
            aux->numeroAlloca = 1;
            aux->numero = 1;
            printf("    %%%d = alloca %s\n", aux->numero, aux->type);
        }
    }
    if(strcmp(node->id, "Assign")==0){
        recursive(node->child->brother, fun);
        llvm* aux = searchLlvm(fun, node->child->token);
        llvm* last = getLastLlvm(fun);
        printf("    store %s %%%d, %s* %%%d\n", aux->type, last->numero, aux->type, aux->numero);
    }
    if(strcmp(node->id, "Return")==0){
        funcReturn(node, fun);
        returnCheck = true;
    }
    if(strcmp(node->id, "Print")==0){
        llvm* new = newLlvmFunc("", false);
        fun = insertLlvm(fun, new);
        llvm* aux = searchLlvm(fun, node->child->token);
        if(aux->numeroString==0)
            printf("    %%%d = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([%lu x i8], [%lu x i8]* @str, i64 0, i64 0))\n", new->numero, strlen(node->child->token), strlen(node->child->token));
        else
            printf("    %%%d = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([%lu x i8], [%lu x i8]* @str.%d, i64 0, i64 0))\n", new->numero, strlen(node->child->token), strlen(node->child->token), aux->numeroString);
    }
    
    if(node->child)             funcBody(head, node->child, fun);
    if(node->brother)           funcBody(head, node->brother, fun);
}


void recursive(Node* node, llvm* fun){
    if(strcmp(node->id, "Id")==0){
        llvm* aux = searchLlvm(fun, node->token);//vai buscar a variavel que queres alterar
        llvm* new = newLlvmFunc("", false);
        fun = insertLlvm(fun, new);
        printf("    %%%d = load %s, %s* %%%d\n", new->numero, aux->type, aux->type, aux->numero);
        node->numero = new->numero;
//        if(node->brother)
//            recursive(node->brother, fun);
    }
//        fun = assign(node, fun);
    else if(strcmp(node->id, "IntLit")==0){//
        llvm* aux = searchLlvm(fun, node->token);//vai buscar a variavel que queres alterar
        printf("    store i32 %s, %s* %%%d\n", node->token, aux->type, aux->numero);
    }
    else if(strcmp(node->id, "RealLit")==0){
        llvm* aux = searchLlvm(fun, node->token);//vai buscar a variavel que queres alterar
        printf("    store double %s, %s* %%%d\n", node->token, aux->type, aux->numero);
    }
    else if(strcmp(node->id, "Add")==0){
        recursive(node->child, fun);
        recursive(node->child->brother, fun);
        llvm* new = newLlvmFunc("", false);
        fun = insertLlvm(fun, new);
        llvm* aux = searchLlvm(fun, node->child->brother->token);
        printf("    %%%d = add %s %%%d, %%%d\n", new->numero, aux->type, node->child->numero, node->child->brother->numero);
        node->numero = new->numero;
    }
    /*else if(strcmp(node->id, "Mul")==0){
        recursive(node->child, fun);
        recursive(node->child->brother, fun);
        llvm* new = newLlvmFunc("", false);
        fun = insertLlvm(fun, new);
        llvm* aux = searchLlvm(fun, node->child->brother->token);
        printf("    %%%d = mul %s %%%d, %%%d\n", new->numero, aux->type, node->child->numero, node->child->brother->numero);
        node->numero = new->numero;
    }*/
}


void stringCheck(Node* node, llvm* fun){
    if(strcmp(node->id, "StrLit")==0){
        printCheck = true;
        llvm* new = newLlvmFunc(node->token, true);
        fun = insertString(fun, new);
        if(new->numeroString==0)
            printf("@.str = private unnamed_addr constant [%lu x i8] c\"%s\n", strlen(node->token), node->token);
        else
            printf("@.str.%d = private unnamed_addr constant [%lu x i8] c\"%s\n", new->numeroString, strlen(node->token), node->token);
    }
    if(node->child)             stringCheck(node->child, fun);
    if(node->brother)           stringCheck(node->brother, fun);
}


//declare i32 @atoi(...)
void convert(Simbolo* head, Node* node){
    bool cheackMain = false;
    Node* func = (Node*) malloc(sizeof(Node));
    func = node->child;
    while(func){
        if(strcmp(func->id, "VarDecl")==0){
            globalVariable(head, func->child->brother->token);
        }
        else if(strcmp(func->id, "FuncDecl")==0){
            if(strcmp(func->child->child->token, "main")==0)
                cheackMain = true;
            llvm* fun = newLlvmFunc("", false);
            stringCheck(func, fun);
            if(printCheck)
                printf("declare i32 @printf(i8*, ...)\n");
            defineFunc(head, func->child->child, fun);
            funcBody(head, func->child->brother, fun);//fornece FuncBody
            if(!returnCheck){
                char type[10];
                strcpy(type, lowercase(func->child->child->brother->id));
                strcpy(type, type2(type));
                if(strcmp(type, "i32")==0)
                    printf("    ret i32 0\n");
                else if(strcmp(type, "double")==0)
                    printf("    ret double 0.0\n");
                else if(strcmp(type, "i8*")==0)
                    printf("    ret i8* NULL\n");
                else{
                    llvm* new = newLlvmFunc("", false);
                    fun = insertLlvm(fun, new);
                    new->numeroParam = new->numero;
                    printf("    %%%d = alloca i1\n", new->numero);
                    llvm* new2 = newLlvmFunc("", false);
                    fun = insertLlvm(fun, new2);
                    new2->numeroParam = new2->numero;
                    printf("    %%%d = load i1, i1* %%%d\n", new2->numero, new2->last->numero);
                    printf("    ret i1 %%%d\n", new2->numero);
                }
            }
        returnCheck = false;
        printf("}\n");
        }
        func = func->brother;
    }
    if(!cheackMain){
//        printf("define i32 @main(i32 %%argc, i8** %%argv) {\n  ret i32 0\n}\n");
    }
}
