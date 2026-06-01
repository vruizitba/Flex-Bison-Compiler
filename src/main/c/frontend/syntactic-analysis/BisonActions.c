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

Program * ProgramSemanticAction(Program * program) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	_compilerState->abstractSyntaxtTree = program;
	return program;
}

Program * EmptyDeclarationListSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Program * program = calloc(1, sizeof(Program));
	_compilerState->abstractSyntaxtTree = program;
	return program;
}

Program * MainDeclarationSemanticAction(Program * program, StatementList * mainBody) {
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

Program * AddClassSemanticAction(Program * program, Class * classNode) {
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

Program * AddFunctionSemanticAction(Program * program, Function * function) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	function->next = program->functions;
	program->functions = function;
	return program;
}

Expression * IntegerExpressionSemanticAction(int value) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Expression * expr = calloc(1, sizeof(Expression));
	expr->kind = EXPRESSION_INTEGER;
	expr->integerValue = value;
	return expr;
}

Expression * FloatExpressionSemanticAction(double value) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Expression * expr = calloc(1, sizeof(Expression));
	expr->kind = EXPRESSION_FLOAT;
	expr->floatValue = value;
	return expr;
}

Expression * StringExpressionSemanticAction(char * value) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Expression * expr = calloc(1, sizeof(Expression));
	expr->kind = EXPRESSION_STRING;
	expr->stringValue = value;
	return expr;
}

Expression * CharExpressionSemanticAction(char value) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Expression * expr = calloc(1, sizeof(Expression));
	expr->kind = EXPRESSION_CHAR;
	expr->charValue = value;
	return expr;
}

Expression * BooleanExpressionSemanticAction(int value) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Expression * expr = calloc(1, sizeof(Expression));
	expr->kind = EXPRESSION_BOOLEAN;
	expr->booleanValue = value;
	return expr;
}

Expression * IdentifierExpressionSemanticAction(char * name) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Expression * expr = calloc(1, sizeof(Expression));
	expr->kind = EXPRESSION_IDENTIFIER;
	expr->identifier = name;
	return expr;
}

Expression * ThisExpressionSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Expression * expr = calloc(1, sizeof(Expression));
	expr->kind = EXPRESSION_THIS;
	return expr;
}

Expression * NewExpressionSemanticAction(char * className, ArgumentList * arguments) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Expression * expr = calloc(1, sizeof(Expression));
	expr->kind = EXPRESSION_NEW;
	expr->newExpression.className = className;
	expr->newExpression.arguments = arguments;
	return expr;
}

ArgumentList * ArgumentListSemanticAction(Expression * expression) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	ArgumentList * node = calloc(1, sizeof(ArgumentList));
	node->expression = expression;
	return node;
}

ArgumentList * AppendArgumentSemanticAction(ArgumentList * list, Expression * expression) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	ArgumentList * node = calloc(1, sizeof(ArgumentList));
	node->expression = expression;
	ArgumentList * current = list;
	while (current->next != NULL) {
		current = current->next;
	}
	current->next = node;
	return list;
}

Expression * BinaryExpressionSemanticAction(Expression * left, BinaryOperator operator, Expression * right) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Expression * expr = calloc(1, sizeof(Expression));
	expr->kind = EXPRESSION_BINARY;
	expr->binary.operator = operator;
	expr->binary.left = left;
	expr->binary.right = right;
	return expr;
}

Expression * UnaryExpressionSemanticAction(UnaryOperator operator, Expression * operand) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Expression * expr = calloc(1, sizeof(Expression));
	expr->kind = EXPRESSION_UNARY;
	expr->unary.operator = operator;
	expr->unary.operand = operand;
	return expr;
}

Expression * FieldAccessExpressionSemanticAction(Expression * object, char * field) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Expression * expr = calloc(1, sizeof(Expression));
	expr->kind = EXPRESSION_FIELD_ACCESS;
	expr->fieldAccess.object = object;
	expr->fieldAccess.field = field;
	return expr;
}

