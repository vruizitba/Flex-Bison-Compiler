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


/* Legacy calculator types. */
#if 0

typedef enum ExpressionType ExpressionType;
typedef enum FactorType FactorType;

typedef struct Constant Constant;
typedef struct Expression Expression;
typedef struct Factor Factor;
typedef struct Program Program;

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

#endif

/**
 * 
 * DSL AST node types.
 */

typedef enum {
    VISIBILITY_PUBLIC,
    VISIBILITY_PRIVATE,
    VISIBILITY_PROTECTED
} Visibility;

typedef enum {
    TYPEKIND_INT, TYPEKIND_BOOL, TYPEKIND_STRING, TYPEKIND_CHAR,
    TYPEKIND_FLOAT, TYPEKIND_DOUBLE, TYPEKIND_VOID,
    TYPEKIND_CLASS, TYPEKIND_POINTER, TYPEKIND_ARRAY,
    TYPEKIND_ERROR
} TypeKind;

typedef enum {
    BINARY_OPERATOR_ADD, BINARY_OPERATOR_SUB, BINARY_OPERATOR_MUL, BINARY_OPERATOR_DIV, BINARY_OPERATOR_MOD,
    BINARY_OPERATOR_EQUAL, BINARY_OPERATOR_NOT_EQUAL,
    BINARY_OPERATOR_LESS, BINARY_OPERATOR_GREATER, BINARY_OPERATOR_LESS_EQUAL, BINARY_OPERATOR_GREATER_EQUAL,
    BINARY_OPERATOR_AND, BINARY_OPERATOR_OR,
    BINARY_OPERATOR_ASSIGN, BINARY_OPERATOR_PLUS_ASSIGN, BINARY_OPERATOR_MINUS_ASSIGN,
    BINARY_OPERATOR_MUL_ASSIGN, BINARY_OPERATOR_DIV_ASSIGN, BINARY_OPERATOR_MOD_ASSIGN
} BinaryOperator;

typedef enum {
    UNARY_OPERATOR_NOT, UNARY_OPERATOR_NEGATE,
    UNARY_OPERATOR_PRE_INCREMENT, UNARY_OPERATOR_PRE_DECREMENT,
    UNARY_OPERATOR_POST_INCREMENT, UNARY_OPERATOR_POST_DECREMENT,
    UNARY_OPERATOR_DEREFERENCE, UNARY_OPERATOR_ADDRESS_OF
} UnaryOperator;

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
typedef struct Expression Expression;
typedef struct Statement Statement;
typedef struct Parameter Parameter;
typedef struct StatementList StatementList;
typedef struct ArgumentList ArgumentList;
typedef struct Method Method;
typedef struct Field Field;
typedef struct Class Class;
typedef struct Function Function;
typedef struct Program Program;
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

struct Expression {
    ExpressionKind kind;
    /* Method-call annotations set by semantic analysis. Borrowed: not freed by destroyExpression. */
    Method * resolvedMethod;
    char * resolvedOwnerClass;
    union {
        int integerValue;
        double floatValue;
        char * stringValue;
        char charValue;
        int booleanValue;
        char * identifier;
        struct {
            BinaryOperator operator;
            Expression * left;
            Expression * right;
        } binary;
        struct {
            UnaryOperator operator;
            Expression * operand;
        } unary;
        struct {
            Expression * object;
            char * field;
        } fieldAccess;
        struct {
            Expression * array;
            Expression * index;
        } indexAccess;
        struct {
            Expression * callee;
            ArgumentList * arguments;
        } call;
        struct {
            char * className;
            ArgumentList * arguments;
        } newExpression;
    };
};

void destroyExpression(Expression * expression);

struct Statement {
    StatementKind kind;
    union {
        struct {
            Type * type;
            char * name;
            Expression * initializer;    // NULL if no initializer
        } variableDeclaration;
        Expression * expressionStatement;
        struct {
            Expression * condition;
            Statement * thenBranch;
            Statement * elseBranch;      // NULL if no else
        } ifStatement;
        struct {
            Expression * condition;
            Statement * body;
        } whileStatement;
        struct {
            Statement * initializer;     // NULL if empty
            Expression * condition;
            Expression * step;
            Statement * body;
        } forStatement;
        struct {
            Expression * value;          // NULL for bare return;
        } returnStatement;
        StatementList * block;
    };
};

void destroyStatement(Statement * statement);

struct StatementList {
    Statement * statement;
    StatementList * next;
};

struct ArgumentList {
    Expression * expression;
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
    Expression * initializer;    // NULL if no initializer
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

struct Program {
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
void destroyProgram(Program * program);

#endif
