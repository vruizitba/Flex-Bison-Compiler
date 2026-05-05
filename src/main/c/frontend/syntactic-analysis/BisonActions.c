#include "BisonActions.h"

/* MODULE INTERNAL STATE */

static CompilerState * _compilerState = NULL;
static Logger * _logger = NULL;

/** Shutdown module's internal state. */
void _shutdownBisonActionsModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: BisonActions...");
		destroyLogger(_logger);
		_logger = NULL;
	}
	_compilerState = NULL;
}

ModuleDestructor initializeBisonActionsModule(CompilerState * compilerState) {
	_compilerState = compilerState;
	_logger = createLogger("BisonActions");
	return _shutdownBisonActionsModule;
}

/* IMPORTED FUNCTIONS */

/* PRIVATE FUNCTIONS */

static void _logSyntacticAnalyzerAction(const char * functionName);

/**
 * Logs a syntactic-analyzer action in DEBUGGING level.
 */
static void _logSyntacticAnalyzerAction(const char * functionName) {
	logDebugging(_logger, "%s", functionName);
}

/* PUBLIC FUNCTIONS */

Constant * IntegerConstantSemanticAction(const int value) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Constant * constant = calloc(1, sizeof(Constant));
	constant->value = value;
	return constant;
}

Expression * ArithmeticExpressionSemanticAction(Expression * leftExpression, Expression * rightExpression, ExpressionType type) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Expression * expression = calloc(1, sizeof(Expression));
	expression->leftExpression = leftExpression;
	expression->rightExpression = rightExpression;
	expression->type = type;
	return expression;
}

Expression * FactorExpressionSemanticAction(Factor * factor) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Expression * expression = calloc(1, sizeof(Expression));
	expression->factor = factor;
	expression->type = FACTOR;
	return expression;
}

Factor * ConstantFactorSemanticAction(Constant * constant) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Factor * factor = calloc(1, sizeof(Factor));
	factor->constant = constant;
	factor->type = CONSTANT;
	return factor;
}

Factor * ExpressionFactorSemanticAction(Expression * expression) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Factor * factor = calloc(1, sizeof(Factor));
	factor->expression = expression;
	factor->type = EXPRESSION;
	return factor;
}

Program * ExpressionProgramSemanticAction(Expression * expression) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Program * program = calloc(1, sizeof(Program));
	program->expression = expression;
	_compilerState->abstractSyntaxtTree = program;
	return program;
}

DslProgram * ProgramSemanticAction(DslProgram * program) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	_compilerState->abstractSyntaxtTree = program;
	return program;
}

DslProgram * EmptyDeclarationListSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	DslProgram * program = calloc(1, sizeof(DslProgram));
	return program;
}

DslProgram * MainDeclarationSemanticAction(DslProgram * program, StatementList * mainBody) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	program->mainBody = mainBody;
	return program;
}

Type * BaseTypeSemanticAction(TypeKind kind) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Type * type = calloc(1, sizeof(Type));
	type->kind = kind;
	return type;
}

Type * TypeModifierSemanticAction(char isSigned, char isUnsigned, char isShort, char isLong) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Type * type = calloc(1, sizeof(Type));
	type->isSigned = isSigned;
	type->isUnsigned = isUnsigned;
	type->isShort = isShort;
	type->isLong = isLong;
	return type;
}

Type * MergeTypeModifiersSemanticAction(Type * list, Type * modifier) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	list->isSigned |= modifier->isSigned;
	list->isUnsigned |= modifier->isUnsigned;
	list->isShort |= modifier->isShort;
	list->isLong |= modifier->isLong;
	free(modifier);
	return list;
}

Type * ModifiedTypeSemanticAction(Type * modifiers, Type * base) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	base->isSigned = modifiers->isSigned;
	base->isUnsigned = modifiers->isUnsigned;
	base->isShort = modifiers->isShort;
	base->isLong = modifiers->isLong;
	free(modifiers);
	return base;
}

Type * PointerTypeSemanticAction(Type * inner) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Type * type = calloc(1, sizeof(Type));
	type->kind = TYPEKIND_POINTER;
	type->inner = inner;
	return type;
}

Type * ArrayTypeSemanticAction(Type * inner, int size) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Type * type = calloc(1, sizeof(Type));
	type->kind = TYPEKIND_ARRAY;
	type->inner = inner;
	type->arraySize = size;
	return type;
}

Type * ArrayTypeNoSizeSemanticAction(Type * inner) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Type * type = calloc(1, sizeof(Type));
	type->kind = TYPEKIND_ARRAY;
	type->inner = inner;
	type->arraySize = -1;
	return type;
}
	
Type * ClassTypeSemanticAction(char * name) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Type * type = calloc(1, sizeof(Type));
	type->kind = TYPEKIND_CLASS;
	type->className = name;
	return type;
}
