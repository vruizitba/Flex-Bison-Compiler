#ifndef ABSTRACT_SYNTAX_TREE_HEADER
#define ABSTRACT_SYNTAX_TREE_HEADER

#include "../../support/logging/Logger.h"
#include "../../support/type/ModuleDestructor.h"
#include <stdlib.h>

/** Initialize module's internal state. */
ModuleDestructor initializeAbstractSyntaxTreeModule();

/**
 * This type definitions allows self-referencing types (e.g., an expression
 * that is made of another expressions, such as talking about you in 3rd
 * person, but without the madness).
 */

typedef enum ExpressionType ExpressionType;
typedef enum FactorType FactorType;

typedef struct Constant Constant;
typedef struct Expression Expression;
typedef struct Factor Factor;
typedef struct Program Program;

/**
 * Node types for the Abstract Syntax Tree (AST).
 */

enum ExpressionType {
	ADDITION,
	DIVISION,
	FACTOR,
	MULTIPLICATION,
	SUBTRACTION
};

enum FactorType {
	CONSTANT,
	EXPRESSION
};

struct Constant {
	int value;
};

struct Factor {
	union {
		Constant * constant;
		Expression * expression;
	};
	FactorType type;
};

struct Expression {
	union {
		Factor * factor;
		struct {
			Expression * leftExpression;
			Expression * rightExpression;
		};
	};
	ExpressionType type;
};

struct Program {
	Expression * expression;
};

/**
 * Node recursive super-duper-trambolik-destructors.
 */

void destroyConstant(Constant * constant);
void destroyExpression(Expression * expression);
void destroyFactor(Factor * factor);
void destroyProgram(Program * program);

/**
 * Our DSL types. For now, we won't delete the previous types.
 */

typedef enum {
    VISIBILITY_PUBLIC,
    VISIBILITY_PRIVATE,
    VISIBILITY_PROTECTED
} Visibility;

typedef enum {
    TYPEKIND_INT, TYPEKIND_BOOL, TYPEKIND_STRING, TYPEKIND_CHAR,
    TYPEKIND_FLOAT, TYPEKIND_DOUBLE, TYPEKIND_VOID,
    TYPEKIND_CLASS, TYPEKIND_POINTER, TYPEKIND_ARRAY
} TypeKind;

typedef enum {
    BINARY_OPERATOR_ADD, BINARY_OPERATOR_SUB, BINARY_OPERATOR_MUL, BINARY_OPERATOR_DIV, BINARY_OPERATOR_MOD,
    BINARY_OPERATOR_EQUAL, BINARY_OPERATOR_NOT_EQUAL,
    BINARY_OPERATOR_LESS, BINARY_OPERATOR_GREATER, BINARY_OPERATOR_LESS_EQUAL, BINARY_OPERATOR_GREATER_EQUAL,
    BINARY_OPERATOR_AND, BINARY_OPERATOR_OR,
    BINARY_OPERATOR_ASSIGN, BINARY_OPERATOR_PLUS_ASSIGN, BINARY_OPERATOR_MINUS_ASSIGN,
    BINARY_OPERATOR_MUL_ASSIGN, BINARY_OPERATOR_DIV_ASSIGN, BINARY_OPERATOR_MOD_ASSIGN
} DslBinaryOperator;

typedef enum {
    UNARY_OPERATOR_NOT, UNARY_OPERATOR_NEGATE,
    UNARY_OPERATOR_PRE_INCREMENT, UNARY_OPERATOR_PRE_DECREMENT,
    UNARY_OPERATOR_POST_INCREMENT, UNARY_OPERATOR_POST_DECREMENT,
    UNARY_OPERATOR_DEREFERENCE, UNARY_OPERATOR_ADDRESS_OF
} DslUnaryOperator;

typedef enum {
    EXPRESSION_INTEGER, EXPRESSION_FLOAT, EXPRESSION_STRING,
    EXPRESSION_CHAR, EXPRESSION_BOOLEAN,
    EXPRESSION_IDENTIFIER, EXPRESSION_THIS, EXPRESSION_NEW,
    EXPRESSION_BINARY, EXPRESSION_UNARY,
    EXPRESSION_FIELD_ACCESS, EXPRESSION_ARROW_ACCESS,
    EXPRESSION_INDEX, EXPRESSION_CALL
} ExpressionKind;

