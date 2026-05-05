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

extern CompilationStatus OperatorLexemeAction(TokenLabel label);
//extern CompilationStatus EnterImportExpressionLexemeAction(FlexContext context);
extern CompilationStatus EnterMultilineCommentLexemeAction(FlexContext context);
extern CompilationStatus EOFLexemeAction();
extern CompilationStatus IgnoredLexemeAction();
extern CompilationStatus IntegerLexemeAction();
//extern CompilationStatus LeaveImportExpressionLexemeAction(); 
extern CompilationStatus LeaveMultilineCommentLexemeAction();
extern CompilationStatus PunctuationLexemeAction(TokenLabel label);
//extern CompilationStatus SubexpressionLexemeAction(); 
extern CompilationStatus CharLexemeAction();
extern CompilationStatus EnterStringLiteralLexemeAction(FlexContext context);
extern CompilationStatus FloatLexemeAction();
extern CompilationStatus IdentifierLexemeAction();
extern CompilationStatus KeywordLexemeAction(TokenLabel label);
extern CompilationStatus LeaveStringLiteralLexemeAction();
extern CompilationStatus StringCharLexemeAction();
extern CompilationStatus StringEscapeLexemeAction();
extern CompilationStatus UnknownLexemeAction();

%}

/**
 * Flex contexts (a.k.a. start conditions).
 *
 * @see https://westes.github.io/flex/manual/Start-Conditions.html
 */
/* %x IMPORT_EXPRESSION */
%x MULTILINE_COMMENT
%x STRING_LITERAL

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


"//"[^\n]*							{ return IgnoredLexemeAction(); }
"/*"								{ return EnterMultilineCommentLexemeAction(MULTILINE_COMMENT); }
<MULTILINE_COMMENT>"*/"				{ return LeaveMultilineCommentLexemeAction(); }
<MULTILINE_COMMENT>[^*]+			{ return IgnoredLexemeAction(); }
<MULTILINE_COMMENT>.				{ return IgnoredLexemeAction(); }

"++"								{ return OperatorLexemeAction(INCREMENT); }
"--"								{ return OperatorLexemeAction(DECREMENT); }
"+="								{ return OperatorLexemeAction(PLUS_ASSIGN); }
"-="								{ return OperatorLexemeAction(MINUS_ASSIGN); }
"*="								{ return OperatorLexemeAction(ASTERISK_ASSIGN); }
"/="								{ return OperatorLexemeAction(DIVIDE_ASSIGN); }
"%="								{ return OperatorLexemeAction(MODULO_ASSIGN); }
"+"									{ return OperatorLexemeAction(ADD); }
"-"									{ return OperatorLexemeAction(SUBTRACT); }
"*"									{ return OperatorLexemeAction(ASTERISK); }
"/"									{ return OperatorLexemeAction(DIVIDE); }
"%"									{ return OperatorLexemeAction(MODULO); }
"=="								{ return OperatorLexemeAction(EQUAL); }
"!="								{ return OperatorLexemeAction(NOT_EQUAL); }
"<="								{ return OperatorLexemeAction(LESS_EQUAL); }
">="								{ return OperatorLexemeAction(GREATER_EQUAL); }
"<"									{ return OperatorLexemeAction(LESS); }
">"									{ return OperatorLexemeAction(GREATER); }
"&&"								{ return OperatorLexemeAction(AND); }
"||"								{ return OperatorLexemeAction(OR); }
"!"									{ return OperatorLexemeAction(NOT); }
"->"								{ return OperatorLexemeAction(ARROW); }
"."									{ return OperatorLexemeAction(DOT); }
"&"									{ return OperatorLexemeAction(AMPERSAND); }
"="									{ return OperatorLexemeAction(ASSIGN); }

"("									{ return PunctuationLexemeAction(OPEN_PARENTHESIS); }
")"									{ return PunctuationLexemeAction(CLOSE_PARENTHESIS); }
"{"									{ return PunctuationLexemeAction(OPEN_BRACE); }
"}"									{ return PunctuationLexemeAction(CLOSE_BRACE); }
"["									{ return PunctuationLexemeAction(OPEN_BRACKET); }
"]"									{ return PunctuationLexemeAction(CLOSE_BRACKET); }
","									{ return PunctuationLexemeAction(COMMA); }
";"									{ return PunctuationLexemeAction(SEMICOLON); }

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

'(\\.|[^'\\])'						{ return CharLexemeAction(); }
"\""								{ return EnterStringLiteralLexemeAction(STRING_LITERAL); }
<STRING_LITERAL>"\""				{ return LeaveStringLiteralLexemeAction(); }
<STRING_LITERAL>\\.					{ return StringEscapeLexemeAction(); }
<STRING_LITERAL>[^"\\]+				{ return StringCharLexemeAction(); }

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
