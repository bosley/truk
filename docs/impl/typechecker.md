# Type Checker Architecture

## Overview

The type checker implements a multi-stage validation architecture that processes the Abstract Syntax Tree (AST) through distinct phases. The implementation uses three specialized visitor classes and inline control flow checking to provide comprehensive validation. Each stage focuses on a specific concern, building upon the results of previous stages.

## High-Level Process Flow

```mermaid
flowchart TD
    Start[AST Root Node] --> Init[Initialize Type Checker]
    Init --> Stage1[Stage 1: Symbol Collection]
    Stage1 --> Stage2[Stage 2: Lambda Capture Validation]
    Stage2 --> Stage3[Stage 3: Type Checking]
    Stage3 --> Aggregate[Aggregate All Errors]
    
    Aggregate --> HasErrors{Has Errors?}
    HasErrors -->|Yes| Report[Report All Errors]
    HasErrors -->|No| Success[Validation Complete]
    
    Report --> End[End]
    Success --> End
```

## Multi-Stage Validation Sequence

```mermaid
sequenceDiagram
    participant Client as Compiler
    participant TC as Type Checker
    participant SC as Symbol Collector
    participant LV as Lambda Validator
    
    Client->>TC: check(AST root)
    activate TC
    
    Note over TC: Stage 1: Symbol Collection
    TC->>SC: collect_symbols(root)
    activate SC
    SC->>SC: Build scope hierarchy
    SC->>SC: Register symbols
    SC->>SC: Track lambdas
    SC-->>TC: symbol_collection_result
    deactivate SC
    
    Note over TC: Stage 2: Lambda Capture
    TC->>LV: validate_lambda_captures(root, symbols)
    activate LV
    LV->>LV: Visit each lambda
    LV->>LV: Check identifier references
    LV->>LV: Validate scope boundaries
    LV-->>TC: lambda_capture_result
    deactivate LV
    
    Note over TC: Stage 3: Type Checking
    TC->>TC: Visit AST nodes (self)
    TC->>TC: Validate type compatibility
    TC->>TC: Check assignments
    TC->>TC: Validate operations
    TC->>TC: Inline control flow checks
    
    Note over TC: Aggregate all errors
    TC->>TC: Collect errors from all stages
    
    TC-->>Client: validation complete
    deactivate TC
```

## Symbol Collection Process

```mermaid
flowchart TD
    Start[Start Symbol Collection] --> InitScope[Initialize Global Scope]
    InitScope --> Visit[Visit AST Node]
    
    Visit --> NodeType{Node Type?}
    
    NodeType -->|Function| RegFunc[Register Function Symbol]
    RegFunc --> FuncScope[Create Function Scope]
    FuncScope --> RegParams[Register Parameters]
    RegParams --> VisitBody[Visit Function Body]
    
    NodeType -->|Lambda| RegLambda[Track Lambda]
    RegLambda --> LambdaScope[Create Lambda Scope]
    LambdaScope --> LambdaParams[Register Lambda Parameters]
    LambdaParams --> VisitLambdaBody[Visit Lambda Body]
    
    NodeType -->|Variable| DetermineScope{Scope Kind?}
    DetermineScope -->|Global| MarkGlobal[Mark as GLOBAL]
    DetermineScope -->|Function| MarkFunc[Mark as FUNCTION_LOCAL]
    DetermineScope -->|Lambda| MarkLambda[Mark as LAMBDA_LOCAL]
    MarkGlobal --> RegVar[Register Variable]
    MarkFunc --> RegVar
    MarkLambda --> RegVar
    
    NodeType -->|Block| BlockScope[Create Block Scope]
    BlockScope --> VisitStmts[Visit Statements]
    
    NodeType -->|Other| VisitChildren[Visit Child Nodes]
    
    VisitBody --> PopScope1[Pop Function Scope]
    VisitLambdaBody --> PopScope2[Pop Lambda Scope]
    VisitStmts --> PopScope3[Pop Block Scope]
    VisitChildren --> Continue
    RegVar --> Continue
    PopScope1 --> Continue
    PopScope2 --> Continue
    PopScope3 --> Continue
    
    Continue --> More{More Nodes?}
    More -->|Yes| Visit
    More -->|No| BuildResult[Build Result Structure]
    BuildResult --> End[Return Symbol Collection Result]
```

