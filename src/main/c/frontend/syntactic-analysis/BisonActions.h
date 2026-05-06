#ifndef BISON_ACTIONS_HEADER
#define BISON_ACTIONS_HEADER

#include "../../support/logging/Logger.h"
#include "../../support/type/CompilerState.h"
#include "../../support/type/ModuleDestructor.h"
#include "../../support/type/TokenLabel.h"
#include "AbstractSyntaxTree.h"
#include "BisonParser.h"
#include <stdlib.h>

/** Initialize module's internal state. */
ModuleDestructor initializeBisonActionsModule();

/**
 * Bison semantic actions.
 */

Constant * IntegerConstantSemanticAction(const int value);
Expression * ArithmeticExpressionSemanticAction(Expression * leftExpression, Expression * rightExpression, ExpressionType type);
Expression * FactorExpressionSemanticAction(Factor * factor);
Factor * ConstantFactorSemanticAction(Constant * constant);
Factor * ExpressionFactorSemanticAction(Expression * expression);
Program * ExpressionProgramSemanticAction(Expression * expression);

DslProgram * ProgramSemanticAction(DslProgram * program);
DslProgram * EmptyDeclarationListSemanticAction();
DslProgram * MainDeclarationSemanticAction(DslProgram * program, StatementList * mainBody);

Type * BaseTypeSemanticAction(TypeKind kind);
Type * TypeModifierSemanticAction(char isSigned, char isUnsigned, char isShort, char isLong);
Type * MergeTypeModifiersSemanticAction(Type * list, Type * modifier);
Type * ModifiedTypeSemanticAction(Type * modifiers, Type * base);
Type * PointerTypeSemanticAction(Type * inner);
Type * ArrayTypeSemanticAction(Type * inner, int size);
Type * ArrayTypeNoSizeSemanticAction(Type * inner);
Type * ClassTypeSemanticAction(char * name);

Class * ClassSemanticAction(char * name, char * parentName, MemberList * members);
DslProgram * AddClassSemanticAction(DslProgram * program, Class * classNode);

Field * FieldSemanticAction(Visibility visibility, char isStatic, Type * type, char * name);
MemberList * AppendFieldSemanticAction(MemberList * list, Field * field);

#endif
