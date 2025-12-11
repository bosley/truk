# TRUK Grammar

```
program         ::= declaration*

declaration     ::= fn_decl
                  | struct_decl
                  | var_decl
                  | const_decl

fn_decl         ::= "fn" IDENTIFIER "(" param_list? ")" (":" type)? block

param_list      ::= param ("," param)*

param           ::= IDENTIFIER type_annotation

struct_decl     ::= "struct" IDENTIFIER "{" field_list? "}"

field_list      ::= field ("," field)* ","?

field           ::= IDENTIFIER type_annotation

var_decl        ::= "var" IDENTIFIER type_annotation ("=" expression)? ";"

const_decl      ::= "const" IDENTIFIER type_annotation "=" expression ";"

type_annotation ::= ":" type

type            ::= primitive_type
                  | array_type
                  | pointer_type
                  | IDENTIFIER

primitive_type  ::= "i8" | "i16" | "i32" | "i64"
                  | "u8" | "u16" | "u32" | "u64"
                  | "f32" | "f64"
                  | "bool" | "void"

array_type      ::= "[" expression? "]" type

pointer_type    ::= "*" type

block           ::= "{" statement* "}"

statement       ::= expression_stmt
                  | var_decl
                  | const_decl
                  | if_stmt
                  | while_stmt
                  | for_stmt
                  | return_stmt
                  | break_stmt
                  | continue_stmt
                  | block

expression_stmt ::= expression ";"

if_stmt         ::= "if" expression block ("else" (if_stmt | block))?

while_stmt      ::= "while" expression block

for_stmt        ::= "for" expression? ";" expression? ";" expression? block

return_stmt     ::= "return" expression? ";"

break_stmt      ::= "break" ";"

continue_stmt   ::= "continue" ";"

expression      ::= assignment

assignment      ::= cast (("=" | "+=" | "-=" | "*=" | "/=" | "%=") assignment)?

cast            ::= logical_or ("as" type)*

logical_or      ::= logical_and ("||" logical_and)*

logical_and     ::= equality ("&&" equality)*

equality        ::= comparison (("==" | "!=") comparison)*

comparison      ::= bitwise_or (("<" | "<=" | ">" | ">=") bitwise_or)*

bitwise_or      ::= bitwise_xor ("|" bitwise_xor)*

bitwise_xor     ::= bitwise_and ("^" bitwise_and)*

bitwise_and     ::= shift ("&" shift)*

shift           ::= additive (("<<" | ">>") additive)*

additive        ::= multiplicative (("+" | "-") multiplicative)*

multiplicative  ::= unary (("*" | "/" | "%") unary)*

unary           ::= ("!" | "-" | "~" | "&" | "*") unary
                  | postfix

postfix         ::= primary (call | index | member)*

call            ::= "(" argument_list? ")"

argument_list   ::= expression ("," expression)*

index           ::= "[" expression "]"

member          ::= "." IDENTIFIER

primary         ::= INTEGER
                  | FLOAT
                  | STRING
                  | "true" | "false"
                  | "nil"
                  | IDENTIFIER
                  | "(" expression ")"
                  | array_literal
                  | struct_literal
                  | type_param

type_param      ::= "@" type

array_literal   ::= "[" (expression ("," expression)* ","?)? "]"

struct_literal  ::= IDENTIFIER "{" (field_init ("," field_init)* ","?)? "}"

field_init      ::= IDENTIFIER ":" expression

IDENTIFIER      ::= [a-zA-Z_][a-zA-Z0-9_]*

INTEGER         ::= [0-9]+ | "0x"[0-9a-fA-F]+ | "0b"[01]+ | "0o"[0-7]+

FLOAT           ::= [0-9]+"."[0-9]+([eE][+-]?[0-9]+)?

STRING          ::= '"' ([^"\\\n] | "\\" .)* '"'
```