typedef enum {
    STATEMENT_VARIABLE_DECLARATION, STATEMENT_IF, STATEMENT_WHILE,
    STATEMENT_FOR, STATEMENT_RETURN, STATEMENT_EXPRESSION, STATEMENT_BLOCK
} StatementKind;

typedef struct Type Type;
typedef struct DslExpression DslExpression;
typedef struct DslStatement DslStatement;
typedef struct Parameter Parameter;
typedef struct StatementList StatementList;
typedef struct ArgumentList ArgumentList;
typedef struct Method Method;
typedef struct Field Field;
typedef struct Class Class;
typedef struct Function Function;
typedef struct DslProgram DslProgram;
typedef struct MemberList MemberList;
typedef struct MemberSuffix MemberSuffix;

struct Type {
    TypeKind kind;
    char isSigned;
    char isUnsigned;
    char isShort;
    char isLong;
    char * className;   // TYPEKIND_CLASS
    Type * inner;       // TYPEKIND_POINTER, TYPEKIND_ARRAY
    int arraySize;      // TYPEKIND_ARRAY (-1 = no size specified)
};

void destroyType(Type * type);

struct DslExpression {
    ExpressionKind kind;
    union {
        int integerValue;
        double floatValue;
        char * stringValue;
        char charValue;
        int booleanValue;
        char * identifier;
        struct {
            DslBinaryOperator operator;
            DslExpression * left;
            DslExpression * right;
        } binary;
        struct {
            DslUnaryOperator operator;
            DslExpression * operand;
        } unary;
        struct {
            DslExpression * object;
            char * field;
        } fieldAccess;
        struct {
            DslExpression * array;
            DslExpression * index;
        } indexAccess;
        struct {
            DslExpression * callee;
            ArgumentList * arguments;
        } call;
        struct {
            char * className;
            ArgumentList * arguments;
        } newExpression;
    };
};

void destroyDslExpression(DslExpression * expression);

struct DslStatement {
    StatementKind kind;
    union {
        struct {
            Type * type;
            char * name;
            DslExpression * initializer;    // NULL if no initializer
        } variableDeclaration;
        DslExpression * expressionStatement;
        struct {
            DslExpression * condition;
            DslStatement * thenBranch;
            DslStatement * elseBranch;      // NULL if no else
        } ifStatement;
        struct {
            DslExpression * condition;
            DslStatement * body;
        } whileStatement;
        struct {
            DslStatement * initializer;     // NULL if empty
            DslExpression * condition;
            DslExpression * step;
            DslStatement * body;
        } forStatement;
        struct {
            DslExpression * value;          // NULL for bare return;
        } returnStatement;
        StatementList * block;
    };
};

void destroyDslStatement(DslStatement * statement);

struct StatementList {
    DslStatement * statement;
    StatementList * next;
};

struct ArgumentList {
    DslExpression * expression;
    ArgumentList * next;
};

struct Parameter {
    Type * type;
    char * name;
    Parameter * next;
};

void destroyStatementList(StatementList * list);
void destroyArgumentList(ArgumentList * list);
void destroyParameter(Parameter * parameter);

struct Field {
    Visibility visibility;
    char isStatic;
    Type * type;
    char * name;
    DslExpression * initializer;    // NULL if no initializer
    Field * next;
};

struct Method {
    Visibility visibility;
    char isStatic;
    char isConstructor;
    Type * returnType;              // NULL if constructor
    char * name;
    Parameter * parameters;
    StatementList * body;
    Method * next;
};

struct Class {
    char * name;
    char * parentName;              // NULL if no extends
    Field * fields;
    Method * methods;
    Class * next;
};

struct Function {
    Type * returnType;
    char * name;
    Parameter * parameters;
    StatementList * body;
    Function * next;
};

struct DslProgram {
    Class * classes;
    Function * functions;
    StatementList * mainBody;
};

struct MemberList {
    Field * fields;
    Method * methods;
};

struct MemberSuffix {
    int isConstructor;
    int isMethod;
    Type * typeTail;
    char * memberName;
    Parameter * parameters;
    StatementList * body;
};

void destroyMemberList(MemberList * memberList);
void destroyMemberSuffix(MemberSuffix * suffix);
void destroyField(Field * field);
void destroyMethod(Method * method);
void destroyClass(Class * class);
void destroyFunction(Function * function);
void destroyDslProgram(DslProgram * program);

#endif
