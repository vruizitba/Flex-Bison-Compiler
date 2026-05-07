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
	_compilerState->abstractSyntaxtTree = program;
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
	
static Type * _makeClassType(char * name) {
	Type * type = calloc(1, sizeof(Type));
	type->kind = TYPEKIND_CLASS;
	type->className = name;
	return type;
}

Class * ClassSemanticAction(char * name, char * parentName, MemberList * members) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Class * classNode = calloc(1, sizeof(Class));
	classNode->name = name;
	classNode->parentName = parentName;
	if (members != NULL) {
		classNode->fields = members->fields;
		classNode->methods = members->methods;
		free(members);
	}
	return classNode;
}

static Type * _buildClassType(char * className, Type * typeTail) {
	Type * base = _makeClassType(className);
	if (typeTail == NULL) {
		return base;
	}
	Type * current = typeTail;
	while (current->inner != NULL) {
		current = current->inner;
	}
	current->inner = base;
	return typeTail;
}

Type * BuildClassTypeSemanticAction(char * className, Type * typeTail) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return _buildClassType(className, typeTail);
}

static Field * _buildField(Visibility visibility, char isStatic, Type * type, char * name) {
	Field * field = calloc(1, sizeof(Field));
	field->visibility = visibility;
	field->isStatic = isStatic;
	field->type = type;
	field->name = name;
	return field;
}

DslProgram * AddClassSemanticAction(DslProgram * program, Class * classNode) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	classNode->next = program->classes;
	program->classes = classNode;
	return program;
}

Parameter * ParameterSemanticAction(Type * type, char * name) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Parameter * param = calloc(1, sizeof(Parameter));
	param->type = type;
	param->name = name;
	return param;
}

Parameter * AppendParameterSemanticAction(Parameter * list, Parameter * param) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Parameter * current = list;
	while (current->next != NULL) {
		current = current->next;
	}
	current->next = param;
	return list;
}

static Method * _buildMethod(Visibility visibility, char isStatic, Type * returnType, char * name, Parameter * parameters, StatementList * body) {
	Method * method = calloc(1, sizeof(Method));
	method->visibility = visibility;
	method->isStatic = isStatic;
	method->isConstructor = 0;
	method->returnType = returnType;
	method->name = name;
	method->parameters = parameters;
	method->body = body;
	return method;
}

static Method * _buildConstructor(Visibility visibility, char * name, Parameter * parameters, StatementList * body) {
	Method * method = calloc(1, sizeof(Method));
	method->visibility = visibility;
	method->isStatic = 0;
	method->isConstructor = 1;
	method->returnType = NULL;
	method->name = name;
	method->parameters = parameters;
	method->body = body;
	return method;
}

MemberSuffix * FieldTailSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return calloc(1, sizeof(MemberSuffix));
}

MemberSuffix * MethodTailSemanticAction(Parameter * parameters, StatementList * body) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	MemberSuffix * suffix = calloc(1, sizeof(MemberSuffix));
	suffix->isMethod = 1;
	suffix->parameters = parameters;
	suffix->body = body;
	return suffix;
}

MemberSuffix * ConstructorSuffixSemanticAction(Parameter * parameters, StatementList * body) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	MemberSuffix * suffix = calloc(1, sizeof(MemberSuffix));
	suffix->isConstructor = 1;
	suffix->parameters = parameters;
	suffix->body = body;
	return suffix;
}

MemberSuffix * ClassMemberSuffixSemanticAction(Type * typeTail, char * memberName, MemberSuffix * tail) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	tail->typeTail = typeTail;
	tail->memberName = memberName;
	return tail;
}

MemberList * BuildMemberSemanticAction(char isStatic, Type * type, char * name, MemberSuffix * tail) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	MemberList * list = calloc(1, sizeof(MemberList));
	if (tail->isMethod) {
		list->methods = _buildMethod(VISIBILITY_PUBLIC, isStatic, type, name, tail->parameters, tail->body);
	} else {
		list->fields = _buildField(VISIBILITY_PUBLIC, isStatic, type, name);
	}
	free(tail);
	return list;
}

