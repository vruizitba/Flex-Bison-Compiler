#include "SymbolTable.h"
#include "../../support/logging/Logger.h"
#include <stdlib.h>
#include <string.h>

/* TYPE_ERROR sentinel — static, never freed. */
static Type _typeErrorSentinel = { .kind = TYPEKIND_ERROR };

struct SymbolTable {
    const char * currentClass;
    const char * currentFunction;
};

/* ---- Lifecycle ---- */
SymbolTable * createSymbolTable() {
    return calloc(1, sizeof(SymbolTable));
}

void destroySymbolTable(SymbolTable * table) {
    free(table);
}

/* ---- Registration ---- */
void registerClass(SymbolTable * table, Class * classNode) {
    (void)table;
    (void)classNode;
}

void registerFunction(SymbolTable * table, Function * function) {
    (void)table;
    (void)function;
}

/* ---- Lookup ---- */
Class * lookupClass(SymbolTable * table, const char * name) {
    (void)table;
    (void)name;
    return NULL;
}

Function * lookupFunction(SymbolTable * table, const char * name) {
    (void)table;
    (void)name;
    return NULL;
}

FieldInfo * lookupField(SymbolTable * table, const char * className, const char * fieldName) {
    (void)table;
    (void)className;
    (void)fieldName;
    return NULL;
}

MethodInfo * lookupMethod(SymbolTable * table, const char * className, const char * methodName, Type ** argTypes, int argCount) {
    (void)table;
    (void)className;
    (void)methodName;
    (void)argTypes;
    (void)argCount;
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