## Lambda Capture Validation Process

```mermaid
flowchart TD
    Start[Start Lambda Validation] --> GetSymbols[Receive Symbol Collection Result]
    GetSymbols --> VisitNode[Visit AST Node]
    
    VisitNode --> IsLambda{Is Lambda Node?}
    IsLambda -->|Yes| EnterLambda[Set Current Lambda Context]
    IsLambda -->|No| CheckId{Is Identifier?}
    
    EnterLambda --> SetScope[Set Lambda Scope]
    SetScope --> VisitLambdaBody[Visit Lambda Body]
    VisitLambdaBody --> ExitLambda[Clear Lambda Context]
    ExitLambda --> NextNode
    
    CheckId -->|Yes| InLambda{Inside Lambda?}
    CheckId -->|No| VisitChildren[Visit Child Nodes]
    
    InLambda -->|No| NextNode[Continue to Next Node]
    InLambda -->|Yes| FindSymbol[Search for Symbol in Scope Hierarchy]
    
    FindSymbol --> SymbolFound{Symbol Found?}
    SymbolFound -->|No| NextNode
    SymbolFound -->|Yes| CheckScopeKind{Scope Kind?}
    
    CheckScopeKind -->|Global| AllowAccess[Allow: Global Symbol]
    CheckScopeKind -->|Other| CheckLocation{Symbol Location?}
    
    CheckLocation -->|In Lambda or Below| AllowAccess2[Allow: Lambda-Local]
    CheckLocation -->|Between Global and Lambda| RejectCapture[Reject: Captured Variable]
    
    RejectCapture --> AddError[Add Capture Error]
    AddError --> NextNode
    AllowAccess --> NextNode
    AllowAccess2 --> NextNode
    VisitChildren --> NextNode
    
    NextNode --> MoreNodes{More Nodes?}
    MoreNodes -->|Yes| VisitNode
    MoreNodes -->|No| BuildErrors[Build Error List]
    BuildErrors --> End[Return Lambda Capture Result]
```

## Scope Hierarchy Structure

```mermaid
graph TD
    Global[Global Scope] --> Func1[Function: main]
    Global --> Func2[Function: helper]
    Global --> Struct1[Struct: Point]
    
    Func1 --> Block1[Block Scope]
    Block1 --> Var1[Variable: total]
    Block1 --> Var2[Variable: count]
    Block1 --> Lambda1[Lambda Scope]
    
    Lambda1 --> Param1[Parameter: x]
    Lambda1 --> LambdaBlock[Lambda Body Block]
    LambdaBlock --> LocalVar[Variable: result]
    
    Func2 --> Block2[Block Scope]
    Block2 --> Var3[Variable: temp]
    
    style Global fill:#e1f5ff
    style Func1 fill:#fff4e1
    style Func2 fill:#fff4e1
    style Lambda1 fill:#ffe1f5
    style Block1 fill:#f0f0f0
    style Block2 fill:#f0f0f0
    style LambdaBlock fill:#f0f0f0
```

## Symbol Scope Classification

```mermaid
flowchart LR
    Symbols[All Symbols] --> Global[GLOBAL Symbols]
    Symbols --> FuncLocal[FUNCTION_LOCAL Symbols]
    Symbols --> LambdaLocal[LAMBDA_LOCAL Symbols]
    Symbols --> Params[PARAMETER Symbols]
    
    Global --> GlobEx[Functions, Structs, Global Variables]
    FuncLocal --> FuncEx[Variables in Function Scope]
    LambdaLocal --> LambdaEx[Variables in Lambda Scope]
    Params --> ParamEx[Function/Lambda Parameters]
    
    style Global fill:#90EE90
    style FuncLocal fill:#FFB6C1
    style LambdaLocal fill:#87CEEB
    style Params fill:#DDA0DD
```

## Lambda Capture Rules

