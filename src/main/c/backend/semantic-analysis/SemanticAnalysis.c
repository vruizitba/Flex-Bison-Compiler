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

    /* Register all classes. */
    for (Class * class = program->classes; class != NULL; class = class->next) {
        if (!registerClass(table, class)) {
            logError(_logger, "Duplicate class declaration: '%s'.", class->name);
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

CompilationStatus executeSemanticAnalysis(CompilerState * compilerState) {
    logDebugging(_logger, "Beginning semantic analysis...");
    compilerState->symbolTable = createSymbolTable();

    CompilationStatus status = _collectDeclarations(compilerState->symbolTable, compilerState->abstractSyntaxtTree);

    if (status != SUCCEEDED) {
        logError(_logger, "Semantic analysis failed.");
    } else {
        logDebugging(_logger, "Semantic analysis complete.");
    }

    destroySymbolTable(compilerState->symbolTable);
    compilerState->symbolTable = NULL;
    return status;
}
