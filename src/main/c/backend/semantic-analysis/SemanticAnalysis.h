#ifndef SEMANTIC_ANALYSIS_HEADER
#define SEMANTIC_ANALYSIS_HEADER

#include "../../support/type/CompilationStatus.h"
#include "../../support/type/CompilerState.h"
#include "../../support/type/ModuleDestructor.h"

ModuleDestructor initializeSemanticAnalysisModule();
CompilationStatus executeSemanticAnalysis(CompilerState * compilerState);

#endif