MemberList * BuildMemberFromIdentifierSemanticAction(char * outerIdentifier, MemberSuffix * suffix) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	MemberList * list = calloc(1, sizeof(MemberList));
	if (suffix->isConstructor) {
		list->methods = _buildConstructor(VISIBILITY_PUBLIC, outerIdentifier, suffix->parameters, suffix->body);
	} else {
		Type * fullType = _buildClassType(outerIdentifier, suffix->typeTail);
		if (suffix->isMethod) {
			list->methods = _buildMethod(VISIBILITY_PUBLIC, 0, fullType, suffix->memberName, suffix->parameters, suffix->body);
		} else {
			list->fields = _buildField(VISIBILITY_PUBLIC, 0, fullType, suffix->memberName);
		}
	}
	free(suffix);
	return list;
}

MemberList * SetMemberVisibilitySemanticAction(MemberList * list, Visibility visibility) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	if (list == NULL) {
		return NULL;
	}
	if (list->fields != NULL) {
		list->fields->visibility = visibility;
	}
	if (list->methods != NULL) {
		list->methods->visibility = visibility;
	}
	return list;
}

MemberList * MergeMemberListsSemanticAction(MemberList * dst, MemberList * src) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	if (src == NULL) {
		return dst;
	}
	if (dst == NULL) {
		return src;
	}
	if (src->fields != NULL) {
		if (dst->fields == NULL) {
			dst->fields = src->fields;
		} else {
			Field * current = dst->fields;
			while (current->next != NULL) {
				current = current->next;
			}
			current->next = src->fields;
		}
	}
	if (src->methods != NULL) {
		if (dst->methods == NULL) {
			dst->methods = src->methods;
		} else {
			Method * current = dst->methods;
			while (current->next != NULL) {
				current = current->next;
			}
			current->next = src->methods;
		}
	}
	free(src);
	return dst;
}

Function * FunctionSemanticAction(Type * returnType, char * name, Parameter * parameters, StatementList * body) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Function * func = calloc(1, sizeof(Function));
	func->returnType = returnType;
	func->name = name;
	func->parameters = parameters;
	func->body = body;
	return func;
}

DslProgram * AddFunctionSemanticAction(DslProgram * program, Function * function) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	function->next = program->functions;
	program->functions = function;
	return program;
}

DslStatement * VariableDeclarationSemanticAction(Type * type, char * name, DslExpression * initializer) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	DslStatement * stmt = calloc(1, sizeof(DslStatement));
	stmt->kind = STATEMENT_VARIABLE_DECLARATION;
	stmt->variableDeclaration.type = type;
	stmt->variableDeclaration.name = name;
	stmt->variableDeclaration.initializer = initializer;
	return stmt;
}

DslStatement * ReturnStatementSemanticAction(DslExpression * value) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	DslStatement * stmt = calloc(1, sizeof(DslStatement));
	stmt->kind = STATEMENT_RETURN;
	stmt->returnStatement.value = value;
	return stmt;
}

StatementList * AppendStatementSemanticAction(StatementList * list, DslStatement * stmt) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	StatementList * node = calloc(1, sizeof(StatementList));
	node->statement = stmt;
	if (list == NULL) {
		return node;
	}
	StatementList * current = list;
	while (current->next != NULL) {
		current = current->next;
	}
	current->next = node;
	return list;
}

DslStatement * ExpressionStatementSemanticAction(DslExpression * expr) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	DslStatement * stmt = calloc(1, sizeof(DslStatement));
	stmt->kind = STATEMENT_EXPRESSION;
	stmt->expressionStatement = expr;
	return stmt;
}

DslStatement * BlockStatementSemanticAction(StatementList * body) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	DslStatement * stmt = calloc(1, sizeof(DslStatement));
	stmt->kind = STATEMENT_BLOCK;
	stmt->block = body;
	return stmt;
}
