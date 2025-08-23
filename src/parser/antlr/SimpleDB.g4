grammar SimpleDB;

/*
 * =============================================================================
 * PARSER RULES
 * =============================================================================
 * Parser rules define the grammatical structure of the language.
 * They are written in lowerCamelCase.
 */

// The entry point for any command. It can be one of the following statements,
// optionally followed by a semicolon, and then the End-Of-File marker.
query
    : (createStatement | dropStatement | insertStatement | showStatement | selectStatement) SEMICOLON? EOF
    ;

// --- SELECT Statement ---
selectStatement
    : SELECT projection FROM tableName=IDENTIFIER
    ;

projection
    : ASTERISK
    | columnList
    ;

columnList
    : IDENTIFIER (COMMA IDENTIFIER)*
    ;

// --- CREATE TABLE Statement ---
createStatement
    : CREATE TABLE tableName=IDENTIFIER LPAREN columnDefinitions RPAREN
    ;

columnDefinitions
    : columnDef (COMMA columnDef)*
    ;

columnDef
    : columnName=IDENTIFIER columnType=dataType
    ;

dataType
    : INT_TYPE
    | TEXT_TYPE
    ;

// --- DROP TABLE Statement ---
dropStatement
    : DROP TABLE tableName=IDENTIFIER
    ;

// --- INSERT Statement ---
insertStatement
    : INSERT INTO tableName=IDENTIFIER (LPAREN columnList RPAREN)? VALUES LPAREN valueList RPAREN
    ;

valueList
    : value (COMMA value)*
    ;

// TODO: Expand 'value' to a full 'expression' rule to handle things like
//       negative numbers (-5), arithmetic (5 + 2), and functions.
//       For now, it only supports simple literals.
value
    : STRING_LITERAL
    | INTEGER_LITERAL
    ;

// --- SHOW TABLES Statement ---
showStatement
    : SHOW TABLES
    ;


/*
 * =============================================================================
 * LEXER RULES
 * =============================================================================
 * Lexer rules define the tokens (the "words") of the language.
 * They are written in UPPER_SNAKE_CASE.
 *
 * IMPORTANT: More specific rules (keywords) must be defined BEFORE
 * more general rules (IDENTIFIER).
 */

// --- Keywords (Case-Insensitive) ---
SELECT : S E L E C T;
FROM   : F R O M;
CREATE : C R E A T E;
TABLE  : T A B L E;
DROP   : D R O P;
INSERT : I N S E R T;
INTO   : I N T O;
VALUES : V A L U E S;
SHOW   : S H O W;
TABLES : T A B L E S;
INT_TYPE : I N T;
TEXT_TYPE: T E X T;

// --- Literals (Case-Sensitive) ---
STRING_LITERAL
    : '\'' ( ~['] | '\'\'')* '\'' // Handles 'a string with '' escaped quotes'
    ;

INTEGER_LITERAL
    : [0-9]+
    ;

// --- Identifier (Case-Sensitive) ---
// This MUST be defined AFTER all the keywords.
IDENTIFIER
    : [a-zA-Z_] [a-zA-Z_0-9]* | '"' ( ~["] | '""')+ '"'
    ;

// --- Punctuation and Symbols ---
COMMA    : ',';
ASTERISK : '*';
SEMICOLON: ';';
LPAREN   : '(';
RPAREN   : ')';

// --- Whitespace (Skipped) ---
// This tells the lexer to match whitespace but not produce a token for the parser.
WS
    : [ \t\r\n]+ -> skip
    ;

/*
 * =============================================================================
 * FRAGMENT RULES
 * =============================================================================
 * Fragments are helper rules for the lexer. They are not tokens themselves
 * but are used to build other lexer rules. They are used here to define
 * case-insensitivity for our keywords.
 */
fragment A: [aA];
fragment B: [bB];
fragment C: [cC];
fragment D: [dD];
fragment E: [eE];
fragment F: [fF];
fragment G: [gG];
fragment H: [hH];
fragment I: [iI];
fragment J: [jJ];
fragment K: [kK];
fragment L: [lL];
fragment M: [mM];
fragment N: [nN];
fragment O: [oO];
fragment P: [pP];
fragment Q: [qQ];
fragment R: [rR];
fragment S: [sS];
fragment T: [tT];
fragment U: [uU];
fragment V: [vV];
fragment W: [wW];
fragment X: [xX];
fragment Y: [yY];
fragment Z: [zZ];
