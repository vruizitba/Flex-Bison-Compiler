#include "Generator.h"

/* MODULE INTERNAL STATE */

const char _indentationCharacter = ' ';
const char _indentationSize = 4;
static Logger * _logger = NULL;
static Program * _currentProgram = NULL;

/** Shutdown module's internal state. */
void _shutdownGeneratorModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: Generator...");
		destroyLogger(_logger);
		_logger = NULL;
	}
}

ModuleDestructor initializeGeneratorModule() {
	_logger = createLogger("Generator");
	return _shutdownGeneratorModule;
}

/** PRIVATE FUNCTIONS */

static char * _indentation(const unsigned int indentationLevel);
static void _output(const unsigned int indentationLevel, const char * const format, ...);
static void _generatePrologue(void);
static char * _generateTypeName(Type * type);
static char * _generateDeclarator(Type * type, const char * name);
static void _generateForwardDeclarations(Program * program);
static void _generateField(Field * field);
static void _generateClass(Class * classNode);
static void _generateClassesInOrder(Program * program);
static char * _manglingTypeName(Type * type);
static char * _generateMangling(Parameter * params);
static void _generateParams(Parameter * params, const char * className);
static void _generateSignature(const char * retType, const char * name, Parameter * params, const char * selfClass);
static void _generatePrototypes(Program * program);
static bool _classHasConstructor(Class * classNode);
static void _generateMethod(Class * classNode, Method * method);
static void _generateConstructor(Class * classNode, Method * method);
static void _generateClassMethods(Class * classNode);
static void _generateArgumentList(const char * name, ArgumentList * args);
static void _generateExpression(Expression * expr);
static void _generateStatement(Statement * stmt, unsigned int indent);
static void _generateStatementList(StatementList * list, unsigned int indent);
static void _generateBlockStatement(Statement * stmt, unsigned int indent);
static void _generateReturnStatement(Statement * stmt, unsigned int indent);
static void _generateExpressionStatement(Statement * stmt, unsigned int indent);
static void _generateVariableDeclaration(Statement * stmt, unsigned int indent);
static void _generateIfStatement(Statement * stmt, unsigned int indent);
static void _generateWhileStatement(Statement * stmt, unsigned int indent);
static void _generateForStatement(Statement * stmt, unsigned int indent);
static void _generateForInitializer(Statement * stmt);
static void _generateFunction(Function * function);
static void _generateMain(StatementList * body);

/* Legacy. */
#if 0

static const char _expressionTypeToCharacter(const ExpressionType type);
static void _generateConstant(const unsigned int indentationLevel, Constant * constant);
static void _generateEpilogue(const int value);
static void _generateExpression(const unsigned int indentationLevel, Expression * expression);
static void _generateFactor(const unsigned int indentationLevel, Factor * factor);
static void _generateProgram(Program * program);
static void _generatePrologue(void);

/**
 * Converts and expression type to the proper character of the operation
 * involved, or returns '\0' if that's not possible.
 */
static const char _expressionTypeToCharacter(const ExpressionType type) {
	switch (type) {
		case ADDITION: return '+';
		case DIVISION: return '/';
		case MULTIPLICATION: return '*';
		case SUBTRACTION: return '-';
		default:
			logError(_logger, "The specified expression type cannot be converted into character: %d", type);
			return '\0';
	}
}

/**
 * Generates the output of a constant.
 */
static void _generateConstant(const unsigned int indentationLevel, Constant * constant) {
	_output(indentationLevel, "%s", "[ $C$, circle, draw, black!20\n");
	_output(1 + indentationLevel, "%s%d%s", "[ $", constant->value, "$, circle, draw ]\n");
	_output(indentationLevel, "%s", "]\n");
}

/**
 * Creates the epilogue of the generated output, that is, the final lines that
 * completes a valid Latex document.
 */
static void _generateEpilogue(const int value) {
	_output(0, "%s%d%s",
		"            [ $", value, "$, circle, draw, blue ]\n"
		"        ]\n"
		"    \\end{forest}\n"
		"\\end{document}\n\n"
	);
}