Expression * ArrowAccessExpressionSemanticAction(Expression * object, char * field) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Expression * expr = calloc(1, sizeof(Expression));
	expr->kind = EXPRESSION_ARROW_ACCESS;
	expr->fieldAccess.object = object;
	expr->fieldAccess.field = field;
	return expr;
}

Expression * IndexExpressionSemanticAction(Expression * array, Expression * index) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Expression * expr = calloc(1, sizeof(Expression));
	expr->kind = EXPRESSION_INDEX;
	expr->indexAccess.array = array;
	expr->indexAccess.index = index;
	return expr;
}

Expression * CallExpressionSemanticAction(Expression * callee, ArgumentList * arguments) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Expression * expr = calloc(1, sizeof(Expression));
	expr->kind = EXPRESSION_CALL;
	expr->call.callee = callee;
	expr->call.arguments = arguments;
	return expr;
}

Expression * PostfixIncrementExpressionSemanticAction(Expression * operand) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Expression * expr = calloc(1, sizeof(Expression));
	expr->kind = EXPRESSION_UNARY;
	expr->unary.operator = UNARY_OPERATOR_POST_INCREMENT;
	expr->unary.operand = operand;
	return expr;
}

Expression * PostfixDecrementExpressionSemanticAction(Expression * operand) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Expression * expr = calloc(1, sizeof(Expression));
	expr->kind = EXPRESSION_UNARY;
	expr->unary.operator = UNARY_OPERATOR_POST_DECREMENT;
	expr->unary.operand = operand;
	return expr;
}

Statement * VariableDeclarationSemanticAction(Type * type, char * name, Expression * initializer) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Statement * stmt = calloc(1, sizeof(Statement));
	stmt->kind = STATEMENT_VARIABLE_DECLARATION;
	stmt->variableDeclaration.type = type;
	stmt->variableDeclaration.name = name;
	stmt->variableDeclaration.initializer = initializer;
	return stmt;
}

Statement * IfStatementSemanticAction(Expression * condition, Statement * thenBranch, Statement * elseBranch) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Statement * stmt = calloc(1, sizeof(Statement));
	stmt->kind = STATEMENT_IF;
	stmt->ifStatement.condition = condition;
	stmt->ifStatement.thenBranch = thenBranch;
	stmt->ifStatement.elseBranch = elseBranch;
	return stmt;
}

Statement * WhileStatementSemanticAction(Expression * condition, Statement * body) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Statement * stmt = calloc(1, sizeof(Statement));
	stmt->kind = STATEMENT_WHILE;
	stmt->whileStatement.condition = condition;
	stmt->whileStatement.body = body;
	return stmt;
}

Statement * ForStatementSemanticAction(Statement * initializer, Expression * condition, Expression * step, Statement * body) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Statement * stmt = calloc(1, sizeof(Statement));
	stmt->kind = STATEMENT_FOR;
	stmt->forStatement.initializer = initializer;
	stmt->forStatement.condition = condition;
	stmt->forStatement.step = step;
	stmt->forStatement.body = body;
	return stmt;
}

Statement * ReturnStatementSemanticAction(Expression * value) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Statement * stmt = calloc(1, sizeof(Statement));
	stmt->kind = STATEMENT_RETURN;
	stmt->returnStatement.value = value;
	return stmt;
}

StatementList * AppendStatementSemanticAction(StatementList * list, Statement * stmt) {
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

Statement * ExpressionStatementSemanticAction(Expression * expr) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Statement * stmt = calloc(1, sizeof(Statement));
	stmt->kind = STATEMENT_EXPRESSION;
	stmt->expressionStatement = expr;
	return stmt;
}

Statement * BlockStatementSemanticAction(StatementList * body) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Statement * stmt = calloc(1, sizeof(Statement));
	stmt->kind = STATEMENT_BLOCK;
	stmt->block = body;
	return stmt;
}
