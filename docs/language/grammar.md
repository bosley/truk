[← Back to Documentation Index](../start-here.md)

# TRUK Grammar

**Language Reference:** [Builtins](builtins.md) · [Maps](maps.md) · [Defer](defer.md) · [Imports](imports.md) · [Lambdas](lambdas.md) · [Privacy](privacy.md) · [Runtime](runtime.md)

---

## Notes

- **Privacy:** Identifiers starting with underscore (`_`) are private to their defining file. See [privacy.md](privacy.md) for details.
- **Enums:** Enum values are accessed via member syntax: `EnumName.VALUE`. The type checker disambiguates between struct member access and enum value access.
- **Cast Precedence:** The `as` cast operator has higher precedence than binary operators but lower than unary operators. This means:
  - `*ptr as i32` works as `(*ptr) as i32` ✓
  - `a * b as i32` requires `(a * b) as i32` for casting the result
  - `x as i32 + 5` works as `(x as i32) + 5` ✓

```
program         ::= declaration*

declaration     ::= fn_decl
                  | struct_decl
                  | enum_decl
                  | var_decl
                  | const_decl
                  | extern_decl
                  | shard_decl

extern_decl     ::= "extern" (extern_fn_decl | extern_struct_decl | extern_enum_decl | extern_var_decl)

shard_decl      ::= "shard" STRING ";"

extern_fn_decl  ::= "fn" IDENTIFIER "(" param_list? ")" (":" type)? ";"

extern_struct_decl ::= "struct" IDENTIFIER ("{" field_list? "}" | ";")

extern_enum_decl ::= "enum" IDENTIFIER ":" primitive_type ("{" enum_value_list? "}" | ";")

extern_var_decl ::= "var" IDENTIFIER type_annotation ";"

fn_decl         ::= "fn" IDENTIFIER "(" param_list? ")" (":" type)? block

param_list      ::= param ("," param)*

param           ::= IDENTIFIER type_annotation

struct_decl     ::= "struct" IDENTIFIER "{" field_list? "}"

field_list      ::= field ("," field)* ","?

field           ::= IDENTIFIER type_annotation

enum_decl       ::= "enum" IDENTIFIER ":" primitive_type "{" enum_value_list? "}"

enum_value_list ::= enum_value ("," enum_value)* ","?

enum_value      ::= IDENTIFIER ("=" INTEGER)?

var_decl        ::= "var" IDENTIFIER type_annotation ("=" expression)? ";"

const_decl      ::= "const" IDENTIFIER type_annotation "=" expression ";"

let_decl        ::= "let" identifier_list "=" expression ";"

identifier_list ::= IDENTIFIER ("," IDENTIFIER)*

type_annotation ::= ":" type

type            ::= primitive_type
                  | array_type
                  | pointer_type
                  | map_type
                  | tuple_type
                  | IDENTIFIER

primitive_type  ::= "i8" | "i16" | "i32" | "i64"
                  | "u8" | "u16" | "u32" | "u64"
                  | "f32" | "f64"
                  | "bool" | "void"

array_type      ::= "[" expression? "]" type

pointer_type    ::= "*" type

map_type        ::= "map" "[" type "," type "]"

tuple_type      ::= "(" type ("," type)+ ")"

block           ::= "{" statement* "}"

statement       ::= expression_stmt
                  | var_decl
                  | const_decl
                  | let_decl
                  | if_stmt
                  | while_stmt
                  | for_stmt
                  | match_stmt
                  | return_stmt
                  | break_stmt
                  | continue_stmt
                  | defer_stmt
                  | block

expression_stmt ::= expression ";"

if_stmt         ::= "if" expression block ("else" (if_stmt | block))?

while_stmt      ::= "while" expression block

for_stmt        ::= "for" expression? ";" expression? ";" expression? block

match_stmt      ::= "match" expression "{" match_case+ "}"

match_case      ::= ("case" expression | "_") "=>" (statement | block) ","

return_stmt     ::= "return" (expression ("," expression)*)? ";"

break_stmt      ::= "break" ";"

continue_stmt   ::= "continue" ";"

defer_stmt      ::= "defer" (expression ";" | block)

expression      ::= assignment

assignment      ::= logical_or (("=" | "+=" | "-=" | "*=" | "/=" | "%=") assignment)?

logical_or      ::= logical_and ("||" logical_and)*

logical_and     ::= equality ("&&" equality)*

equality        ::= comparison (("==" | "!=") comparison)*

comparison      ::= bitwise_or (("<" | "<=" | ">" | ">=") bitwise_or)*

bitwise_or      ::= bitwise_xor ("|" bitwise_xor)*

bitwise_xor     ::= bitwise_and ("^" bitwise_and)*

bitwise_and     ::= shift ("&" shift)*

shift           ::= additive (("<<" | ">>") additive)*

additive        ::= multiplicative (("+" | "-") multiplicative)*

multiplicative  ::= cast (("*" | "/" | "%") cast)*

cast            ::= unary ("as" type)*

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

IDENTIFIER      ::= [a-zA-Z_][a-zA-Z0-9_]* | "_"

INTEGER         ::= [0-9]+ | "0x"[0-9a-fA-F]+ | "0b"[01]+ | "0o"[0-7]+

FLOAT           ::= [0-9]+"."[0-9]+([eE][+-]?[0-9]+)?

STRING          ::= '"' ([^"\\\n] | "\\" .)* '"'
```