/**
 * Generates the output of an expression.
 */
static void _generateExpression(const unsigned int indentationLevel, Expression * expression) {
	_output(indentationLevel, "%s", "[ $E$, circle, draw, black!20\n");
	switch (expression->type) {
		case ADDITION:
		case DIVISION:
		case MULTIPLICATION:
		case SUBTRACTION:
			_generateExpression(1 + indentationLevel, expression->leftExpression);
			_output(1 + indentationLevel, "%s%c%s", "[ $", _expressionTypeToCharacter(expression->type), "$, circle, draw, purple ]\n");
			_generateExpression(1 + indentationLevel, expression->rightExpression);
			break;
		case FACTOR:
			_generateFactor(1 + indentationLevel, expression->factor);
			break;
		default:
			logError(_logger, "The specified expression type is unknown: %d", expression->type);
			break;
	}
	_output(indentationLevel, "%s", "]\n");
}

/**
 * Generates the output of a factor.
 */
static void _generateFactor(const unsigned int indentationLevel, Factor * factor) {
	_output(indentationLevel, "%s", "[ $F$, circle, draw, black!20\n");
	switch (factor->type) {
		case CONSTANT:
			_generateConstant(1 + indentationLevel, factor->constant);
			break;
		case EXPRESSION:
			_output(1 + indentationLevel, "%s", "[ $($, circle, draw, purple ]\n");
			_generateExpression(1 + indentationLevel, factor->expression);
			_output(1 + indentationLevel, "%s", "[ $)$, circle, draw, purple ]\n");
			break;
		default:
			logError(_logger, "The specified factor type is unknown: %d", factor->type);
			break;
	}
	_output(indentationLevel, "%s", "]\n");
}

/**
 * Generates the output of the program.
 */
static void _generateProgram(Program * program) {
	_generateExpression(3, program->expression);
}

/**
 * Creates the prologue of the generated output, a Latex document that renders
 * a tree thanks to the Forest package.
 *
 * @see https://ctan.dcc.uchile.cl/graphics/pgf/contrib/forest/forest-doc.pdf
 */
static void _generatePrologue(void) {
	_output(0, "%s",
		"\\documentclass{standalone}\n\n"
		"\\usepackage[utf8]{inputenc}\n"
		"\\usepackage[T1]{fontenc}\n"
		"\\usepackage{amsmath}\n"
		"\\usepackage{forest}\n"
		"\\usepackage{microtype}\n\n"
		"\\begin{document}\n"
		"    \\centering\n"
		"    \\begin{forest}\n"
		"        [ \\text{$=$}, circle, draw, purple\n"
	);
}

#endif

/**
 * Generates an indentation string for the specified level.
 */
static char * _indentation(const unsigned int level) {
	return indentation(_indentationCharacter, level, _indentationSize);
}

/**
 * Outputs a formatted string to standard output. The "fflush" instruction
 * allows to see the output even close to a failure, because it drops the
 * buffering.
 */
static void _output(const unsigned int indentationLevel, const char * const format, ...) {
	va_list arguments;
	va_start(arguments, format);
	char * indentation = _indentation(indentationLevel);
	char * effectiveFormat = concatenate(2, indentation, format);
	vfprintf(stdout, effectiveFormat, arguments);
	fflush(stdout);
	free(effectiveFormat);
	free(indentation);
	va_end(arguments);
}

static char * _generateModifierPrefix(Type * type) {
	char buf[32] = "";
	if (type->isUnsigned) {
		strcat(buf, "unsigned ");
	} else if (type->isSigned) {
		strcat(buf, "signed ");
	}
	if (type->isShort) {
		strcat(buf, "short ");
	} else if (type->isLong) {
		strcat(buf, "long ");
	}
	return strdup(buf);
}

