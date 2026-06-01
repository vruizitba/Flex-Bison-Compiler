#ifndef COMPILER_STATE_HEADER
#define COMPILER_STATE_HEADER

#include "../../backend/semantic-analysis/SymbolTable.h"

/**
 * The global state of the compiler. Transports data structures across
 * the different phases of compilation.
 */
typedef struct {
	/**
	 * The root node of the AST.
	 */
	void * abstractSyntaxtTree;

	/**
	 * Symbol table populated during semantic analysis.
	 */
	SymbolTable * symbolTable;

	// TODO: Add an stack to handle nested scopes.
	// TODO: Add more configuration.
	// TODO: Add whatever you need.
	// TODO: ...
} CompilerState;

#endif
