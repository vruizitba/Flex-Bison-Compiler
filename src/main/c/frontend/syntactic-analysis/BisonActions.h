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

Program * ProgramSemanticAction(Program * program);
Program * EmptyDeclarationListSemanticAction();
Program * MainDeclarationSemanticAction(Program * program, StatementList * mainBody);

Type * BaseTypeSemanticAction(TypeKind kind);
Type * TypeModifierSemanticAction(char isSigned, char isUnsigned, char isShort, char isLong);
Type * MergeTypeModifiersSemanticAction(Type * list, Type * modifier);
Type * ModifiedTypeSemanticAction(Type * modifiers, Type * base);
Type * PointerTypeSemanticAction(Type * inner);
Type * ArrayTypeSemanticAction(Type * inner, int size);
Type * ArrayTypeNoSizeSemanticAction(Type * inner);
Type * BuildClassTypeSemanticAction(char * className, Type * typeTail);

Class * ClassSemanticAction(char * name, char * parentName, MemberList * members);
Program * AddClassSemanticAction(Program * program, Class * classNode);

Parameter * ParameterSemanticAction(Type * type, char * name);
Parameter * AppendParameterSemanticAction(Parameter * list, Parameter * param);

MemberSuffix * FieldTailSemanticAction();
MemberSuffix * MethodTailSemanticAction(Parameter * parameters, StatementList * body);
MemberSuffix * ConstructorSuffixSemanticAction(Parameter * parameters, StatementList * body);
MemberSuffix * ClassMemberSuffixSemanticAction(Type * typeTail, char * memberName, MemberSuffix * tail);

MemberList * BuildMemberSemanticAction(char isStatic, Type * type, char * name, MemberSuffix * tail);
MemberList * BuildMemberFromIdentifierSemanticAction(char * outerIdentifier, MemberSuffix * suffix);
MemberList * SetMemberVisibilitySemanticAction(MemberList * list, Visibility visibility);
MemberList * MergeMemberListsSemanticAction(MemberList * dst, MemberList * src);

Function * FunctionSemanticAction(Type * returnType, char * name, Parameter * parameters, StatementList * body);
Program * AddFunctionSemanticAction(Program * program, Function * function);

Expression * IntegerExpressionSemanticAction(int value);
Expression * FloatExpressionSemanticAction(double value);
Expression * StringExpressionSemanticAction(char * value);
Expression * CharExpressionSemanticAction(char value);
Expression * BooleanExpressionSemanticAction(int value);
Expression * IdentifierExpressionSemanticAction(char * name);
Expression * ThisExpressionSemanticAction();
Expression * NewExpressionSemanticAction(char * className, ArgumentList * arguments);

ArgumentList * ArgumentListSemanticAction(Expression * expr);
ArgumentList * AppendArgumentSemanticAction(ArgumentList * list, Expression * expr);

Expression * UnaryExpressionSemanticAction(UnaryOperator operator, Expression * operand);
Expression * BinaryExpressionSemanticAction(Expression * left, BinaryOperator operator, Expression * right);

Expression * FieldAccessExpressionSemanticAction(Expression * object, char * field);
Expression * ArrowAccessExpressionSemanticAction(Expression * object, char * field);
Expression * IndexExpressionSemanticAction(Expression * array, Expression * index);
Expression * CallExpressionSemanticAction(Expression * callee, ArgumentList * arguments);
Expression * PostfixIncrementExpressionSemanticAction(Expression * operand);
Expression * PostfixDecrementExpressionSemanticAction(Expression * operand);

StatementList * AppendStatementSemanticAction(StatementList * list, Statement * stmt);
Statement * VariableDeclarationSemanticAction(Type * type, char * name, Expression * initializer);
Statement * IfStatementSemanticAction(Expression * condition, Statement * thenBranch, Statement * elseBranch);
Statement * WhileStatementSemanticAction(Expression * condition, Statement * body);
Statement * ForStatementSemanticAction(Statement * initializer, Expression * condition, Expression * step, Statement * body);
Statement * ReturnStatementSemanticAction(Expression * value);
Statement * ExpressionStatementSemanticAction(Expression * expr);
Statement * BlockStatementSemanticAction(StatementList * body);

#endif
