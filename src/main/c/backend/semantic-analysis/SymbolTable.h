#ifndef SYMBOL_TABLE_HEADER
#define SYMBOL_TABLE_HEADER

#include "../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include <stdbool.h>

typedef struct SymbolTable SymbolTable;

typedef struct {
    Field * field;
    char * ownerClassName;
} FieldInfo;

typedef struct {
    Method * method;
    char * ownerClassName;
} MethodInfo;

/* ---- Lifecycle ---- */
SymbolTable * createSymbolTable();
void destroySymbolTable(SymbolTable * table);

/* ---- Registration ---- */
void registerClass(SymbolTable * table, Class * classNode);
void registerFunction(SymbolTable * table, Function * function);

/* ---- Lookup ---- */
Class * lookupClass(SymbolTable * table, const char * name);
Function * lookupFunction(SymbolTable * table, const char * name);
FieldInfo * lookupField(SymbolTable * table, const char * className, const char * fieldName);
MethodInfo * lookupMethod(SymbolTable * table, const char * className, const char * methodName, Type ** argTypes, int argCount);

/* ---- Scope stack ---- */
void pushScope(SymbolTable * table);
void popScope(SymbolTable * table);

/** Returns false if the name already exists in any active scope (shadowing forbidden). */
bool declareVariable(SymbolTable * table, const char * name, Type * type);
Type * lookupVariable(SymbolTable * table, const char * name);

/* ---- Type checker ---- */
Type * typeOf(SymbolTable * table, Expression * expr);
bool isAssignable(SymbolTable * table, Type * from, Type * to);
Type * typeError();
bool isTypeError(Type * type);

/* ---- Context ---- */
void setCurrentClass(SymbolTable * table, const char * className);
const char * getCurrentClass(SymbolTable * table);
void setCurrentFunction(SymbolTable * table, const char * functionName);
const char * getCurrentFunction(SymbolTable * table);

#endif