/** DSL type → C type string (heap-allocated, caller must free). bool→char, string→char*, class→ClassName*. */
static char * _generateTypeName(Type * type) {
	if (type == NULL) {
		return strdup("void");
	}
	switch (type->kind) {
		case TYPEKIND_INT: {
			char * prefix = _generateModifierPrefix(type);
			char * result = concatenate(2, prefix, "int");
			free(prefix);
			return result;
		}
		case TYPEKIND_BOOL:   return strdup("char");
		case TYPEKIND_STRING: return strdup("char *");
		case TYPEKIND_CHAR: {
			char * prefix = _generateModifierPrefix(type);
			char * result = concatenate(2, prefix, "char");
			free(prefix);
			return result;
		}
		case TYPEKIND_FLOAT:  return strdup("float");
		case TYPEKIND_DOUBLE: {
			char * prefix = _generateModifierPrefix(type);
			char * result = concatenate(2, prefix, "double");
			free(prefix);
			return result;
		}
		case TYPEKIND_VOID:   return strdup("void");
		case TYPEKIND_CLASS:  return concatenate(2, type->className, " *");
		case TYPEKIND_POINTER: {
			char * inner = _generateTypeName(type->inner);
			char * result = concatenate(2, inner, " *");
			free(inner);
			return result;
		}
		case TYPEKIND_ARRAY: {
			/* As a general type (return, parameter): decay to pointer. */
			char * inner = _generateTypeName(type->inner);
			char * result = concatenate(2, inner, " *");
			free(inner);
			return result;
		}
		default:
			logError(_logger, "Unknown type kind: %d", type->kind);
			return strdup("void");
	}
}

static char * _generateDeclarator(Type * type, const char * name) {
	if (type->kind == TYPEKIND_ARRAY && type->arraySize >= 0) {
		char * inner = _generateTypeName(type->inner);
		char arraySuffix[32];
		snprintf(arraySuffix, sizeof(arraySuffix), "[%d]", type->arraySize);
		char * result = concatenate(4, inner, " ", name, arraySuffix);
		free(inner);
		return result;
	}
	if (type->kind == TYPEKIND_ARRAY && type->arraySize == -1) {
		/* Unsized array parameter: decay to pointer. */
		char * inner = _generateTypeName(type->inner);
		char * result = concatenate(3, inner, " *", name);
		free(inner);
		return result;
	}
	char * typeName = _generateTypeName(type);
	char * result = concatenate(3, typeName, " ", name);
	free(typeName);
	return result;
}

static void _generateField(Field * field) {
	char * decl = _generateDeclarator(field->type, field->name);
	_output(1, "%s;\n", decl);
	free(decl);
}

static void _generateClass(Class * classNode) {
	_output(0, "struct %s {\n", classNode->name);
	if (classNode->parentName != NULL) {
		_output(1, "%s parent;\n", classNode->parentName);
	}
	for (Field * f = classNode->fields; f != NULL; f = f->next) {
		_generateField(f);
	}
	_output(0, "};\n\n");
}

static void _generateClassesInOrder(Program * program) {
	int total = 0;
	for (Class * c = program->classes; c != NULL; c = c->next) {
		total++;
	}
	bool * emitted = calloc(total, sizeof(bool));
	int done = 0;
	while (done < total) {
		int doneBeforePass = done;
		int idx = 0;
		for (Class * c = program->classes; c != NULL; c = c->next, idx++) {
			if (emitted[idx]) {
				continue;
			}
			bool parentReady = true;
			if (c->parentName != NULL) {
				int pidx = 0;
				for (Class * p = program->classes; p != NULL; p = p->next, pidx++) {
					if (strcmp(p->name, c->parentName) == 0 && !emitted[pidx]) {
						parentReady = false;
						break;
					}
				}
			}
			if (parentReady) {
				_generateClass(c);
				emitted[idx] = true;
				done++;
			}
		}
		if (done == doneBeforePass) {
			logError(_logger, "Inheritance cycle detected during code generation; aborting.");
			break;
		}
	}
	free(emitted);
}

