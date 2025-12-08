
The scanner reads over a buffer to find "static base objects" and "groups" in discrete steps, driven by the owner of
the scanner. When scanning over a buffer, the scanner provides the ability to extend the :stop: chars to "extend" the
set of symbols to treat as whitespace. This allows for context-specific parsing while still providing largely helpful
contextual delineations of data.

The scanner object understands the following "types" of data

integers    (1 +1 -1)
reals       (1.0 +1.0 -1.0)
symbols     someLine23423OfRepeated@SymboasD2332 
    the symbols are ANYTHING that was not an integer, real, or whitespace.
    the way we determine what SHOULD be grabbed is by externally managing the context of what we are parsing and
    selectively scanning "to specific endpoints."

    When we scan we can offer stop symbols that work just like whitespace. This means we can IwantToDoSOmething{ like this }
    and we can "scan until ws or '{'"", get that symbol, and follow up by expecting a "group" (explained below)
groups
    - any "mirrored" symbols : () [] {}
      any repeated pairs, with optional escape mechanism: "" '' `` || 
        when grouping the scanner asks for the optional escape so that things like nested strings can be correctly captured:
                "This is a \"nested\" string"
        
        but since hte logic is generic enough, we don't need to force ourselves to the standard "" `` '' we can also group
            || // %%, really any pair we want.

            This might be useful for doing path work "/home/billy/Documents/some_file" could scan // groups etc

Given all this info, know that SXS is the loader for truck. It drives the canner to parse the truk syntax. 
Think of it like a guy vaccuming up pieces. The pieces are chunks of data that exist, scanner is a hose we use to scoop
them up in particular groups, and sxs tells the hose what has to be scooped next to be valid. The analogy breaks down when
you realize that the setup subtly implies that we are ingesting potentially randomly ordered data. The real scenario is one
where sxs and scanner eat something lined-up and if it doesnt match a specific set of patterns (like a filter) then the whole
thing blows up. If however, the patterns were there and complete, then the truk can start. 

----------------------------------------------------------------------------------------------------------------------------------

all structures produced by sxs will be views into the already allocated buffer. we wont create new representations of the data
at this step. Instead, say we have something of the following form: 

someKeyWord someIdentifier {
    // thisi is some body that doesnt matter! 
    some_symbol = 1 + 2 ^ some_arbitrary_expression_ignore_this
}

This means once we hit someKeyWord we can scan-in someIdentifier and the {} group and not yet evaluate the internals.
This will make for quick ingestion without the need for analyzing things that may not be used by the user's code.

We would end up with something like:
    KeywordStatement {
        Identifier
        Body
    }
where we know the location of the identifier and body but we can inore the values of the identifier and body until later

I guess this is "lazy loading?"

--- ideas:
```
need [
    system/io as io
    system/alu as alu
]

form person {
    name :str :: $len <= 64
    age  :i8  :: 0 < $ < 120
}

fn init(args :list) :int {
    io/println "starting program with args %list" args

    ; any unrecognized keyword we will assume will be a def. if = not present
    ; as a symbol then its an error, otherwise we say is an assignment
    ; when we assign we essentially do a raw s-expression (i8 43) (+ 1 (+ 8 9))
    p = person {
        .name "billy"
        .age  2
    }

    io/println "person: %str, aged: %d" p.name p.age

    ; These are equivalent: 
    variableName = i8 42       ; list-v (virtual outer parenthetical list)
    variableName = (i8 42)     ; the presence of the '=' sign means "eat until newline or comment, expect one list-p or list-v"

    dim pepo [50 person {
        .name ""
        .age 1
    }]

    io/println "first : %s" pepo[0].name

    loop someLoopName {
        if (>= %iter 50) break someLoopName

        set pepo[$iter].name "AYYYYYYYLMAO" 
    }

    0
}
```