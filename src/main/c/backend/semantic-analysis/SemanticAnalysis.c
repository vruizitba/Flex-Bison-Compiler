#include "SemanticAnalysis.h"
#include "SymbolTable.h"
#include "../../support/logging/Logger.h"

static Logger * _logger = NULL;

/* Expected return type (currentReturnType in SymbolTable) by context:
 *   - constructor      -> NULL              (method->returnType is NULL)
 *   - void fn/method   -> a TYPEKIND_VOID Type (the node's own returnType)
 *   - main body        -> &_voidReturnType  (this explicit VOID sentinel)
 * main has no return-type node, so it borrows this sentinel; using VOID
 * (not NULL) keeps main distinguishable from a constructor context. */
static Type _voidReturnType = { .kind = TYPEKIND_VOID };

static void _shutdownSemanticAnalysisModule() {
    if (_logger != NULL) {
        logDebugging(_logger, "Destroying module: SemanticAnalysis...");
        destroyLogger(_logger);
        _logger = NULL;
    }
}

ModuleDestructor initializeSemanticAnalysisModule() {
    _logger = createLogger("SemanticAnalysis");
    return _shutdownSemanticAnalysisModule;
}

static CompilationStatus _collectDeclarations(SymbolTable * table, Program * program) {
    CompilationStatus status = SUCCEEDED;

    /* Register all classes; registerClass also rejects duplicate method signatures. */
    for (Class * class = program->classes; class != NULL; class = class->next) {
        if (lookupClass(table, class->name) != NULL) {
            logError(_logger, "Duplicate class declaration: '%s'.", class->name);
            status = FAILED;
        } else if (!registerClass(table, class)) {
            logError(_logger, "Duplicate member (field or method signature) in class '%s'.", class->name);
            status = FAILED;
        }
    }

    /* Validate extends: parent must be a declared class. */
    for (Class * class = program->classes; class != NULL; class = class->next) {
        if (class->parentName != NULL && lookupClass(table, class->parentName) == NULL) {
            logError(_logger, "Class '%s' extends undefined class '%s'.", class->name, class->parentName);
            status = FAILED;
        }
    }

    /* Detect inheritance cycles before any parent-chain walk in _processBodies. */
    for (Class * class = program->classes; class != NULL; class = class->next) {
        if (hasInheritanceCycle(table, class->name)) {
            logError(_logger, "Inheritance cycle detected involving class '%s'.", class->name);
            status = FAILED;
        }
    }

    /* Register free functions. */
    for (Function * function = program->functions; function != NULL; function = function->next) {
        if (!registerFunction(table, function)) {
            logError(_logger, "Duplicate function declaration: '%s'.", function->name);
            status = FAILED;
        }
    }

    return status;
}

static CompilationStatus _checkStatement(SymbolTable * table, Statement * statement);

static CompilationStatus _checkStatements(SymbolTable * table, StatementList * list) {
    CompilationStatus status = SUCCEEDED;
    for (StatementList * node = list; node != NULL; node = node->next) {
        if (_checkStatement(table, node->statement) != SUCCEEDED) {
            status = FAILED;
        }
    }
    return status;
}