static void _generateForwardDeclarations(Program * program) {
	for (Class * c = program->classes; c != NULL; c = c->next) {
		_output(0, "typedef struct %s %s;\n", c->name, c->name);
	}
	_output(0, "\n");
}

static char * _manglingTypeName(Type * type) {
	if (type == NULL) {
		return strdup("void");
	}
	switch (type->kind) {
		case TYPEKIND_INT:    return strdup("int");
		case TYPEKIND_BOOL:   return strdup("char");
		case TYPEKIND_STRING: return strdup("charptr");
		case TYPEKIND_CHAR:   return strdup("char");
		case TYPEKIND_FLOAT:  return strdup("float");
		case TYPEKIND_DOUBLE: return strdup("double");
		case TYPEKIND_VOID:   return strdup("void");
		case TYPEKIND_CLASS:  return strdup(type->className);
		case TYPEKIND_POINTER: {
			char * inner = _manglingTypeName(type->inner);
			char * result = concatenate(2, inner, "ptr");
			free(inner);
			return result;
		}
		case TYPEKIND_ARRAY: {
			char * inner = _manglingTypeName(type->inner);
			char * result = concatenate(2, inner, "arr");
			free(inner);
			return result;
		}
		default:              return strdup("unknown");
	}
}

/** Generates the mangling suffix for a parameter list. */

static char * _generateMangling(Parameter * params) {
	char * result = strdup("");
	if (params == NULL) {
		return result;
	}
	for (Parameter * p = params; p != NULL; p = p->next) {
		char * typeName = _manglingTypeName(p->type);
		char * sep = (p == params) ? "__" : "_";
		char * next = concatenate(3, result, sep, typeName);
		free(result);
		free(typeName);
		result = next;
	}
	return result;
}

/** Emits the parameter list to stdout, including self if className is non-NULL. */
static void _generateParams(Parameter * params, const char * className) {
	_output(0, "(");
	bool first = true;
	if (className != NULL) {
		_output(0, "%s * self", className);
		first = false;
	}
	for (Parameter * p = params; p != NULL; p = p->next) {
		if (!first) {
			_output(0, ", ");
		}
		char * decl = _generateDeclarator(p->type, p->name);
		_output(0, "%s", decl);
		free(decl);
		first = false;
	}
	_output(0, ")");
}

static void _generateArgumentList(const char * name, ArgumentList * args) {
	_output(0, "%s(", name);
	bool first = true;
	for (ArgumentList * a = args; a != NULL; a = a->next) {
		if (!first) {
			_output(0, ", ");
		}
		_generateExpression(a->expression);
		first = false;
	}
	_output(0, ")");
}

