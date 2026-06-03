#include "SymbolTable.h"
#include "../../support/logging/Logger.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 8

/* Primitive type sentinels — static, never freed. */
static Type _typeErrorSentinel   = { .kind = TYPEKIND_ERROR  };
static Type _typeIntSentinel     = { .kind = TYPEKIND_INT    };
static Type _typeFloatSentinel   = { .kind = TYPEKIND_FLOAT  };
static Type _typeDoubleSentinel  = { .kind = TYPEKIND_DOUBLE };
static Type _typeCharSentinel    = { .kind = TYPEKIND_CHAR   };
static Type _typeBoolSentinel    = { .kind = TYPEKIND_BOOL   };
static Type _typeStringSentinel  = { .kind = TYPEKIND_STRING };
static Type _typeVoidSentinel    = { .kind = TYPEKIND_VOID   };

typedef struct {
    const char * name;
    Type * type;
} VarEntry;

typedef struct {
    VarEntry * varEntries;
    int varCount;
    int varCapacity;
} Scope;

struct SymbolTable {
    Class ** classes;
    int classCount;
    int classCapacity;

    Function ** functions;
    int functionCount;
    int functionCapacity;

    /* Scope stack — index 0 is the outermost scope, top is scopeCount-1. */
    Scope * scopes;
    int scopeCount;
    int scopeCapacity;

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
    table->scopeCapacity = INITIAL_CAPACITY;
    table->scopes = malloc(table->scopeCapacity * sizeof(Scope));
    if (table->scopes == NULL) {
        free(table->functions);
        free(table->classes);
        free(table);
        return NULL;
    }
    return table;
}

void destroySymbolTable(SymbolTable * table) {
    for (int i = 0; i < table->scopeCount; i++) {
        free(table->scopes[i].varEntries);
    }
    free(table->scopes);
    free(table->functions);
    free(table->classes);
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
    if (table->scopeCount == table->scopeCapacity) {
        int newCapacity = table->scopeCapacity * 2;
        Scope * resized = realloc(table->scopes, newCapacity * sizeof(Scope));
        if (resized == NULL) {
            return;
        }
        table->scopes = resized;
        table->scopeCapacity = newCapacity;
    }
    Scope * scope = &table->scopes[table->scopeCount++];
    scope->varCapacity = INITIAL_CAPACITY;
    scope->varCount = 0;
    scope->varEntries = malloc(scope->varCapacity * sizeof(VarEntry));
}

void popScope(SymbolTable * table) {
    if (table->scopeCount == 0) {
        return;
    }
    table->scopeCount--;
    free(table->scopes[table->scopeCount].varEntries);
}

/* Shadowing forbidden: checks every active scope, not just the current one. */
bool declareVariable(SymbolTable * table, const char * name, Type * type) {
    for (int i = 0; i < table->scopeCount; i++) {
        Scope * scope = &table->scopes[i];
        for (int j = 0; j < scope->varCount; j++) {
            if (strcmp(scope->varEntries[j].name, name) == 0) {
                return false;
            }
        }
    }
    Scope * current = &table->scopes[table->scopeCount - 1];
    if (current->varCount == current->varCapacity) {
        int newCapacity = current->varCapacity * 2;
        VarEntry * resized = realloc(current->varEntries, newCapacity * sizeof(VarEntry));
        if (resized == NULL) {
            return false;
        }
        current->varEntries = resized;
        current->varCapacity = newCapacity;
    }
    current->varEntries[current->varCount].name = name;
    current->varEntries[current->varCount].type = type;
    current->varCount++;
    return true;
}

Type * lookupVariable(SymbolTable * table, const char * name) {
    for (int i = table->scopeCount - 1; i >= 0; i--) {
        Scope * scope = &table->scopes[i];
        for (int j = 0; j < scope->varCount; j++) {
            if (strcmp(scope->varEntries[j].name, name) == 0) {
                return scope->varEntries[j].type;
            }
        }
    }
    return NULL;
}

/* ---- Type checker ---- */
Type * typeOf(SymbolTable * table, Expression * expr) {
    if (expr == NULL) {
        return &_typeErrorSentinel;
    }
    switch (expr->kind) {
        case EXPRESSION_INTEGER:    return &_typeIntSentinel;
        case EXPRESSION_FLOAT:      return &_typeFloatSentinel;
        case EXPRESSION_STRING:     return &_typeStringSentinel;
        case EXPRESSION_CHAR:       return &_typeCharSentinel;
        case EXPRESSION_BOOLEAN:    return &_typeBoolSentinel;
        case EXPRESSION_THIS: {
            const char * className = getCurrentClass(table);
            if (className == NULL) {
                return &_typeErrorSentinel;
            }
            Type * type = calloc(1, sizeof(Type));
            if (type == NULL) {
                return &_typeErrorSentinel;
            }
            type->kind = TYPEKIND_CLASS;
            type->className = (char *)className;
            return type;
        }
        case EXPRESSION_IDENTIFIER: {
            Type * type = lookupVariable(table, expr->identifier);
            return type != NULL ? type : &_typeErrorSentinel;
        }
        case EXPRESSION_NEW: {
            if (lookupClass(table, expr->newExpression.className) == NULL) {
                return &_typeErrorSentinel;
            }
            Type * type = calloc(1, sizeof(Type));
            if (type == NULL) {
                return &_typeErrorSentinel;
            }
            type->kind = TYPEKIND_CLASS;
            type->className = expr->newExpression.className;
            return type;
        }
        case EXPRESSION_BINARY: {
            Type * left = typeOf(table, expr->binary.left);
            Type * right = typeOf(table, expr->binary.right);
            if (isTypeError(left) || isTypeError(right)) {
                return &_typeErrorSentinel;
            }
            return left;
        }
        case EXPRESSION_UNARY:
            return typeOf(table, expr->unary.operand);
        default:
            return &_typeErrorSentinel;
    }
}

bool isAssignable(SymbolTable * table, Type * from, Type * to) {
    if (from == NULL || to == NULL) {
        return false;
    }
    if (isTypeError(from) || isTypeError(to)) {
        return false;
    }
    if (from->kind != to->kind) {
        return false;
    }
    if (from->kind != TYPEKIND_CLASS) {
        return true;
    }
    /* Class types: exact match or subclass of target. */
    if (strcmp(from->className, to->className) == 0) {
        return true;
    }
    Class * fromClass = lookupClass(table, from->className);
    while (fromClass != NULL && fromClass->parentName != NULL) {
        if (strcmp(fromClass->parentName, to->className) == 0) {
            return true;
        }
        fromClass = lookupClass(table, fromClass->parentName);
    }
    return false;
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
