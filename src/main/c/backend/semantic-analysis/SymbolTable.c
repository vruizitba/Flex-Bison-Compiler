#include "SymbolTable.h"
#include "../../support/logging/Logger.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 8

/* Primitive type sentinels — static, never freed. */
static Type _typeErrorSentinel  = { .kind = TYPEKIND_ERROR  };
static Type _typeIntSentinel    = { .kind = TYPEKIND_INT    };
static Type _typeFloatSentinel  = { .kind = TYPEKIND_FLOAT  };
static Type _typeDoubleSentinel = { .kind = TYPEKIND_DOUBLE };
static Type _typeCharSentinel   = { .kind = TYPEKIND_CHAR   };
static Type _typeBoolSentinel   = { .kind = TYPEKIND_BOOL   };
static Type _typeStringSentinel = { .kind = TYPEKIND_STRING };
static Type _typeVoidSentinel   = { .kind = TYPEKIND_VOID   };

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
    Type  ** classTypes;
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
    table->classTypes = malloc(table->classCapacity * sizeof(Type *));
    if (table->classTypes == NULL) {
        free(table->classes);
        free(table);
        return NULL;
    }
    table->functionCapacity = INITIAL_CAPACITY;
    table->functions = malloc(table->functionCapacity * sizeof(Function *));
    if (table->functions == NULL) {
        free(table->classTypes);
        free(table->classes);
        free(table);
        return NULL;
    }
    table->scopeCapacity = INITIAL_CAPACITY;
    table->scopes = malloc(table->scopeCapacity * sizeof(Scope));
    if (table->scopes == NULL) {
        free(table->functions);
        free(table->classTypes);
        free(table->classes);
        free(table);
        return NULL;
    }
    return table;
}

void destroySymbolTable(SymbolTable * table) {
    for (int i = 0; i < table->classCount; i++) {
        free(table->classTypes[i]);
    }
    free(table->classTypes);
    for (int i = 0; i < table->scopeCount; i++) {
        free(table->scopes[i].varEntries);
    }
    free(table->scopes);
    free(table->functions);
    free(table->classes);
    free(table);
}

/* ---- Registration ---- */
static bool _typesEqual(Type * a, Type * b) {
    if (a == NULL && b == NULL) {
        return true;
    }
    if (a == NULL || b == NULL) {
        return false;
    }
    if (a->kind != b->kind) {
        return false;
    }
    if (a->kind == TYPEKIND_CLASS) {
        return strcmp(a->className, b->className) == 0;
    }
    return true;
}

static bool _signaturesConflict(Method * a, Method * b) {
    if (strcmp(a->name, b->name) != 0) {
        return false;
    }
    Parameter * pa = a->parameters;
    Parameter * pb = b->parameters;
    while (pa != NULL && pb != NULL) {
        if (!_typesEqual(pa->type, pb->type)) {
            return false;
        }
        pa = pa->next;
        pb = pb->next;
    }
    return pa == NULL && pb == NULL;
}