static void _generateExpression(Expression * expr) {
	if (expr == NULL) {
		return;
	}
	switch (expr->kind) {
		case EXPRESSION_INTEGER:
			_output(0, "%d", expr->integerValue);
			break;
		case EXPRESSION_FLOAT:
			_output(0, "%g", expr->floatValue);
			break;
		case EXPRESSION_STRING:
			_output(0, "\"%s\"", expr->stringValue);
			break;
		case EXPRESSION_CHAR:
			_output(0, "'%c'", expr->charValue);
			break;
		case EXPRESSION_BOOLEAN:
			_output(0, "%d", expr->booleanValue ? 1 : 0);
			break;
		case EXPRESSION_IDENTIFIER:
			_output(0, "%s", expr->identifier);
			break;
		case EXPRESSION_THIS:
			_output(0, "self");
			break;
		case EXPRESSION_BINARY: {
			const char * op;
			switch (expr->binary.operator) {
				case BINARY_OPERATOR_ADD:
					op = "+";
					break;
				case BINARY_OPERATOR_SUB:
					op = "-";
					break;
				case BINARY_OPERATOR_MUL:
					op = "*";
					break;
				case BINARY_OPERATOR_DIV:
					op = "/";
					break;
				case BINARY_OPERATOR_MOD:
					op = "%";
					break;
				case BINARY_OPERATOR_EQUAL:
					op = "==";
					break;
				case BINARY_OPERATOR_NOT_EQUAL:
					op = "!=";
					break;
				case BINARY_OPERATOR_LESS:
					op = "<";
					break;
				case BINARY_OPERATOR_GREATER:
					op = ">";
					break;
				case BINARY_OPERATOR_LESS_EQUAL:
					op = "<=";
					break;
				case BINARY_OPERATOR_GREATER_EQUAL:
					op = ">=";
					break;
				case BINARY_OPERATOR_AND:
					op = "&&";
					break;
				case BINARY_OPERATOR_OR:
					op = "||";
					break;
				case BINARY_OPERATOR_ASSIGN:
					op = "=";
					break;
				case BINARY_OPERATOR_PLUS_ASSIGN:
					op = "+=";
					break;
				case BINARY_OPERATOR_MINUS_ASSIGN:
					op = "-=";
					break;
				case BINARY_OPERATOR_MUL_ASSIGN:
					op = "*=";
					break;
				case BINARY_OPERATOR_DIV_ASSIGN:
					op = "/=";
					break;
				case BINARY_OPERATOR_MOD_ASSIGN:
					op = "%=";
					break;
				default:
					op = "?";
					break;
			}
			_output(0, "(");
			_generateExpression(expr->binary.left);
			_output(0, " %s ", op);
			_generateExpression(expr->binary.right);
			_output(0, ")");
			break;
		}
		case EXPRESSION_UNARY:
			switch (expr->unary.operator) {
				case UNARY_OPERATOR_NOT:
					_output(0, "!");
					_generateExpression(expr->unary.operand);
					break;
				case UNARY_OPERATOR_NEGATE:
					_output(0, "-");
					_generateExpression(expr->unary.operand);
					break;
				case UNARY_OPERATOR_PRE_INCREMENT:
					_output(0, "++");
					_generateExpression(expr->unary.operand);
					break;
				case UNARY_OPERATOR_PRE_DECREMENT:
					_output(0, "--");
					_generateExpression(expr->unary.operand);
					break;
				case UNARY_OPERATOR_POST_INCREMENT:
					_generateExpression(expr->unary.operand);
					_output(0, "++");
					break;
				case UNARY_OPERATOR_POST_DECREMENT:
					_generateExpression(expr->unary.operand);
					_output(0, "--");
					break;
				case UNARY_OPERATOR_DEREFERENCE:
					_output(0, "*");
					_generateExpression(expr->unary.operand);
					break;
				case UNARY_OPERATOR_ADDRESS_OF:
					_output(0, "&");
					_generateExpression(expr->unary.operand);
					break;
				default:
					logError(_logger, "Unknown unary operator: %d.", expr->unary.operator);
					break;
			}
			break;
		case EXPRESSION_NEW: {
			Class * targetClass = NULL;
			for (Class * c = _currentProgram->classes; c != NULL && targetClass == NULL; c = c->next) {
				if (strcmp(c->name, expr->newExpression.className) == 0) {
					targetClass = c;
				}
			}
			if (targetClass == NULL) {
				logError(_logger, "new expression references undeclared class '%s'.", expr->newExpression.className);
				break;
			}
			Parameter * ctorParams = NULL;
			for (Method * m = targetClass->methods; m != NULL && ctorParams == NULL; m = m->next) {
				if (m->isConstructor) {
					ctorParams = m->parameters;
				}
			}
			char * mangling = _generateMangling(ctorParams);
			char * ctorName = concatenate(3, expr->newExpression.className, "__new", mangling);
			free(mangling);
			_generateArgumentList(ctorName, expr->newExpression.arguments);
			free(ctorName);
			break;
		}
		case EXPRESSION_CALL: {
			Expression * callee = expr->call.callee;
			if (callee->kind == EXPRESSION_IDENTIFIER) {
				_generateArgumentList(callee->identifier, expr->call.arguments);
			} else if ((callee->kind == EXPRESSION_FIELD_ACCESS || callee->kind == EXPRESSION_ARROW_ACCESS)
					&& expr->resolvedMethod != NULL) {
				Method * method = expr->resolvedMethod;
				char * mangling = _generateMangling(method->parameters);
				_output(0, "%s__%s%s(", expr->resolvedOwnerClass, callee->fieldAccess.field, mangling);
				free(mangling);
				bool first = true;
				if (!method->isStatic) {
					_output(0, "(%s *) ", expr->resolvedOwnerClass);
					_generateExpression(callee->fieldAccess.object);
					first = false;
				}
				for (ArgumentList * a = expr->call.arguments; a != NULL; a = a->next) {
					if (!first) {
						_output(0, ", ");
					}
					_generateExpression(a->expression);
					first = false;
				}
				_output(0, ")");
			} else {
				logError(_logger, "Unresolved method call in code generation.");
			}
			break;
		}
		case EXPRESSION_FIELD_ACCESS:
		case EXPRESSION_ARROW_ACCESS:
			_generateExpression(expr->fieldAccess.object);
			_output(0, "->");
			for (int i = 0; i < expr->resolvedFieldDepth; i++) {
				_output(0, "parent.");
			}
			_output(0, "%s", expr->fieldAccess.field);
			break;
		case EXPRESSION_INDEX:
			_generateExpression(expr->indexAccess.array);
			_output(0, "[");
			_generateExpression(expr->indexAccess.index);
			_output(0, "]");
			break;
		default:
			logError(_logger, "Unknown expression kind: %d.", expr->kind);
			break;
	}
}