static CompilationStatus _checkStatement(SymbolTable * table, Statement * statement) {
    if (statement == NULL) {
        return SUCCEEDED;
    }
    switch (statement->kind) {
        case STATEMENT_VARIABLE_DECLARATION: {
            Type * declaredType = statement->variableDeclaration.type;
            if (statement->variableDeclaration.initializer != NULL) {
                Type * initType = resolveExpressionType(table, statement->variableDeclaration.initializer);
                if (initType != NULL && !isAssignable(table, initType, declaredType)) {
                    logError(_logger, "Type mismatch in declaration of '%s'.", statement->variableDeclaration.name);
                    return FAILED;
                }
            }
            if (!declareVariable(table, statement->variableDeclaration.name, declaredType)) {
                logError(_logger, "Variable '%s' already declared in an active scope.", statement->variableDeclaration.name);
                return FAILED;
            }
            return SUCCEEDED;
        }
        case STATEMENT_BLOCK: {
            pushScope(table);
            CompilationStatus status = _checkStatements(table, statement->block);
            popScope(table);
            return status;
        }
        case STATEMENT_IF: {
            CompilationStatus status = SUCCEEDED;
            if (isTypeError(resolveExpressionType(table, statement->ifStatement.condition))) {
                logError(_logger, "Invalid 'if' condition.");
                status = FAILED;
            }
            if (_checkStatement(table, statement->ifStatement.thenBranch) != SUCCEEDED) {
                status = FAILED;
            }
            if (statement->ifStatement.elseBranch != NULL) {
                if (_checkStatement(table, statement->ifStatement.elseBranch) != SUCCEEDED) {
                    status = FAILED;
                }
            }
            return status;
        }
        case STATEMENT_WHILE: {
            CompilationStatus status = SUCCEEDED;
            if (isTypeError(resolveExpressionType(table, statement->whileStatement.condition))) {
                logError(_logger, "Invalid 'while' condition.");
                status = FAILED;
            }
            if (_checkStatement(table, statement->whileStatement.body) != SUCCEEDED) {
                status = FAILED;
            }
            return status;
        }
        case STATEMENT_FOR: {
            pushScope(table);
            CompilationStatus status = SUCCEEDED;
            if (statement->forStatement.initializer != NULL) {
                if (_checkStatement(table, statement->forStatement.initializer) != SUCCEEDED) {
                    status = FAILED;
                }
            }
            if (isTypeError(resolveExpressionType(table, statement->forStatement.condition))) {
                logError(_logger, "Invalid 'for' condition.");
                status = FAILED;
            }
            if (isTypeError(resolveExpressionType(table, statement->forStatement.step))) {
                logError(_logger, "Invalid 'for' step expression.");
                status = FAILED;
            }
            if (_checkStatement(table, statement->forStatement.body) != SUCCEEDED) {
                status = FAILED;
            }
            popScope(table);
            return status;
        }
        case STATEMENT_RETURN: {
            Type * expected = getCurrentReturnType(table);
            Expression * value = statement->returnStatement.value;
            if (value != NULL) {
                Type * actual = resolveExpressionType(table, value);
                if (isTypeError(actual)) {
                    logError(_logger, "Invalid return expression.");
                    return FAILED;
                }
                if (expected == NULL || expected->kind == TYPEKIND_VOID) {
                    logError(_logger, "Return with a value in a void context.");
                    return FAILED;
                }
                if (actual != NULL && !isAssignable(table, actual, expected)) {
                    logError(_logger, "Return type mismatch.");
                    return FAILED;
                }
            } else {
                if (expected != NULL && expected->kind != TYPEKIND_VOID) {
                    logError(_logger, "Missing return value.");
                    return FAILED;
                }
            }
            return SUCCEEDED;
        }
        case STATEMENT_EXPRESSION: {
            Type * t = resolveExpressionType(table, statement->expressionStatement);
            if (isTypeError(t)) {
                logError(_logger, "Invalid expression statement.");
                return FAILED;
            }
            return SUCCEEDED;
        }
        default:
            return SUCCEEDED;
    }
}

static CompilationStatus _processBodies(SymbolTable * table, Program * program) {
    CompilationStatus status = SUCCEEDED;

    pushScope(table);
    setCurrentReturnType(table, &_voidReturnType);
    if (_checkStatements(table, program->mainBody) != SUCCEEDED) {
        status = FAILED;
    }
    setCurrentReturnType(table, NULL);
    popScope(table);

    for (Function * function = program->functions; function != NULL; function = function->next) {
        pushScope(table);
        setCurrentReturnType(table, function->returnType);
        for (Parameter * p = function->parameters; p != NULL; p = p->next) {
            if (!declareVariable(table, p->name, p->type)) {
                logError(_logger, "Duplicate parameter '%s' in function '%s'.", p->name, function->name);
                status = FAILED;
            }
        }
        if (_checkStatements(table, function->body) != SUCCEEDED) {
            status = FAILED;
        }
        setCurrentReturnType(table, NULL);
        popScope(table);
    }

    for (Class * class = program->classes; class != NULL; class = class->next) {
        setCurrentClass(table, class->name);
        for (Method * method = class->methods; method != NULL; method = method->next) {
            pushScope(table);
            setCurrentReturnType(table, method->returnType);
            for (Parameter * p = method->parameters; p != NULL; p = p->next) {
                if (!declareVariable(table, p->name, p->type)) {
                    logError(_logger, "Duplicate parameter '%s' in method '%s' of class '%s'.", p->name, method->name, class->name);
                    status = FAILED;
                }
            }
            if (_checkStatements(table, method->body) != SUCCEEDED) {
                status = FAILED;
            }
            setCurrentReturnType(table, NULL);
            popScope(table);
        }
        setCurrentClass(table, NULL);
    }

    return status;
}

CompilationStatus executeSemanticAnalysis(CompilerState * compilerState) {
    logDebugging(_logger, "Beginning semantic analysis...");
    compilerState->symbolTable = createSymbolTable();

    Program * program = compilerState->abstractSyntaxtTree;
    CompilationStatus status = _collectDeclarations(compilerState->symbolTable, program);

    if (status == SUCCEEDED) {
        if (_processBodies(compilerState->symbolTable, program) != SUCCEEDED) {
            status = FAILED;
        }
    }

    if (status != SUCCEEDED) {
        logError(_logger, "Semantic analysis failed.");
    } else {
        logDebugging(_logger, "Semantic analysis complete.");
    }

    destroySymbolTable(compilerState->symbolTable);
    compilerState->symbolTable = NULL;
    return status;
}