```mermaid
flowchart TD
    Start[Identifier Reference in Lambda] --> Search[Search Scope Hierarchy]
    Search --> Found{Symbol Found?}
    
    Found -->|No| Allow1[Allow: Undefined will be caught by type checker]
    Found -->|Yes| CheckKind{Check Scope Kind}
    
    CheckKind -->|GLOBAL| Allow2[✓ Allow: Global symbols accessible everywhere]
    CheckKind -->|LAMBDA_LOCAL| CheckOwner{Owner Lambda?}
    CheckKind -->|PARAMETER| CheckOwner
    CheckKind -->|FUNCTION_LOCAL| Reject[✗ Reject: Cannot capture function-local variables]
    
    CheckOwner -->|Current Lambda| Allow3[✓ Allow: Lambda's own symbols]
    CheckOwner -->|Different Lambda| Reject2[✗ Reject: Cannot access other lambda's locals]
    
    Allow1 --> End[Continue]
    Allow2 --> End
    Allow3 --> End
    Reject --> Error[Report Capture Error]
    Reject2 --> Error
    Error --> End
    
    style Allow1 fill:#90EE90
    style Allow2 fill:#90EE90
    style Allow3 fill:#90EE90
    style Reject fill:#FFB6C1
    style Reject2 fill:#FFB6C1
    style Error fill:#FF6B6B
```

## Validation Stage Dependencies

```mermaid
graph LR
    SC[Symbol Collection] --> LV[Lambda Validation]
    SC --> TC[Type Checking]
    
    LV --> Errors[Error Aggregation]
    TC --> Errors
```

## Error Aggregation Flow

```mermaid
flowchart TD
    Start[Start Validation] --> S1[Stage 1: Symbol Collection]
    S1 --> E1[Collect Errors]
    E1 --> S2[Stage 2: Lambda Capture]
    S2 --> E2[Collect Errors]
    E2 --> S3[Stage 3: Type Checking]
    S3 --> E3[Collect Errors]
    E3 --> Aggregate[Aggregate All Errors]
    Aggregate --> Report[Report to Compiler]
    Report --> End[End]
```

## Key Design Principles

### Separation of Concerns
Each validation stage focuses on a single aspect of correctness:
- Symbol collection builds the scope hierarchy
- Lambda validation enforces non-capturing semantics
- Type checking validates type compatibility and performs inline control flow checks

### Visitor Pattern
All stages use the visitor pattern to traverse the AST, allowing clean separation between the AST structure and validation logic. The type checker itself acts as a visitor for type validation.

### Incremental Error Collection
Errors are collected throughout all stages rather than stopping at the first error, providing comprehensive feedback to the developer.

### Scope-Based Validation
The explicit scope hierarchy enables precise validation of symbol visibility and lambda capture rules.

### Inline Control Flow Checking
Control flow validation is performed inline during type checking using the `control_flow_checker_c` helper class. The type checker calls `check_no_control_flow()` and `check_no_break_or_continue()` as needed to validate control flow constraints in specific contexts like defer blocks and lambdas.

## Implementation Architecture

### Visitor Classes

The type checker uses three main visitor classes:

1. **`symbol_collector_c`**: Traverses the AST to build the scope hierarchy and register all symbols. It creates `scope_info_s` structures for functions, lambdas, and blocks, and populates them with `symbol_entry_s` entries. Lambda parameters are correctly classified as `PARAMETER` scope kind.

2. **`lambda_capture_validator_c`**: Validates that lambdas do not capture variables from enclosing scopes. It tracks the current lambda context and checks each identifier reference against the scope hierarchy to ensure only global symbols and lambda-local symbols are accessed.

3. **`type_checker_c`**: The main type checker class acts as its own visitor for type validation. It traverses the AST, validates type compatibility, checks assignments, and validates operations. It uses the `control_flow_checker_c` helper inline when needed.

### Control Flow Checking

Control flow validation is performed inline using the `control_flow_checker_c` helper class. The type checker calls:
- `check_no_control_flow()`: Validates that a code block contains no return, break, or continue statements (used for defer blocks)
- `check_no_break_or_continue()`: Validates that a code block contains no break or continue statements (used for lambda bodies)

This approach is more efficient than a separate analysis stage, as control flow checks are only performed where needed.

## Lambda Capture Validation Rationale

Lambdas in Truk compile to static C functions and cannot capture variables from enclosing scopes. The lambda capture validator enforces this constraint by:

1. Using the complete scope hierarchy built during symbol collection
2. Tracking which lambda context is currently active during validation
3. Checking each identifier reference to determine if it violates capture rules
