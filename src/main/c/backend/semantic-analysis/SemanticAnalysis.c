#include "SemanticAnalysis.h"
#include "SymbolTable.h"
#include "../../support/logging/Logger.h"

static Logger * _logger = NULL;

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
            logError(_logger, "Duplicate method signature in class '%s'.", class->name);
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
                Type * initType = typeOf(table, statement->variableDeclaration.initializer);
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
        case STATEMENT_WHILE:
            return _checkStatement(table, statement->whileStatement.body);
        case STATEMENT_FOR: {
            pushScope(table);
            CompilationStatus status = SUCCEEDED;
            if (statement->forStatement.initializer != NULL) {
                if (_checkStatement(table, statement->forStatement.initializer) != SUCCEEDED) {
                    status = FAILED;
                }
            }
            if (_checkStatement(table, statement->forStatement.body) != SUCCEEDED) {
                status = FAILED;
            }
            popScope(table);
            return status;
        }
        case STATEMENT_EXPRESSION: {
            Type * t = typeOf(table, statement->expressionStatement);
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
    if (_checkStatements(table, program->mainBody) != SUCCEEDED) {
        status = FAILED;
    }
    popScope(table);

    for (Function * function = program->functions; function != NULL; function = function->next) {
        pushScope(table);
        for (Parameter * p = function->parameters; p != NULL; p = p->next) {
            declareVariable(table, p->name, p->type);
        }
        if (_checkStatements(table, function->body) != SUCCEEDED) {
            status = FAILED;
        }
        popScope(table);
    }

    for (Class * class = program->classes; class != NULL; class = class->next) {
        setCurrentClass(table, class->name);
        for (Method * method = class->methods; method != NULL; method = method->next) {
            pushScope(table);
            for (Parameter * p = method->parameters; p != NULL; p = p->next) {
                declareVariable(table, p->name, p->type);
            }
            if (_checkStatements(table, method->body) != SUCCEEDED) {
                status = FAILED;
            }
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
