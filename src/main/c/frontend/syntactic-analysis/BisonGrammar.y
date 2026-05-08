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

/**
 * Expected reduce/reduce conflict: classic C-style ambiguity between a
 * class type name and a variable name when a statement starts with
 * IDENTIFIER followed by ASTERISK or OPEN_BRACKET.
 *
 * Example: given the input "x * y;" the parser cannot decide between
 *   - pointer declaration: "x" is the pointer type, "y" is the variable
 *   - expression: multiplication "x * y"
 *
 * With a single token of lookahead, they cannot be distinguished.
 * Bison's default resolution: rule declared first wins
 * In this case, picks the declaration path "classTypeTail: %empty".
 *
 * Accepted as a Stage II false positive.
 * Stage III must resolve this via semantic analysis.
 */

%union {
	/** Terminals. */

	char character;
	signed int integer;
	double real;
	char * string;

	/** Non-terminals. */

	DslProgram * dslProgram;
	Class * classNode;
	MemberList * memberList;
	MemberSuffix * memberSuffix;
	Field * field;
	Method * method;
	Parameter * parameter;
	Statement * statement;
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
%destructor { destroyDslExpression($$); } <dslExpression>
%destructor { destroyStatement($$); } <statement>
%destructor { destroyStatementList($$); } <statementList>
%destructor { destroyArgumentList($$); } <argumentList>
%destructor { destroyType($$); } <type>
%destructor { destroyField($$); } <field>
%destructor { destroyMethod($$); } <method>
%destructor { destroyClass($$); } <classNode>
%destructor { destroyMemberList($$); } <memberList>
%destructor { destroyMemberSuffix($$); } <memberSuffix>
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
%token LOWER_THAN_ELSE
%token NEGATE PRE_INCREMENT PRE_DECREMENT POST_INCREMENT POST_DECREMENT DEREFERENCE ADDRESS_OF


/** Root non-terminal. */
%type <dslProgram> program

/** Non-terminals. */
%type <dslProgram> declarationList
%type <classNode> classDeclaration
%type <function> functionDeclaration
%type <statementList> mainDeclaration

%type <type> type baseType typeModifierList typeModifier

%type <string> extendsOptional
%type <memberList> memberList member memberAfterVisibility
%type <memberSuffix> memberAfterIdentifier memberTail
%type <integer> visibility
%type <dslExpression> initializerOptional
%type <parameter> parameterList nonEmptyParamList parameter
%type <type> primitiveType classTypeTail

%type <statementList> block statementList
%type <statement> statement variableDeclaration variableDeclarationBase expressionStatement
%type <statement> returnStatement ifStatement whileStatement forStatement forInitializer
%type <dslExpression> expressionOptional dslExpression

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

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE
%right ASSIGN PLUS_ASSIGN MINUS_ASSIGN ASTERISK_ASSIGN DIVIDE_ASSIGN MODULO_ASSIGN
%left OR
%left AND
%left EQUAL NOT_EQUAL
%left LESS GREATER LESS_EQUAL GREATER_EQUAL
%left ADD SUBTRACT
%left ASTERISK DIVIDE MODULO
%right NOT NEGATE PRE_INCREMENT PRE_DECREMENT DEREFERENCE ADDRESS_OF
%left DOT ARROW OPEN_BRACKET OPEN_PARENTHESIS POST_INCREMENT POST_DECREMENT

%%

// IMPORTANT: To use λ in the following grammar, use the %empty symbol.

program: declarationList								{ $$ = ProgramSemanticAction($1); }
	;

declarationList: %empty									{ $$ = EmptyDeclarationListSemanticAction(); }
	| declarationList mainDeclaration					{ $$ = MainDeclarationSemanticAction($1, $2); }
	| declarationList classDeclaration					{ $$ = AddClassSemanticAction($1, $2); }
	| declarationList functionDeclaration				{ $$ = AddFunctionSemanticAction($1, $2); }
	;

mainDeclaration: MAIN OPEN_BRACE statementList CLOSE_BRACE	{ $$ = $3; }
	;

functionDeclaration: FUNCTION type IDENTIFIER
		OPEN_PARENTHESIS parameterList CLOSE_PARENTHESIS block
		{ $$ = FunctionSemanticAction($2, $3, $5, $7); }
	;

statementList: %empty									{ $$ = NULL; }
	| statementList statement							{ $$ = AppendStatementSemanticAction($1, $2); }
	;

classDeclaration: CLASS IDENTIFIER extendsOptional OPEN_BRACE memberList CLOSE_BRACE
		{ $$ = ClassSemanticAction($2, $3, $5); }
	;

extendsOptional: %empty									{ $$ = NULL; }
	| EXTENDS IDENTIFIER								{ $$ = $2; }
	;

memberList: %empty										{ $$ = NULL; }
	| memberList member									{ $$ = MergeMemberListsSemanticAction($1, $2); }
	;

member: visibility memberAfterVisibility				{ $$ = SetMemberVisibilitySemanticAction($2, $1); }
	;

memberAfterVisibility:
	  STATIC type IDENTIFIER memberTail					{ $$ = BuildMemberSemanticAction(1, $2, $3, $4); }
	| primitiveType IDENTIFIER memberTail				{ $$ = BuildMemberSemanticAction(0, $1, $2, $3); }
	| IDENTIFIER memberAfterIdentifier					{ $$ = BuildMemberFromIdentifierSemanticAction($1, $2); }
	;

memberAfterIdentifier:
	  OPEN_PARENTHESIS parameterList CLOSE_PARENTHESIS block
		{ $$ = ConstructorSuffixSemanticAction($2, $4); }
	| classTypeTail IDENTIFIER memberTail
		{ $$ = ClassMemberSuffixSemanticAction($1, $2, $3); }
	;

memberTail:
	  SEMICOLON											{ $$ = FieldTailSemanticAction(); }
	| OPEN_PARENTHESIS parameterList CLOSE_PARENTHESIS block
		{ $$ = MethodTailSemanticAction($2, $4); }
	;

classTypeTail: %empty									{ $$ = NULL; }
	| classTypeTail ASTERISK							{ $$ = PointerTypeSemanticAction($1); }
	| classTypeTail OPEN_BRACKET INTEGER CLOSE_BRACKET	{ $$ = ArrayTypeSemanticAction($1, $3); }
	| classTypeTail OPEN_BRACKET CLOSE_BRACKET			{ $$ = ArrayTypeNoSizeSemanticAction($1); }
	;

block: OPEN_BRACE statementList CLOSE_BRACE				{ $$ = $2; }
	;

statement: variableDeclaration							{ $$ = $1; }
	| ifStatement										{ $$ = $1; }
	| whileStatement									{ $$ = $1; }
	| forStatement										{ $$ = $1; }
	| returnStatement									{ $$ = $1; }
	| expressionStatement								{ $$ = $1; }
	| block												{ $$ = BlockStatementSemanticAction($1); }
	;

variableDeclarationBase: type IDENTIFIER initializerOptional	{ $$ = VariableDeclarationSemanticAction($1, $2, $3); }
	;

variableDeclaration: variableDeclarationBase SEMICOLON	{ $$ = $1; }
	;

ifStatement: IF OPEN_PARENTHESIS dslExpression CLOSE_PARENTHESIS statement %prec LOWER_THAN_ELSE
		{ $$ = IfStatementSemanticAction($3, $5, NULL); }
	| IF OPEN_PARENTHESIS dslExpression CLOSE_PARENTHESIS statement ELSE statement
		{ $$ = IfStatementSemanticAction($3, $5, $7); }
	;

whileStatement: WHILE OPEN_PARENTHESIS dslExpression CLOSE_PARENTHESIS statement
		{ $$ = WhileStatementSemanticAction($3, $5); }
	;

forStatement: FOR OPEN_PARENTHESIS forInitializer SEMICOLON dslExpression SEMICOLON dslExpression CLOSE_PARENTHESIS statement
		{ $$ = ForStatementSemanticAction($3, $5, $7, $9); }
	;

forInitializer: variableDeclarationBase					{ $$ = $1; }
	| dslExpression										{ $$ = ExpressionStatementSemanticAction($1); }
	;

initializerOptional: %empty								{ $$ = NULL; }
	| ASSIGN dslExpression								{ $$ = $2; }
	;

returnStatement: RETURN expressionOptional SEMICOLON	{ $$ = ReturnStatementSemanticAction($2); }
	;

expressionStatement: dslExpression SEMICOLON			{ $$ = ExpressionStatementSemanticAction($1); }
	;

expressionOptional: %empty								{ $$ = NULL; }
	| dslExpression										{ $$ = $1; }
	;

dslExpression: assignmentExpression						{ $$ = $1; }
	;

assignmentExpression: logicalOrExpression				{ $$ = $1; }
	| unaryExpression ASSIGN assignmentExpression		{ $$ = BinaryExpressionSemanticAction($1, BINARY_OPERATOR_ASSIGN, $3); }
	| unaryExpression PLUS_ASSIGN assignmentExpression	{ $$ = BinaryExpressionSemanticAction($1, BINARY_OPERATOR_PLUS_ASSIGN, $3); }
	| unaryExpression MINUS_ASSIGN assignmentExpression	{ $$ = BinaryExpressionSemanticAction($1, BINARY_OPERATOR_MINUS_ASSIGN, $3); }
	| unaryExpression ASTERISK_ASSIGN assignmentExpression	{ $$ = BinaryExpressionSemanticAction($1, BINARY_OPERATOR_MUL_ASSIGN, $3); }
	| unaryExpression DIVIDE_ASSIGN assignmentExpression	{ $$ = BinaryExpressionSemanticAction($1, BINARY_OPERATOR_DIV_ASSIGN, $3); }
	| unaryExpression MODULO_ASSIGN assignmentExpression	{ $$ = BinaryExpressionSemanticAction($1, BINARY_OPERATOR_MOD_ASSIGN, $3); }
	;

logicalOrExpression: logicalAndExpression				{ $$ = $1; }
	| logicalOrExpression OR logicalAndExpression		{ $$ = BinaryExpressionSemanticAction($1, BINARY_OPERATOR_OR, $3); }
	;

logicalAndExpression: equalityExpression				{ $$ = $1; }
	| logicalAndExpression AND equalityExpression		{ $$ = BinaryExpressionSemanticAction($1, BINARY_OPERATOR_AND, $3); }
	;

equalityExpression: relationalExpression				{ $$ = $1; }
	| equalityExpression EQUAL relationalExpression		{ $$ = BinaryExpressionSemanticAction($1, BINARY_OPERATOR_EQUAL, $3); }
	| equalityExpression NOT_EQUAL relationalExpression	{ $$ = BinaryExpressionSemanticAction($1, BINARY_OPERATOR_NOT_EQUAL, $3); }
	;

relationalExpression: additiveExpression				{ $$ = $1; }
	| relationalExpression LESS additiveExpression		{ $$ = BinaryExpressionSemanticAction($1, BINARY_OPERATOR_LESS, $3); }
	| relationalExpression GREATER additiveExpression	{ $$ = BinaryExpressionSemanticAction($1, BINARY_OPERATOR_GREATER, $3); }
	| relationalExpression LESS_EQUAL additiveExpression	{ $$ = BinaryExpressionSemanticAction($1, BINARY_OPERATOR_LESS_EQUAL, $3); }
	| relationalExpression GREATER_EQUAL additiveExpression	{ $$ = BinaryExpressionSemanticAction($1, BINARY_OPERATOR_GREATER_EQUAL, $3); }
	;

additiveExpression: multiplicativeExpression			{ $$ = $1; }
	| additiveExpression ADD multiplicativeExpression	{ $$ = BinaryExpressionSemanticAction($1, BINARY_OPERATOR_ADD, $3); }
	| additiveExpression SUBTRACT multiplicativeExpression	{ $$ = BinaryExpressionSemanticAction($1, BINARY_OPERATOR_SUB, $3); }
	;


multiplicativeExpression: unaryExpression				{ $$ = $1; }
	| multiplicativeExpression ASTERISK unaryExpression	{ $$ = BinaryExpressionSemanticAction($1, BINARY_OPERATOR_MUL, $3); }
	| multiplicativeExpression DIVIDE unaryExpression	{ $$ = BinaryExpressionSemanticAction($1, BINARY_OPERATOR_DIV, $3); }
	| multiplicativeExpression MODULO unaryExpression	{ $$ = BinaryExpressionSemanticAction($1, BINARY_OPERATOR_MOD, $3); }
	;

unaryExpression: postfixExpression						{ $$ = $1; }
	| NOT unaryExpression								{ $$ = UnaryExpressionSemanticAction(UNARY_OPERATOR_NOT, $2); }
	| SUBTRACT unaryExpression %prec NEGATE				{ $$ = UnaryExpressionSemanticAction(UNARY_OPERATOR_NEGATE, $2); }
	| INCREMENT unaryExpression %prec PRE_INCREMENT		{ $$ = UnaryExpressionSemanticAction(UNARY_OPERATOR_PRE_INCREMENT, $2); }
	| DECREMENT unaryExpression %prec PRE_DECREMENT		{ $$ = UnaryExpressionSemanticAction(UNARY_OPERATOR_PRE_DECREMENT, $2); }
	| ASTERISK unaryExpression %prec DEREFERENCE		{ $$ = UnaryExpressionSemanticAction(UNARY_OPERATOR_DEREFERENCE, $2); }
	| AMPERSAND unaryExpression %prec ADDRESS_OF		{ $$ = UnaryExpressionSemanticAction(UNARY_OPERATOR_ADDRESS_OF, $2); }
	;

postfixExpression: primaryExpression					{ $$ = $1; }
	| postfixExpression DOT IDENTIFIER					{ $$ = FieldAccessExpressionSemanticAction($1, $3); }
	| postfixExpression ARROW IDENTIFIER				{ $$ = ArrowAccessExpressionSemanticAction($1, $3); }
	| postfixExpression OPEN_BRACKET dslExpression CLOSE_BRACKET
		{ $$ = IndexExpressionSemanticAction($1, $3); }
	| postfixExpression OPEN_PARENTHESIS argumentList CLOSE_PARENTHESIS
		{ $$ = CallExpressionSemanticAction($1, $3); }
	| postfixExpression INCREMENT %prec POST_INCREMENT	{ $$ = PostfixIncrementExpressionSemanticAction($1); }
	| postfixExpression DECREMENT %prec POST_DECREMENT	{ $$ = PostfixDecrementExpressionSemanticAction($1); }
	;

primaryExpression: INTEGER								{ $$ = IntegerExpressionSemanticAction($1); }
	| REAL												{ $$ = FloatExpressionSemanticAction($1); }
	| STRING											{ $$ = StringExpressionSemanticAction($1); }
	| CHAR												{ $$ = CharExpressionSemanticAction($1); }
	| BOOLEAN_TRUE										{ $$ = BooleanExpressionSemanticAction(1); }
	| BOOLEAN_FALSE										{ $$ = BooleanExpressionSemanticAction(0); }
	| IDENTIFIER										{ $$ = IdentifierExpressionSemanticAction($1); }
	| THIS												{ $$ = ThisExpressionSemanticAction(); }
	| NEW IDENTIFIER OPEN_PARENTHESIS argumentList CLOSE_PARENTHESIS
		{ $$ = NewExpressionSemanticAction($2, $4); }
	| OPEN_PARENTHESIS dslExpression CLOSE_PARENTHESIS	{ $$ = $2; }
	;

argumentList: %empty									{ $$ = NULL; }
	| dslExpression										{ $$ = ArgumentListSemanticAction($1); }
	| argumentList COMMA dslExpression					{ $$ = AppendArgumentSemanticAction($1, $3); }
	;

parameterList: %empty									{ $$ = NULL; }
	| nonEmptyParamList									{ $$ = $1; }
	;

nonEmptyParamList: parameter							{ $$ = $1; }
	| nonEmptyParamList COMMA parameter					{ $$ = AppendParameterSemanticAction($1, $3); }
	;

parameter: type IDENTIFIER								{ $$ = ParameterSemanticAction($1, $2); }
	;

visibility: PUBLIC										{ $$ = VISIBILITY_PUBLIC; }
	| PRIVATE											{ $$ = VISIBILITY_PRIVATE; }
	| PROTECTED											{ $$ = VISIBILITY_PROTECTED; }
	;

primitiveType: baseType									{ $$ = $1; }
	| typeModifierList baseType							{ $$ = ModifiedTypeSemanticAction($1, $2); }
	| primitiveType ASTERISK							{ $$ = PointerTypeSemanticAction($1); }
	| primitiveType OPEN_BRACKET INTEGER CLOSE_BRACKET	{ $$ = ArrayTypeSemanticAction($1, $3); }
	| primitiveType OPEN_BRACKET CLOSE_BRACKET			{ $$ = ArrayTypeNoSizeSemanticAction($1); }
	;

type: primitiveType										{ $$ = $1; }
	| IDENTIFIER classTypeTail							{ $$ = BuildClassTypeSemanticAction($1, $2); }
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
