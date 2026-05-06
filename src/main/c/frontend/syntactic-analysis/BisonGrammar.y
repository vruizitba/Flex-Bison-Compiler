%{

#include "../../support/type/TokenLabel.h"
#include "AbstractSyntaxTree.h"
#include "BisonActions.h"

/**
 * The error reporting function for Bison parser.
 *
 * @todo Add location to the grammar and "pushToken" API function.
 *
 * @see https://www.gnu.org/software/bison/manual/html_node/Error-Reporting-Function.html
 * @see https://www.gnu.org/software/bison/manual/html_node/Tracking-Locations.html
 */
void yyerror(const YYLTYPE * location, const char * message) {}

%}

// You touch this, and you die.
%define api.pure full
%define api.push-pull push
%define api.value.union.name SemanticValue
%define parse.error detailed
%locations

%union {
	/** Terminals. */

	char character;
	signed int integer;
	double real;
	char * string;

	/** Non-terminals (old calculator kept until grammar rules are replaced). */

	Constant * constant;
	Expression * expression;
	Factor * factor;
	Program * program;

	/** Non-terminals (DSL). */

	DslProgram * dslProgram;
	Class * classNode;
	MemberList * memberList;
	Field * field;
	Method * method;
	Parameter * parameter;
	DslStatement * statement;
	StatementList * statementList;
	DslExpression * dslExpression;
	ArgumentList * argumentList;
	Type * type;
	Function * function;
}

/**
 * Destructors. This functions are executed after the parsing ends, so if the
 * AST must be used in the following phases of the compiler you shouldn't used
 * this approach for the AST root node ("program" non-terminal, in this
 * grammar), or it will drop the entire tree even if the parsing succeeds.
 *
 * @see https://www.gnu.org/software/bison/manual/html_node/Destructor-Decl.html
 */
%destructor { destroyConstant($$); } <constant>
%destructor { destroyExpression($$); } <expression>
%destructor { destroyFactor($$); } <factor>

%destructor { destroyDslExpression($$); } <dslExpression>
%destructor { destroyDslStatement($$); } <statement>
%destructor { destroyStatementList($$); } <statementList>
%destructor { destroyArgumentList($$); } <argumentList>
%destructor { destroyType($$); } <type>
%destructor { destroyField($$); } <field>
%destructor { destroyMethod($$); } <method>
%destructor { destroyClass($$); } <classNode>
%destructor { destroyMemberList($$); } <memberList>
%destructor { destroyFunction($$); } <function>
%destructor { destroyParameter($$); } <parameter>

%destructor { free($$); } <string>

/** Terminals. */
%token <character> CHAR
%token <string> IDENTIFIER
%token <string> STRING
%token <integer> INTEGER
%token <real> REAL

%token TYPE_INT TYPE_BOOL TYPE_STRING TYPE_CHAR TYPE_FLOAT TYPE_DOUBLE TYPE_VOID
%token SIGNED UNSIGNED SHORT LONG
%token BOOLEAN_TRUE BOOLEAN_FALSE
%token CLASS EXTENDS PUBLIC PRIVATE PROTECTED STATIC THIS NEW
%token IF ELSE WHILE FOR RETURN
%token MAIN FUNCTION
%token ADD SUBTRACT ASTERISK DIVIDE MODULO
%token INCREMENT DECREMENT
%token PLUS_ASSIGN MINUS_ASSIGN ASTERISK_ASSIGN DIVIDE_ASSIGN MODULO_ASSIGN
%token ASSIGN
%token EQUAL NOT_EQUAL LESS GREATER LESS_EQUAL GREATER_EQUAL
%token AND OR NOT
%token DOT ARROW AMPERSAND
%token OPEN_PARENTHESIS CLOSE_PARENTHESIS
%token OPEN_BRACE CLOSE_BRACE	
%token OPEN_BRACKET CLOSE_BRACKET
%token COMMA SEMICOLON
%token CLOSE_COMMENT
%token OPEN_COMMENT

%token IGNORED
%token UNKNOWN


/** Non-terminals (old calculator kept until grammar rules are replaced). */
%type <constant> constant
%type <expression> expression
%type <factor> factor

/** Root non-terminal. */
%type <dslProgram> program

/** Non-terminals (DSL). */
%type <dslProgram> declarationList
%type <classNode> classDeclaration
%type <function> functionDeclaration
%type <statementList> mainDeclaration

%type <type> type baseType typeModifierList typeModifier

