grammar Generic;

source
    : sourceElements? EOF
    ;

sourceElements
    : sourceElement+
    ;

sourceElement
    : statement
    ;

statement
    : block
    | SingleLineComment
    | MultiLineComment
    | expressionStatement
    | ifStatement
    ;

statementList
    : statement+
    ;

block
    : '{' statementList? '}'
    ;

comment
    : '//'+ eos
    ;

ifStatement
    : If ifCondition statement (elseStatement)?
    ;

ifCondition
    : '(' expressionSequence ')'
    ;

elseStatement
    : Else statement
    ;

expressionStatement
    : expressionSequence eos
    ;

/* Functions, procedures and subroutines are written and called the same way. */
functionCall
    : functionName '(' functionCallArguments ')'
    ;

functionName
    : NAME
    ;

functionCallArguments
    :
    | singleExpression (',' singleExpression)*
    ;

expressionSequence
    : singleExpression (',' singleExpression)*
    ;

singleExpression
    : singleExpression fieldAccessOperator singleExpression
    | singleExpression fieldAccessOperator vectorComponent
    | '++' singleExpression
    | '--' singleExpression
    | unaryOperator singleExpression
    | singleExpression arithmeticOperator singleExpression
    | singleExpression comparisonOperator singleExpression
    | singleExpression logicalOperator singleExpression
    | singleExpression '?' singleExpression ':' singleExpression
    | singleExpression assignmentOperator singleExpression
    | Self
    | vector
    | dsgVar
    | functionCall
    | actorReference
    | literal
    | field
    | '(' expressionSequence ')'
    ;

fieldAccessOperator
    : '.'
    ;

unaryOperator
    : '+'
    | '-'
    | '!'
    ;

arithmeticOperator
    : ('*' | '/' | '%')
    | ('+' | '-')
    ;

comparisonOperator
    : ('<' | '>' | '<=' | '>=')
    | ('==' | '!=')
    ;

logicalOperator
    : '&&'
    | '||'
    ;

assignmentOperator
    : '='
    | '+='
    | '-='
    | '*='
    | '/='
    ;

vector
    : vectorName '(' expressionSequence ')'
    ;

vectorName
    : 'new'? ('Vector' | 'Vector3' | 'Vector3f')
    ;

vectorComponent
    : ('x' | 'y' | 'z')
    | ('X' | 'Y' | 'Z')
    ;

dsgVar
    : 'dsgVar' '(' numericLiteral ')'
    ;

dsgVarIdentifier
    : 'dsgVar'
    | 'dsg'
    | 'dv'
    ;

actorReference
    : NAME
    ;

literal
    : NullLiteral
    | StringLiteral
    | numericLiteral
    ;

numericLiteral
    : DecimalLiteral
    | HexadecimalLiteral
    ;

NullLiteral
    : ('null' | 'NULL')
    ;

BooleanLiteral
    : 'true'
    | 'false'
    ;

DecimalLiteral
    : DecimalIntegerLiteral '.' DecimalLiteral* ExponentPart?
    | '.' DecimalDigit+ ExponentPart?
    | '.' DecimalDigit+ ExponentPart?
    | DecimalIntegerLiteral ExponentPart?
    ;

HexadecimalLiteral
 : '0' [xX] HexadecimalDigit+
 ;

StringLiteral
    : '"' DoubleStringCharacter* '"'
    | '\'' SingleStringCharacter* '\''
    ;

MultiLineComment
    : '/*' .*? '*/'
    ;

SingleLineComment
    : '//' ~[\r\n\u2028\u2029]*
    ;

reservedWord
    : keyword
    | field
    | (NullLiteral | BooleanLiteral)
    ;

keyword
    : If
    | Else
    | Self
    ;

field
    : Position
    | Orientation
    | Speed
    | NormSpeed
    | AbsoluteAxis
    | PrevComportIntell
    | PrevComportReflex
    | ShadowScale
    | PadGlobalVector
    | PadHorizontalAxis
    | PadVerticalAxis
    | PadAnalogForce
    | PadTrueAnalogForce
    | PadRotationangle
    | PadSector
    | SystemDate
    | SystemTime
    ;

/* Keyword */
If      : 'if';
Else    : 'else';
Self    : ('self' | 'this');

/* Field */
Position            : 'Position';
Orientation         : 'Orientation';
Speed               : 'Speed';
NormSpeed           : 'NormSpeed';
AbsoluteAxis        : 'AbsoluteAxis' ('X' | 'Y' | 'Z');
PrevComportIntell   : 'PrevComportIntell';
PrevComportReflex   : 'PrevComportReflex';
ShadowScale         : 'ShadowScale' ('X' | 'Y');
PadGlobalVector     : 'PadGlobalVector';
PadHorizontalAxis   : 'PadHorizontalAxis';
PadVerticalAxis     : 'PadVerticalAxis';
PadAnalogForce      : 'PadAnalogForce';
PadTrueAnalogForce  : 'PadTrueAnalogForce';
PadRotationangle    : 'PadRotationAngle';
PadSector           : 'PadSector';
SystemDate          : 'SystemDate';
SystemTime          : 'SystemTime';


/* Other symbols */
SemiColon : ';';

eos
    : SemiColon
    | EOF
    ;

LineTerminator
    : [\r\n\u2028\u2029] -> channel(HIDDEN)
    ;

fragment DecimalDigit
    : [0-9]
    ;

fragment HexadecimalDigit
    : [0-9a-fA-F]
    ;

fragment DecimalIntegerLiteral
    : '0'
    | [1-9] DecimalDigit*
    ;

fragment ExponentPart
    : [eE] [+-]? DecimalDigit+
    ;

fragment DoubleStringCharacter
    : ~["\\\r\n]
    | '\\' EscapeSequence
    | LineContinuation
    ;

fragment SingleStringCharacter
    : ~['\\\r\n]
    | '\\' EscapeSequence
    | LineContinuation
    ;

fragment EscapeSequence
    : CharacterEscapeSequence
    | '0'
    | HexEscapeSequence
    | UnicodeEscapeSequence
    ;

fragment CharacterEscapeSequence
    : SingleEscapeCharacter
    | NonEscapeCharacter
    ;

fragment HexEscapeSequence
    : 'x' HexadecimalDigit HexadecimalDigit
    ;

fragment UnicodeEscapeSequence
    : 'u' HexadecimalDigit HexadecimalDigit HexadecimalDigit HexadecimalDigit
    ;

fragment SingleEscapeCharacter
    : ['"\\bfnrtv]
    ;

fragment NonEscapeCharacter
    : ~['"\\bfnrtv0-9xu\r\n]
    ;

fragment EscapeCharacter
    : SingleEscapeCharacter
    | DecimalDigit
    | [xu]
    ;

fragment LineContinuation
    : '\\' LineTerminatorSequence
    ;

fragment LineTerminatorSequence
    : '\r\n'
    | LineTerminator
    ;

NAME: [a-zA-Z_][a-zA-Z0-9_]+;
WhiteSpace : (' ' | '\t') -> skip;
