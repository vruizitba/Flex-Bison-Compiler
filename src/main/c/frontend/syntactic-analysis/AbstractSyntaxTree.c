#include "AbstractSyntaxTree.h"

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;

/** Shutdown module's internal state. */
void _shutdownAbstractSyntaxTreeModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: AbstractSyntaxTree...");
		destroyLogger(_logger);
		_logger = NULL;
	}
}

ModuleDestructor initializeAbstractSyntaxTreeModule() {
	_logger = createLogger("AbstractSyntaxTree");
	return _shutdownAbstractSyntaxTreeModule;
}

/* PUBLIC FUNCTIONS */

void destroyConstant(Constant * constant) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (constant != NULL) {
		free(constant);
	}
}

void destroyExpression(Expression * expression) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (expression != NULL) {
		switch (expression->type) {
			case ADDITION:
			case DIVISION:
			case MULTIPLICATION:
			case SUBTRACTION:
				destroyExpression(expression->leftExpression);
				destroyExpression(expression->rightExpression);
				break;
			case FACTOR:
				destroyFactor(expression->factor);
				break;
		}
		free(expression);
	}
}

void destroyFactor(Factor * factor) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (factor != NULL) {
		switch (factor->type) {
			case CONSTANT:
				destroyConstant(factor->constant);
				break;
			case EXPRESSION:
				destroyExpression(factor->expression);
				break;
		}
		free(factor);
	}
}

void destroyProgram(Program * program) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (program != NULL) {
		destroyExpression(program->expression);
		free(program);
	}
}

/**
 * Our DSL types destructors. For now, we won't delete the previous ones.
 */

void destroyType(Type * type) {
	if (type == NULL) {
		return;
	}
	free(type->className);
	destroyType(type->inner);
	free(type);
}

void destroyDslExpression(DslExpression * expression) {
	if (expression == NULL) {
		return;
	}
	switch (expression->kind) {
		case EXPRESSION_STRING:
			free(expression->stringValue);
			break;
		case EXPRESSION_IDENTIFIER:
			free(expression->identifier);
			break;
		case EXPRESSION_BINARY:
			destroyDslExpression(expression->binary.left);
			destroyDslExpression(expression->binary.right);
			break;
		case EXPRESSION_UNARY:
			destroyDslExpression(expression->unary.operand);
			break;
		case EXPRESSION_FIELD_ACCESS:
		case EXPRESSION_ARROW_ACCESS:
			destroyDslExpression(expression->fieldAccess.object);
			free(expression->fieldAccess.field);
			break;
		case EXPRESSION_INDEX:
			destroyDslExpression(expression->indexAccess.array);
			destroyDslExpression(expression->indexAccess.index);
			break;
		case EXPRESSION_CALL:
			destroyDslExpression(expression->call.callee);
			destroyArgumentList(expression->call.arguments);
			break;
		case EXPRESSION_NEW:
			free(expression->newExpression.className);
			destroyArgumentList(expression->newExpression.arguments);
			break;
		default:
			break;
	}
	free(expression);
}

void destroyDslStatement(DslStatement * statement) {
	if (statement == NULL) {
		return;
	}
	switch (statement->kind) {
		case STATEMENT_VARIABLE_DECLARATION:
			destroyType(statement->variableDeclaration.type);
			free(statement->variableDeclaration.name);
			destroyDslExpression(statement->variableDeclaration.initializer);
			break;
		case STATEMENT_EXPRESSION:
			destroyDslExpression(statement->expressionStatement);
			break;
		case STATEMENT_IF:
			destroyDslExpression(statement->ifStatement.condition);
			destroyDslStatement(statement->ifStatement.thenBranch);
			destroyDslStatement(statement->ifStatement.elseBranch);
			break;
		case STATEMENT_WHILE:
			destroyDslExpression(statement->whileStatement.condition);
			destroyDslStatement(statement->whileStatement.body);
			break;
		case STATEMENT_FOR:
			destroyDslStatement(statement->forStatement.initializer);
			destroyDslExpression(statement->forStatement.condition);
			destroyDslExpression(statement->forStatement.step);
			destroyDslStatement(statement->forStatement.body);
			break;
		case STATEMENT_RETURN:
			destroyDslExpression(statement->returnStatement.value);
			break;
		case STATEMENT_BLOCK:
			destroyStatementList(statement->block);
			break;
	}
	free(statement);
}

void destroyStatementList(StatementList * list) {
	if (list == NULL) {
		return;
	}
	destroyDslStatement(list->statement);
	destroyStatementList(list->next);
	free(list);
}

void destroyArgumentList(ArgumentList * list) {
	if (list == NULL) {
		return;
	}
	destroyDslExpression(list->expression);
	destroyArgumentList(list->next);
	free(list);
}

void destroyParameter(Parameter * parameter) {
	if (parameter == NULL) {
		return;
	}
	destroyType(parameter->type);
	free(parameter->name);
	destroyParameter(parameter->next);
	free(parameter);
}

void destroyMemberList(MemberList * memberList) {
	if (memberList == NULL) {
		return;
	}
	destroyField(memberList->fields);
	destroyMethod(memberList->methods);
	free(memberList);
}

void destroyMemberSuffix(MemberSuffix * suffix) {
	if (suffix == NULL) {
		return;
	}
	destroyType(suffix->typeTail);
	free(suffix->memberName);
	destroyParameter(suffix->parameters);
	destroyStatementList(suffix->body);
	free(suffix);
}

void destroyField(Field * field) {
	if (field == NULL) {
		return;
	}
	destroyType(field->type);
	free(field->name);
	destroyDslExpression(field->initializer);
	destroyField(field->next);
	free(field);
}

void destroyMethod(Method * method) {
	if (method == NULL) {
		return;
	}
	destroyType(method->returnType);
	free(method->name);
	destroyParameter(method->parameters);
	destroyStatementList(method->body);
	destroyMethod(method->next);
	free(method);
}

void destroyClass(Class * class) {
	if (class == NULL) {
		return;
	}
	free(class->name);
	free(class->parentName);
	destroyField(class->fields);
	destroyMethod(class->methods);
	destroyClass(class->next);
	free(class);
}

void destroyFunction(Function * function) {
	if (function == NULL) {
		return;
	}
	destroyType(function->returnType);
	free(function->name);
	destroyParameter(function->parameters);
	destroyStatementList(function->body);
	destroyFunction(function->next);
	free(function);
}

void destroyDslProgram(DslProgram * program) {
	if (program == NULL) {
		return;
	}
	destroyClass(program->classes);
	destroyFunction(program->functions);
	destroyStatementList(program->mainBody);
	free(program);
}