static void _generateBlockStatement(Statement * stmt, unsigned int indent) {
	_output(indent, "{\n");
	_generateStatementList(stmt->block, indent + 1);
	_output(indent, "}\n");
}

static void _generateReturnStatement(Statement * stmt, unsigned int indent) {
	_output(indent, "return ");
	_generateExpression(stmt->returnStatement.value);
	_output(0, ";\n");
}

static void _generateExpressionStatement(Statement * stmt, unsigned int indent) {
	_output(indent, "");
	_generateExpression(stmt->expressionStatement);
	_output(0, ";\n");
}

static void _generateVariableDeclaration(Statement * stmt, unsigned int indent) {
	char * decl = _generateDeclarator(stmt->variableDeclaration.type, stmt->variableDeclaration.name);
	if (stmt->variableDeclaration.initializer != NULL) {
		_output(indent, "%s = ", decl);
		_generateExpression(stmt->variableDeclaration.initializer);
		_output(0, ";\n");
	} else {
		_output(indent, "%s;\n", decl);
	}
	free(decl);
}

static void _generateIfStatement(Statement * stmt, unsigned int indent) {
	_output(indent, "if (");
	_generateExpression(stmt->ifStatement.condition);
	_output(0, ") {\n");
	_generateStatement(stmt->ifStatement.thenBranch, indent + 1);
	if (stmt->ifStatement.elseBranch != NULL) {
		_output(indent, "} else {\n");
		_generateStatement(stmt->ifStatement.elseBranch, indent + 1);
	}
	_output(indent, "}\n");
}

static void _generateWhileStatement(Statement * stmt, unsigned int indent) {
	_output(indent, "while (");
	_generateExpression(stmt->whileStatement.condition);
	_output(0, ") {\n");
	_generateStatement(stmt->whileStatement.body, indent + 1);
	_output(indent, "}\n");
}

static void _generateForInitializer(Statement * stmt) {
	if (stmt == NULL) {
		return;
	}
	if (stmt->kind == STATEMENT_VARIABLE_DECLARATION) {
		char * decl = _generateDeclarator(stmt->variableDeclaration.type, stmt->variableDeclaration.name);
		_output(0, "%s", decl);
		free(decl);
		if (stmt->variableDeclaration.initializer != NULL) {
			_output(0, " = ");
			_generateExpression(stmt->variableDeclaration.initializer);
		}
	} else if (stmt->kind == STATEMENT_EXPRESSION) {
		_generateExpression(stmt->expressionStatement);
	}
}