%type <string> extendsOptional
%type <memberList> memberList member
%type <field> fieldDeclaration
%type <integer> visibility staticOptional
%type <dslExpression> initializerOptional
%type <method> methodDeclaration constructorDeclaration
%type <parameter> parameterList parameter

%type <statementList> block statementList
%type <statement> statement variableDeclaration expressionStatement
%type <statement> returnStatement ifStatement whileStatement forStatement forInitializer
%type <dslExpression> expressionOptional

%type <dslExpression> assignmentExpression logicalOrExpression logicalAndExpression
%type <dslExpression> equalityExpression relationalExpression additiveExpression
%type <dslExpression> multiplicativeExpression unaryExpression postfixExpression primaryExpression
%type <argumentList> argumentList

/**
 * Precedence and associativity.
 *
 * @see https://en.cppreference.com/w/cpp/language/operator_precedence.html
 * @see https://www.gnu.org/software/bison/manual/html_node/Precedence.html
 */

%nonassoc ELSE
%right ASSIGN PLUS_ASSIGN MINUS_ASSIGN ASTERISK_ASSIGN DIVIDE_ASSIGN MODULO_ASSIGN
%left OR
%left AND
%left EQUAL NOT_EQUAL
%left LESS GREATER LESS_EQUAL GREATER_EQUAL
%left ADD SUBTRACT
%left ASTERISK DIVIDE MODULO
%right NOT
%left DOT ARROW OPEN_BRACKET OPEN_PARENTHESIS

%%

// IMPORTANT: To use λ in the following grammar, use the %empty symbol.

program: declarationList								{ $$ = ProgramSemanticAction($1); }
	;

declarationList: %empty									{ $$ = EmptyDeclarationListSemanticAction(); }
	| declarationList mainDeclaration					{ $$ = MainDeclarationSemanticAction($1, $2); }
	| declarationList classDeclaration					{ $$ = AddClassSemanticAction($1, $2); }
	;

mainDeclaration: MAIN OPEN_BRACE statementList CLOSE_BRACE	{ $$ = $3; }
	;

statementList: %empty									{ $$ = NULL; }
	;

classDeclaration: CLASS IDENTIFIER extendsOptional OPEN_BRACE memberList CLOSE_BRACE
		{ $$ = ClassSemanticAction($2, $3, $5); }
	;

extendsOptional: %empty									{ $$ = NULL; }
	| EXTENDS IDENTIFIER								{ $$ = $2; }
	;

memberList: %empty										{ $$ = NULL; }
	;

type: baseType											{ $$ = $1; }
	| typeModifierList baseType							{ $$ = ModifiedTypeSemanticAction($1, $2); }
	| type ASTERISK										{ $$ = PointerTypeSemanticAction($1); }
	| type OPEN_BRACKET INTEGER CLOSE_BRACKET			{ $$ = ArrayTypeSemanticAction($1, $3); }
	| type OPEN_BRACKET CLOSE_BRACKET					{ $$ = ArrayTypeNoSizeSemanticAction($1); }
	| IDENTIFIER										{ $$ = ClassTypeSemanticAction($1); }
	;

typeModifierList: typeModifier							{ $$ = $1; }
	| typeModifierList typeModifier						{ $$ = MergeTypeModifiersSemanticAction($1, $2); }
	;

typeModifier: SIGNED									{ $$ = TypeModifierSemanticAction(1, 0, 0, 0); }
	| UNSIGNED											{ $$ = TypeModifierSemanticAction(0, 1, 0, 0); }
	| SHORT												{ $$ = TypeModifierSemanticAction(0, 0, 1, 0); }
	| LONG												{ $$ = TypeModifierSemanticAction(0, 0, 0, 1); }
	;

baseType: TYPE_INT										{ $$ = BaseTypeSemanticAction(TYPEKIND_INT); }
	| TYPE_BOOL											{ $$ = BaseTypeSemanticAction(TYPEKIND_BOOL); }
	| TYPE_STRING										{ $$ = BaseTypeSemanticAction(TYPEKIND_STRING); }
	| TYPE_CHAR											{ $$ = BaseTypeSemanticAction(TYPEKIND_CHAR); }
	| TYPE_FLOAT										{ $$ = BaseTypeSemanticAction(TYPEKIND_FLOAT); }
	| TYPE_DOUBLE										{ $$ = BaseTypeSemanticAction(TYPEKIND_DOUBLE); }
	| TYPE_VOID											{ $$ = BaseTypeSemanticAction(TYPEKIND_VOID); }
	;

%%
