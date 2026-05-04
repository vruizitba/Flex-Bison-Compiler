%top{

#include "../../support/type/CompilationStatus.h"
#include "../../support/type/FlexContext.h"
#include "../../support/type/TokenLabel.h"
#include "../syntactic-analysis/AbstractSyntaxTree.h"
#include "../syntactic-analysis/BisonParser.h"

}

%{

/**
 * @see https://westes.github.io/flex/manual/Definitions-Section.html#Definitions-Section
 */

extern CompilationStatus ArithmeticOperatorLexemeAction(TokenLabel label);
extern CompilationStatus EnterImportExpressionLexemeAction(FlexContext context);
extern CompilationStatus EnterMultilineCommentLexemeAction(FlexContext context);
extern CompilationStatus EOFLexemeAction();
extern CompilationStatus IgnoredLexemeAction();
extern CompilationStatus IntegerLexemeAction();
extern CompilationStatus LeaveImportExpressionLexemeAction();
extern CompilationStatus LeaveMultilineCommentLexemeAction();
extern CompilationStatus ParenthesisLexemeAction(TokenLabel label);
extern CompilationStatus SubexpressionLexemeAction();
extern CompilationStatus FloatLexemeAction();
extern CompilationStatus IdentifierLexemeAction();
extern CompilationStatus KeywordLexemeAction(TokenLabel label);
extern CompilationStatus UnknownLexemeAction();

%}

/**
 * Flex contexts (a.k.a. start conditions).
 *
 * @see https://westes.github.io/flex/manual/Start-Conditions.html
 */
%x IMPORT_EXPRESSION
%x MULTILINE_COMMENT

/**
 * Reusable patterns.
 *
 * @see https://westes.github.io/flex/manual/Matching.html
 * @see https://westes.github.io/flex/manual/Patterns.html
 */
whitespace			[[:space:]]+

%%

%{

/**
 * @see https://westes.github.io/flex/manual/Rules-Section.html#Rules-Section
 */

%}

"{"									{ return EnterImportExpressionLexemeAction(IMPORT_EXPRESSION); }
<IMPORT_EXPRESSION>[^{}]+			{ return SubexpressionLexemeAction(); }
<IMPORT_EXPRESSION>"}"				{ return LeaveImportExpressionLexemeAction(); }
<IMPORT_EXPRESSION>.				{ return UnknownLexemeAction(); }

"//"[^\n]*							{ return IgnoredLexemeAction(); }
"/*"								{ return EnterMultilineCommentLexemeAction(MULTILINE_COMMENT); }
<MULTILINE_COMMENT>"*/"				{ return LeaveMultilineCommentLexemeAction(); }
<MULTILINE_COMMENT>[^*]+			{ return IgnoredLexemeAction(); }
<MULTILINE_COMMENT>.				{ return IgnoredLexemeAction(); }

"-"									{ return ArithmeticOperatorLexemeAction(SUB); }
"*"									{ return ArithmeticOperatorLexemeAction(MUL); }
"/"									{ return ArithmeticOperatorLexemeAction(DIV); }
"+"									{ return ArithmeticOperatorLexemeAction(ADD); }

"("									{ return ParenthesisLexemeAction(OPEN_PARENTHESIS); }
")"									{ return ParenthesisLexemeAction(CLOSE_PARENTHESIS); }

"int"								{ return KeywordLexemeAction(TYPE_INT); }
"bool"								{ return KeywordLexemeAction(TYPE_BOOL); }
"string"							{ return KeywordLexemeAction(TYPE_STRING); }
"char"								{ return KeywordLexemeAction(TYPE_CHAR); }
"float"								{ return KeywordLexemeAction(TYPE_FLOAT); }
"double"							{ return KeywordLexemeAction(TYPE_DOUBLE); }
"void"								{ return KeywordLexemeAction(TYPE_VOID); }
"signed"							{ return KeywordLexemeAction(SIGNED); }
"unsigned"							{ return KeywordLexemeAction(UNSIGNED); }
"short"								{ return KeywordLexemeAction(SHORT); }
"long"								{ return KeywordLexemeAction(LONG); }
"true"								{ return KeywordLexemeAction(BOOLEAN_TRUE); }
"false"								{ return KeywordLexemeAction(BOOLEAN_FALSE); }
"class"								{ return KeywordLexemeAction(CLASS); }
"extends"							{ return KeywordLexemeAction(EXTENDS); }
"public"							{ return KeywordLexemeAction(PUBLIC); }
"private"							{ return KeywordLexemeAction(PRIVATE); }
"protected"							{ return KeywordLexemeAction(PROTECTED); }
"static"							{ return KeywordLexemeAction(STATIC); }
"this"								{ return KeywordLexemeAction(THIS); }
"new"								{ return KeywordLexemeAction(NEW); }
"if"								{ return KeywordLexemeAction(IF); }
"else"								{ return KeywordLexemeAction(ELSE); }
"while"								{ return KeywordLexemeAction(WHILE); }
"for"								{ return KeywordLexemeAction(FOR); }
"return"							{ return KeywordLexemeAction(RETURN); }
"main"								{ return KeywordLexemeAction(MAIN); }
"function"							{ return KeywordLexemeAction(FUNCTION); }

[[:alpha:]_][[:alnum:]_]*			{ return IdentifierLexemeAction(); }

[[:digit:]]+\.[[:digit:]]+			{ return FloatLexemeAction(); }
[[:digit:]]+						{ return IntegerLexemeAction(); }

{whitespace}						{ return IgnoredLexemeAction(); }
.									{ return UnknownLexemeAction(); }

<<EOF>>								{ return EOFLexemeAction(); }

%%

/**
 * @see https://westes.github.io/flex/manual/User-Code-Section.html#User-Code-Section
 */

#include "FlexActions.h"
#include "FlexExport.h"