static void _generateForStatement(Statement * stmt, unsigned int indent) {
	_output(indent, "for (");
	_generateForInitializer(stmt->forStatement.initializer);
	_output(0, "; ");
	_generateExpression(stmt->forStatement.condition);
	_output(0, "; ");
	_generateExpression(stmt->forStatement.step);
	_output(0, ") {\n");
	_generateStatement(stmt->forStatement.body, indent + 1);
	_output(indent, "}\n");
}

static void _generateStatement(Statement * stmt, unsigned int indent) {
	if (stmt == NULL) {
		return;
	}
	switch (stmt->kind) {
		case STATEMENT_BLOCK:              return _generateBlockStatement(stmt, indent);
		case STATEMENT_RETURN:             return _generateReturnStatement(stmt, indent);
		case STATEMENT_EXPRESSION:         return _generateExpressionStatement(stmt, indent);
		case STATEMENT_VARIABLE_DECLARATION: return _generateVariableDeclaration(stmt, indent);
		case STATEMENT_IF:                 return _generateIfStatement(stmt, indent);
		case STATEMENT_WHILE:              return _generateWhileStatement(stmt, indent);
		case STATEMENT_FOR:                return _generateForStatement(stmt, indent);
		default:
			logError(_logger, "Unknown statement kind: %d.", stmt->kind);
	}
}

static void _generateStatementList(StatementList * list, unsigned int indent) {
	for (StatementList * s = list; s != NULL; s = s->next) {
		_generateStatement(s->statement, indent);
	}
}

static void _generateSignature(const char * retType, const char * name, Parameter * params, const char * selfClass) {
	_output(0, "%s %s", retType, name);
	_generateParams(params, selfClass);
}

/** Emits forward declarations (prototypes) for every method, constructor and free function. */
static void _generatePrototypes(Program * program) {
	for (Class * c = program->classes; c != NULL; c = c->next) {
		if (!_classHasConstructor(c)) {
			_output(0, "%s * %s__new();\n", c->name, c->name);
		}
		for (Method * m = c->methods; m != NULL; m = m->next) {
			char * mangling = _generateMangling(m->parameters);
			if (m->isConstructor) {
				char * retType = concatenate(2, c->name, " *");
				char * name = concatenate(3, c->name, "__new", mangling);
				_generateSignature(retType, name, m->parameters, NULL);
				free(retType);
				free(name);
			} else {
				char * retType = _generateTypeName(m->returnType);
				char * name = concatenate(4, c->name, "__", m->name, mangling);
				const char * selfClass = m->isStatic ? NULL : c->name;
				_generateSignature(retType, name, m->parameters, selfClass);
				free(retType);
				free(name);
			}
			free(mangling);
			_output(0, ";\n");
		}
	}
	for (Function * f = program->functions; f != NULL; f = f->next) {
		char * retType = _generateTypeName(f->returnType);
		_generateSignature(retType, f->name, f->parameters, NULL);
		free(retType);
		_output(0, ";\n");
	}
	_output(0, "\n");
}

static void _generateMethod(Class * classNode, Method * method) {
	char * mangling = _generateMangling(method->parameters);
	char * retType = _generateTypeName(method->returnType);
	const char * selfClass = method->isStatic ? NULL : classNode->name;
	char * name = concatenate(4, classNode->name, "__", method->name, mangling);
	_generateSignature(retType, name, method->parameters, selfClass);
	free(mangling);
	free(retType);
	free(name);
	_output(0, " {\n");
	_generateStatementList(method->body, 1);
	_output(0, "}\n\n");
}

/** True if the class declares at least one constructor. */
static bool _classHasConstructor(Class * classNode) {
	for (Method * m = classNode->methods; m != NULL; m = m->next) {
		if (m->isConstructor) {
			return true;
		}
	}
	return false;
}

