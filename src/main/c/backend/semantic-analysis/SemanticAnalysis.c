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

CompilationStatus executeSemanticAnalysis(CompilerState * compilerState) {
    logDebugging(_logger, "Beginning semantic analysis...");
    compilerState->symbolTable = createSymbolTable();
    logDebugging(_logger, "Semantic analysis complete.");
    destroySymbolTable(compilerState->symbolTable);
    compilerState->symbolTable = NULL;
    return SUCCEEDED;
}
