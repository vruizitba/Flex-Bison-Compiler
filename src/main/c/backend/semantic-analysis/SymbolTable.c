#include "SymbolTable.h"
#include "../../support/logging/Logger.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 8

/* TYPE_ERROR sentinel — static, never freed. */
static Type _typeErrorSentinel = { .kind = TYPEKIND_ERROR };

struct SymbolTable {
    Class ** classes;
    int classCount;
    int classCapacity;

    Function ** functions;
    int functionCount;
    int functionCapacity;

    const char * currentClass;
    const char * currentFunction;
};

/* ---- Lifecycle ---- */
SymbolTable * createSymbolTable() {
    SymbolTable * table = calloc(1, sizeof(SymbolTable));
    if (table == NULL) {
        return NULL;
    }
    table->classCapacity = INITIAL_CAPACITY;
    table->classes = malloc(table->classCapacity * sizeof(Class *));
    if (table->classes == NULL) {
        free(table);
        return NULL;
    }
    table->functionCapacity = INITIAL_CAPACITY;
    table->functions = malloc(table->functionCapacity * sizeof(Function *));
    if (table->functions == NULL) {
        free(table->classes);
        free(table);
        return NULL;
    }
    return table;
}

void destroySymbolTable(SymbolTable * table) {
    free(table->classes);
    free(table->functions);
    free(table);
}

/* ---- Registration ---- */
bool registerClass(SymbolTable * table, Class * classNode) {
    if (lookupClass(table, classNode->name) != NULL) {
        return false;
    }
    if (table->classCount == table->classCapacity) {
        int newCapacity = table->classCapacity * 2;
        Class ** resized = realloc(table->classes, newCapacity * sizeof(Class *));
        if (resized == NULL) {
            return false;
        }
        table->classes = resized;
        table->classCapacity = newCapacity;
    }
    table->classes[table->classCount++] = classNode;
    return true;
}

bool registerFunction(SymbolTable * table, Function * function) {
    if (lookupFunction(table, function->name) != NULL) {
        return false;
    }
    if (table->functionCount == table->functionCapacity) {
        int newCapacity = table->functionCapacity * 2;
        Function ** resized = realloc(table->functions, newCapacity * sizeof(Function *));
        if (resized == NULL) {
            return false;
        }
        table->functions = resized;
        table->functionCapacity = newCapacity;
    }
    table->functions[table->functionCount++] = function;
    return true;
}

/* ---- Lookup ---- */
Class * lookupClass(SymbolTable * table, const char * name) {
    for (int i = 0; i < table->classCount; i++) {
        if (strcmp(table->classes[i]->name, name) == 0) {
            return table->classes[i];
        }
    }
    return NULL;
}

Function * lookupFunction(SymbolTable * table, const char * name) {
    for (int i = 0; i < table->functionCount; i++) {
        if (strcmp(table->functions[i]->name, name) == 0) {
            return table->functions[i];
        }
    }
    return NULL;
}

FieldInfo * lookupField(SymbolTable * table, const char * className, const char * fieldName) {
    Class * class = lookupClass(table, className);
    while (class != NULL) {
        for (Field * field = class->fields; field != NULL; field = field->next) {
            if (strcmp(field->name, fieldName) == 0) {
                FieldInfo * info = malloc(sizeof(FieldInfo));
                if (info == NULL) {
                    return NULL;
                }
                info->field = field;
                info->ownerClassName = class->name;
                return info;
            }
        }
        class = class->parentName != NULL ? lookupClass(table, class->parentName) : NULL;
    }
    return NULL;
}

static int _paramCount(Parameter * params) {
    int count = 0;
    for (Parameter * param = params; param != NULL; param = param->next) {
        count++;
    }
    return count;
}

/* argCount < 0 matches any arity; type-based overload resolution is not yet implemented. */
MethodInfo * lookupMethod(SymbolTable * table, const char * className, const char * methodName, Type ** argTypes, int argCount) {
    (void)argTypes;
    Class * class = lookupClass(table, className);
    while (class != NULL) {
        for (Method * method = class->methods; method != NULL; method = method->next) {
            if (strcmp(method->name, methodName) != 0) {
                continue;
            }
            if (argCount >= 0 && _paramCount(method->parameters) != argCount) {
                continue;
            }
            MethodInfo * info = malloc(sizeof(MethodInfo));
            if (info == NULL) {
                return NULL;
            }
            info->method = method;
            info->ownerClassName = class->name;
            return info;
        }
        class = class->parentName != NULL ? lookupClass(table, class->parentName) : NULL;
    }
    return NULL;
}

/* ---- Scope stack ---- */
void pushScope(SymbolTable * table) {
    (void)table;
}

void popScope(SymbolTable * table) {
    (void)table;
}

bool declareVariable(SymbolTable * table, const char * name, Type * type) {
    (void)table;
    (void)name;
    (void)type;
    return true;
}

Type * lookupVariable(SymbolTable * table, const char * name) {
    (void)table;
    (void)name;
    return NULL;
}

/* ---- Type checker ---- */
Type * typeOf(SymbolTable * table, Expression * expr) {
    (void)table;
    (void)expr;
    return &_typeErrorSentinel;
}

bool isAssignable(SymbolTable * table, Type * from, Type * to) {
    (void)table;
    (void)from;
    (void)to;
    return true;
}

Type * typeError() {
    return &_typeErrorSentinel;
}

bool isTypeError(Type * type) {
    return type != NULL && type->kind == TYPEKIND_ERROR;
}

/* ---- Context ---- */
void setCurrentClass(SymbolTable * table, const char * className) {
    if (table != NULL) {
        table->currentClass = className;
    }
}

const char * getCurrentClass(SymbolTable * table) {
    return table != NULL ? table->currentClass : NULL;
}

void setCurrentFunction(SymbolTable * table, const char * functionName) {
    if (table != NULL) {
        table->currentFunction = functionName;
    }
}

const char * getCurrentFunction(SymbolTable * table) {
    return table != NULL ? table->currentFunction : NULL;
}
