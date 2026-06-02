#include "Generator.h"

/* MODULE INTERNAL STATE */

const char _indentationCharacter = ' ';
const char _indentationSize = 4;
static Logger * _logger = NULL;

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
static void _generateForwardDeclarations(Program * program);
static void _generateField(Field * field);
static void _generateClass(Class * classNode);
static void _generateClassesInOrder(Program * program);
static char * _manglingTypeName(Type * type);
static char * _generateMangling(Parameter * params);
static void _generateParams(Parameter * params, const char * className);
static void _generateMethod(Class * classNode, Method * method);
static void _generateConstructor(Class * classNode, Method * method);
static void _generateClassMethods(Class * classNode);

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

/** DSL type → C type string (heap-allocated, caller must free). bool→char, string→char*, class→ClassName*. */
static char * _generateTypeName(Type * type) {
	if (type == NULL) {
		return strdup("void");
	}
	switch (type->kind) {
		case TYPEKIND_INT:    return strdup("int");
		case TYPEKIND_BOOL:   return strdup("char");
		case TYPEKIND_STRING: return strdup("char *");
		case TYPEKIND_CHAR:   return strdup("char");
		case TYPEKIND_FLOAT:  return strdup("float");
		case TYPEKIND_DOUBLE: return strdup("double");
		case TYPEKIND_VOID:   return strdup("void");
		case TYPEKIND_CLASS:  return concatenate(2, type->className, " *");
		default:
			logError(_logger, "Unknown type kind: %d", type->kind);
			return strdup("void");
	}
}

static void _generateField(Field * field) {
	char * typeName = _generateTypeName(field->type);
	_output(1, "%s %s;\n", typeName, field->name);
	free(typeName);
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
	if (type == NULL) return strdup("void");
	switch (type->kind) {
		case TYPEKIND_INT:    return strdup("int");
		case TYPEKIND_BOOL:   return strdup("char");
		case TYPEKIND_STRING: return strdup("charptr");
		case TYPEKIND_CHAR:   return strdup("char");
		case TYPEKIND_FLOAT:  return strdup("float");
		case TYPEKIND_DOUBLE: return strdup("double");
		case TYPEKIND_VOID:   return strdup("void");
		case TYPEKIND_CLASS:  return strdup(type->className);
		default:              return strdup("unknown");
	}
}

/** Generates the mangling suffix for a parameter list. */

static char * _generateMangling(Parameter * params) {
	char * result = strdup("");
	if (params == NULL){
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
		char * typeName = _generateTypeName(p->type);
		_output(0, "%s %s", typeName, p->name);
		free(typeName);
		first = false;
	}
	_output(0, ")");
}

static void _generateMethod(Class * classNode, Method * method) {
	char * mangling = _generateMangling(method->parameters);
	char * retType = _generateTypeName(method->returnType);
	const char * selfClass = method->isStatic ? NULL : classNode->name;
	_output(0, "%s %s__%s%s", retType, classNode->name, method->name, mangling);
	free(mangling);
	free(retType);
	_generateParams(method->parameters, selfClass);
	_output(0, " {\n");
	_output(0, "}\n\n");
}

static void _generateConstructor(Class * classNode, Method * method) {
	char * mangling = _generateMangling(method->parameters);
	_output(0, "%s * %s__new%s", classNode->name, classNode->name, mangling);
	free(mangling);
	_generateParams(method->parameters, NULL);
	_output(0, " {\n");
	_output(1, "%s * self = _xmalloc(sizeof(%s));\n", classNode->name, classNode->name);
	_output(1, "return self;\n");
	_output(0, "}\n\n");
}

static void _generateClassMethods(Class * classNode) {
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
		"static void * _xmalloc(size_t size) {\n"
		"    void * ptr = malloc(size);\n"
		"    if (!ptr) { fprintf(stderr, \"out of memory\\n\"); exit(1); }\n"
		"    return ptr;\n"
		"}\n\n"
	);
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
	logDebugging(_logger, "Generating C output...");
	_generatePrologue();
	_generateForwardDeclarations(program);
	_generateClassesInOrder(program);
	for (Class * c = program->classes; c != NULL; c = c->next) {
		_generateClassMethods(c);
	}
	logDebugging(_logger, "Generation done.");
}