bool registerClass(SymbolTable * table, Class * classNode) {
    if (lookupClass(table, classNode->name) != NULL) {
        return false;
    }
    for (Method * m1 = classNode->methods; m1 != NULL; m1 = m1->next) {
        for (Method * m2 = m1->next; m2 != NULL; m2 = m2->next) {
            if (_signaturesConflict(m1, m2)) {
                return false;
            }
        }
    }
    for (Field * f1 = classNode->fields; f1 != NULL; f1 = f1->next) {
        for (Field * f2 = f1->next; f2 != NULL; f2 = f2->next) {
            if (strcmp(f1->name, f2->name) == 0) {
                return false;
            }
        }
    }
    if (table->classCount == table->classCapacity) {
        int newCapacity = table->classCapacity * 2;
        Class ** resizedClasses = realloc(table->classes, newCapacity * sizeof(Class *));
        if (resizedClasses == NULL) {
            return false;
        }
        table->classes = resizedClasses;
        Type ** resizedTypes = realloc(table->classTypes, newCapacity * sizeof(Type *));
        if (resizedTypes == NULL) {
            return false;
        }
        table->classTypes = resizedTypes;
        table->classCapacity = newCapacity;
    }
    Type * classType = calloc(1, sizeof(Type));
    if (classType == NULL) {
        return false;
    }
    classType->kind = TYPEKIND_CLASS;
    classType->className = classNode->name;
    table->classes[table->classCount] = classNode;
    table->classTypes[table->classCount++] = classType;
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

bool hasInheritanceCycle(SymbolTable * table, const char * className) {
    Class * current = lookupClass(table, className);
    int steps = 0;
    while (current != NULL && current->parentName != NULL) {
        current = lookupClass(table, current->parentName);
        if (current == NULL) {
            return false; /* undefined parent: not a cycle (reported elsewhere) */
        }
        if (++steps > table->classCount) {
            return true;
        }
    }
    return false;
}

static Type * lookupClassType(SymbolTable * table, const char * name) {
    for (int i = 0; i < table->classCount; i++) {
        if (strcmp(table->classes[i]->name, name) == 0) {
            return table->classTypes[i];
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

static bool _argsMatchParams(SymbolTable * table, Type ** argTypes, int argCount, Parameter * params) {
    Parameter * p = params;
    for (int i = 0; i < argCount && p != NULL; i++, p = p->next) {
        if (!isAssignable(table, argTypes[i], p->type)) {
            return false;
        }
    }
    return true;
}

/* argCount < 0 matches any arity; argTypes NULL skips type-based overload resolution. */
MethodInfo * lookupMethod(SymbolTable * table, const char * className, const char * methodName, Type ** argTypes, int argCount) {
    Class * class = lookupClass(table, className);
    while (class != NULL) {
        for (Method * method = class->methods; method != NULL; method = method->next) {
            bool skip = strcmp(method->name, methodName) != 0;
            if (!skip && argCount >= 0) {
                skip = _paramCount(method->parameters) != argCount;
            }
            if (!skip && argTypes != NULL && argCount > 0) {
                skip = !_argsMatchParams(table, argTypes, argCount, method->parameters);
            }
            if (!skip) {
                MethodInfo * info = malloc(sizeof(MethodInfo));
                if (info == NULL) {
                    return NULL;
                }
                info->method = method;
                info->ownerClassName = class->name;
                return info;
            }
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
    Scope * scope = &table->scopes[table->scopeCount];
    scope->varCapacity = INITIAL_CAPACITY;
    scope->varCount = 0;
    scope->varEntries = malloc(scope->varCapacity * sizeof(VarEntry));
    if (scope->varEntries == NULL) {
        return;
    }
    table->scopeCount++;
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
typedef enum {
    NUMERIC_RANK_NONE   = -1,
    NUMERIC_RANK_CHAR   = 0,
    NUMERIC_RANK_INT    = 1,
    NUMERIC_RANK_FLOAT  = 2,
    NUMERIC_RANK_DOUBLE = 3
} NumericRank;

static NumericRank _numericRank(TypeKind kind) {
    switch (kind) {
        case TYPEKIND_CHAR:
            return NUMERIC_RANK_CHAR;
        case TYPEKIND_INT:
            return NUMERIC_RANK_INT;
        case TYPEKIND_FLOAT:
            return NUMERIC_RANK_FLOAT;
        case TYPEKIND_DOUBLE:
            return NUMERIC_RANK_DOUBLE;
        default:
            return NUMERIC_RANK_NONE;
    }
}

static Type * _widenNumeric(Type * left, Type * right) {
    NumericRank leftRank  = _numericRank(left->kind);
    NumericRank rightRank = _numericRank(right->kind);
    if (leftRank == NUMERIC_RANK_NONE || rightRank == NUMERIC_RANK_NONE) {
        return &_typeErrorSentinel;
    }
    switch (leftRank >= rightRank ? leftRank : rightRank) {
        case NUMERIC_RANK_CHAR:
            return &_typeCharSentinel;
        case NUMERIC_RANK_INT:
            return &_typeIntSentinel;
        case NUMERIC_RANK_FLOAT:
            return &_typeFloatSentinel;
        case NUMERIC_RANK_DOUBLE:
            return &_typeDoubleSentinel;
        default:
            return &_typeErrorSentinel;
    }
}

static bool _isSubclassOrEqual(SymbolTable * table, const char * child, const char * ancestor) {
    if (child == NULL || ancestor == NULL) {
        return false;
    }
    if (strcmp(child, ancestor) == 0) {
        return true;
    }
    Class * c = lookupClass(table, child);
    while (c != NULL && c->parentName != NULL) {
        if (strcmp(c->parentName, ancestor) == 0) {
            return true;
        }
        c = lookupClass(table, c->parentName);
    }
    return false;
}

/* public: always; private: only the owner; protected: the owner or a subclass. */
static bool _isAccessible(SymbolTable * table, Visibility visibility, const char * ownerName, const char * currentClass) {
    switch (visibility) {
        case VISIBILITY_PUBLIC:
            return true;
        case VISIBILITY_PRIVATE:
            return currentClass != NULL && strcmp(currentClass, ownerName) == 0;
        case VISIBILITY_PROTECTED:
            return _isSubclassOrEqual(table, currentClass, ownerName);
    }
    return false;
}

static bool _areComparable(SymbolTable * table, Type * a, Type * b) {
    if (_numericRank(a->kind) != NUMERIC_RANK_NONE && _numericRank(b->kind) != NUMERIC_RANK_NONE) {
        return true;
    }
    if (a->kind == TYPEKIND_BOOL && b->kind == TYPEKIND_BOOL) {
        return true;
    }
    if (a->kind == TYPEKIND_STRING && b->kind == TYPEKIND_STRING) {
        return true;
    }
    if (a->kind == TYPEKIND_CLASS && b->kind == TYPEKIND_CLASS) {
        return isAssignable(table, a, b) || isAssignable(table, b, a);
    }
    return false;
}

static Type * _resolveUnaryType(SymbolTable * table, Expression * expr) {
    Type * operand = resolveExpressionType(table, expr->unary.operand);
    if (isTypeError(operand)) {
        return &_typeErrorSentinel;
    }
    switch (expr->unary.operator) {
        case UNARY_OPERATOR_NOT:
            return operand->kind == TYPEKIND_BOOL ? &_typeBoolSentinel : &_typeErrorSentinel;
        case UNARY_OPERATOR_NEGATE:
        case UNARY_OPERATOR_PRE_INCREMENT:
        case UNARY_OPERATOR_PRE_DECREMENT:
        case UNARY_OPERATOR_POST_INCREMENT:
        case UNARY_OPERATOR_POST_DECREMENT:
            return _numericRank(operand->kind) != NUMERIC_RANK_NONE ? operand : &_typeErrorSentinel;
        case UNARY_OPERATOR_DEREFERENCE:
        case UNARY_OPERATOR_ADDRESS_OF:
        default:
            return &_typeErrorSentinel;
    }
}

static Type * _resolveBinaryType(SymbolTable * table, Expression * expr) {
    Type * left = resolveExpressionType(table, expr->binary.left);
    Type * right = resolveExpressionType(table, expr->binary.right);
    if (isTypeError(left) || isTypeError(right)) {
        return &_typeErrorSentinel;
    }
    switch (expr->binary.operator) {
        case BINARY_OPERATOR_ADD:
        case BINARY_OPERATOR_SUB:
        case BINARY_OPERATOR_MUL:
        case BINARY_OPERATOR_DIV:
        case BINARY_OPERATOR_MOD:
            return _widenNumeric(left, right);
        case BINARY_OPERATOR_LESS:
        case BINARY_OPERATOR_GREATER:
        case BINARY_OPERATOR_LESS_EQUAL:
        case BINARY_OPERATOR_GREATER_EQUAL:
            if (_numericRank(left->kind) == NUMERIC_RANK_NONE ||
                _numericRank(right->kind) == NUMERIC_RANK_NONE) {
                return &_typeErrorSentinel;
            }
            return &_typeBoolSentinel;
        case BINARY_OPERATOR_EQUAL:
        case BINARY_OPERATOR_NOT_EQUAL:
            if (!_areComparable(table, left, right)) {
                return &_typeErrorSentinel;
            }
            return &_typeBoolSentinel;
        case BINARY_OPERATOR_AND:
        case BINARY_OPERATOR_OR:
            if (left->kind != TYPEKIND_BOOL || right->kind != TYPEKIND_BOOL) {
                return &_typeErrorSentinel;
            }
            return &_typeBoolSentinel;
        case BINARY_OPERATOR_ASSIGN:
            if (!isAssignable(table, right, left)) {
                return &_typeErrorSentinel;
            }
            return left;
        case BINARY_OPERATOR_PLUS_ASSIGN:
        case BINARY_OPERATOR_MINUS_ASSIGN:
        case BINARY_OPERATOR_MUL_ASSIGN:
        case BINARY_OPERATOR_DIV_ASSIGN:
        case BINARY_OPERATOR_MOD_ASSIGN:
            if (_numericRank(left->kind) == NUMERIC_RANK_NONE ||
                _numericRank(right->kind) == NUMERIC_RANK_NONE) {
                return &_typeErrorSentinel;
            }
            return left;
        default:
            return &_typeErrorSentinel;
    }
}

static Type * _resolveFieldAccessType(SymbolTable * table, Expression * expr) {
    Type * objectType = resolveExpressionType(table, expr->fieldAccess.object);
    if (objectType == NULL || isTypeError(objectType) || objectType->kind != TYPEKIND_CLASS) {
        return &_typeErrorSentinel;
    }
    FieldInfo * info = lookupField(table, objectType->className, expr->fieldAccess.field);
    if (info == NULL) {
        return &_typeErrorSentinel;
    }
    Visibility visibility = info->field->visibility;
    Type * fieldType = info->field->type;
    const char * ownerName = info->ownerClassName;
    free(info);
    if (!_isAccessible(table, visibility, ownerName, getCurrentClass(table))) {
        return &_typeErrorSentinel;
    }
    int depth = 0;
    for (const char * className = objectType->className; className != NULL && strcmp(className, ownerName) != 0; ) {
        Class * class = lookupClass(table, className);
        if (class == NULL) {
            break;
        }
        className = class->parentName;
        depth++;
    }
    expr->resolvedFieldDepth = depth;
    return fieldType;
}

static bool _collectArgTypes(SymbolTable * table, ArgumentList * args, Type *** outTypes, int * outCount) {
    int argCount = 0;
    for (ArgumentList * a = args; a != NULL; a = a->next) {
        argCount++;
    }
    *outCount = argCount;
    *outTypes = NULL;
    if (argCount == 0) {
        return true;
    }
    Type ** argTypes = malloc(argCount * sizeof(Type *));
    if (argTypes == NULL) {
        return false;
    }
    int i = 0;
    for (ArgumentList * a = args; a != NULL; a = a->next, i++) {
        argTypes[i] = resolveExpressionType(table, a->expression);
        if (isTypeError(argTypes[i])) {
            free(argTypes);
            return false;
        }
    }
    *outTypes = argTypes;
    return true;
}

static Type * _resolveNewType(SymbolTable * table, Expression * expr) {
    Type * type = lookupClassType(table, expr->newExpression.className);
    if (type == NULL) {
        return &_typeErrorSentinel;
    }
    Type ** argTypes = NULL;
    int argCount = 0;
    if (!_collectArgTypes(table, expr->newExpression.arguments, &argTypes, &argCount)) {
        return &_typeErrorSentinel;
    }
    Class * class = lookupClass(table, expr->newExpression.className);
    bool hasConstructor = false;
    for (Method * method = class->methods; method != NULL; method = method->next) {
        if (method->isConstructor) {
            hasConstructor = true;
            break;
        }
    }
    if (!hasConstructor) {
        free(argTypes);
        return argCount == 0 ? type : &_typeErrorSentinel;
    }
    MethodInfo * info = lookupMethod(table, expr->newExpression.className,
                                     expr->newExpression.className, argTypes, argCount);
    free(argTypes);
    if (info == NULL) {
        return &_typeErrorSentinel;
    }
    free(info);
    return type;
}

static Type * _resolveCallType(SymbolTable * table, Expression * expr) {
    Expression * callee = expr->call.callee;

    Type ** argTypes = NULL;
    int argCount = 0;
    if (!_collectArgTypes(table, expr->call.arguments, &argTypes, &argCount)) {
        return &_typeErrorSentinel;
    }

    if (callee->kind == EXPRESSION_FIELD_ACCESS || callee->kind == EXPRESSION_ARROW_ACCESS) {
        Type * objectType = resolveExpressionType(table, callee->fieldAccess.object);
        if (objectType == NULL || isTypeError(objectType) || objectType->kind != TYPEKIND_CLASS) {
            free(argTypes);
            return &_typeErrorSentinel;
        }
        MethodInfo * info = lookupMethod(table, objectType->className, callee->fieldAccess.field, argTypes, argCount);
        free(argTypes);
        if (info == NULL) {
            return &_typeErrorSentinel;
        }
        Visibility visibility = info->method->visibility;
        Type * returnType = info->method->returnType;
        const char * ownerName = info->ownerClassName;
        if (!_isAccessible(table, visibility, ownerName, getCurrentClass(table))) {
            free(info);
            return &_typeErrorSentinel;
        }
        /* Annotate the call node for the code generator. */
        expr->resolvedMethod = info->method;
        expr->resolvedOwnerClass = info->ownerClassName;
        free(info);
        return returnType != NULL ? returnType : &_typeVoidSentinel;
    }
    if (callee->kind == EXPRESSION_IDENTIFIER) {
        Function * function = lookupFunction(table, callee->identifier);
        if (function == NULL) {
            free(argTypes);
            return &_typeErrorSentinel;
        }
        if (_paramCount(function->parameters) != argCount) {
            free(argTypes);
            return &_typeErrorSentinel;
        }
        if (!_argsMatchParams(table, argTypes, argCount, function->parameters)) {
            free(argTypes);
            return &_typeErrorSentinel;
        }
        free(argTypes);
        return function->returnType != NULL ? function->returnType : &_typeVoidSentinel;
    }
    free(argTypes);
    return NULL;
}

bool isAssignable(SymbolTable * table, Type * from, Type * to) {
    if (from == NULL || to == NULL) {
        return false;
    }
    if (isTypeError(from) || isTypeError(to)) {
        return false;
    }
    NumericRank fromRank = _numericRank(from->kind);
    NumericRank toRank   = _numericRank(to->kind);
    if (fromRank != NUMERIC_RANK_NONE && toRank != NUMERIC_RANK_NONE) {
        return fromRank <= toRank;
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

Type * resolveExpressionType(SymbolTable * table, Expression * expr) {
    if (expr == NULL) {
        return &_typeErrorSentinel;
    }
    switch (expr->kind) {
        case EXPRESSION_INTEGER:
            return &_typeIntSentinel;
        case EXPRESSION_FLOAT:
            return &_typeFloatSentinel;
        case EXPRESSION_STRING:
            return &_typeStringSentinel;
        case EXPRESSION_CHAR:
            return &_typeCharSentinel;
        case EXPRESSION_BOOLEAN:
            return &_typeBoolSentinel;
        case EXPRESSION_THIS: {
            const char * className = getCurrentClass(table);
            if (className == NULL) {
                return &_typeErrorSentinel;
            }
            Type * type = lookupClassType(table, className);
            return type != NULL ? type : &_typeErrorSentinel;
        }
        case EXPRESSION_IDENTIFIER: {
            Type * type = lookupVariable(table, expr->identifier);
            if (type != NULL) {
                return type;
            }
            Type * classType = lookupClassType(table, expr->identifier);
            return classType != NULL ? classType : &_typeErrorSentinel;
        }
        case EXPRESSION_NEW:
            return _resolveNewType(table, expr);
        case EXPRESSION_BINARY:
            return _resolveBinaryType(table, expr);
        case EXPRESSION_UNARY:
            return _resolveUnaryType(table, expr);
        case EXPRESSION_FIELD_ACCESS:
        case EXPRESSION_ARROW_ACCESS:
            return _resolveFieldAccessType(table, expr);
        case EXPRESSION_CALL:
            return _resolveCallType(table, expr);
        default:
            return NULL;
    }
}