/** Generates a constructor. A NULL method emits the implicit default constructor (no params, no body). */
static void _generateConstructor(Class * classNode, Method * method) {
	Parameter * params = method ? method->parameters : NULL;
	StatementList * body = method ? method->body : NULL;
	char * mangling = _generateMangling(params);
	char * retType = concatenate(2, classNode->name, " *");
	char * name = concatenate(3, classNode->name, "__new", mangling);
	_generateSignature(retType, name, params, NULL);
	free(mangling);
	free(retType);
	free(name);
	_output(0, " {\n");
	_output(1, "%s * self = _xmalloc(sizeof(%s));\n", classNode->name, classNode->name);
	_generateStatementList(body, 1);
	_output(1, "return self;\n");
	_output(0, "}\n\n");
}

static void _generateClassMethods(Class * classNode) {
	if (!_classHasConstructor(classNode)) {
		_generateConstructor(classNode, NULL);
	}
	for (Method * m = classNode->methods; m != NULL; m = m->next) {
		if (m->isConstructor) {
			_generateConstructor(classNode, m);
		} else {
			_generateMethod(classNode, m);
		}
	}
}

static void _generatePrologue(void) {
	_output(0, "%s",
		"#include <stdlib.h>\n"
		"#include <string.h>\n"
		"#include <stdio.h>\n\n"
		"/* Allocation registry: every object is tracked and freed at program exit. */\n"
		"#define BLOCK_SIZE 16\n\n"
		"static void ** _allocations = NULL;\n"
		"static int _allocCount = 0;\n\n"
		"static void _freeAll(void) {\n"
		"    for (int i = 0; i < _allocCount; i++) {\n"
		"        free(_allocations[i]);\n"
		"    }\n"
		"    free(_allocations);\n"
		"    _allocations = NULL;\n"
		"    _allocCount = 0;\n"
		"}\n\n"
		"static void * _xmalloc(size_t size) {\n"
		"    void * ptr = malloc(size);\n"
		"    if (!ptr) {\n"
		"        fprintf(stderr, \"out of memory\\n\");\n"
		"        exit(1);\n"
		"    }\n"
		"    if (_allocCount % BLOCK_SIZE == 0) {\n"
		"        _allocations = realloc(_allocations, (_allocCount + BLOCK_SIZE) * sizeof(void *));\n"
		"        if (!_allocations) {\n"
		"            fprintf(stderr, \"out of memory\\n\");\n"
		"            exit(1);\n"
		"        }\n"
		"    }\n"
		"    _allocations[_allocCount++] = ptr;\n"
		"    return ptr;\n"
		"}\n\n"
	);
}

static void _generateFunction(Function * function) {
	char * retType = _generateTypeName(function->returnType);
	_generateSignature(retType, function->name, function->parameters, NULL);
	free(retType);
	_output(0, " {\n");
	_generateStatementList(function->body, 1);
	_output(0, "}\n\n");
}

static void _generateMain(StatementList * body) {
	_output(0, "int main(void) {\n");
	_output(1, "atexit(_freeAll);\n");
	_generateStatementList(body, 1);
	_output(0, "}\n");
}

/** PUBLIC FUNCTIONS */

void executeGenerator(CompilerState * compilerState) {
	/* Legacy.
	logDebugging(_logger, "Generating final output...");
	_generatePrologue();
	_generateProgram(compilerState->abstractSyntaxtTree);
	_generateEpilogue(compilerState->value);
	logDebugging(_logger, "Generation is done.");
	*/
	Program * program = compilerState->abstractSyntaxtTree;
	if (program == NULL) {
		return;
	}
	_currentProgram = program;
	logDebugging(_logger, "Generating C output...");
	_generatePrologue();
	_generateForwardDeclarations(program);
	_generatePrototypes(program);
	_generateClassesInOrder(program);
	for (Class * c = program->classes; c != NULL; c = c->next) {
		_generateClassMethods(c);
	}
	for (Function * f = program->functions; f != NULL; f = f->next) {
		_generateFunction(f);
	}
	if (program->mainBody != NULL) {
		_generateMain(program->mainBody);
	}
	logDebugging(_logger, "Generation done.");
}
