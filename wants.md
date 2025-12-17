enums with data inside that we can match on, we should either build enum WITH match or
build match first

slice syntaxing. we can do arr [] but we should do `var slice: []i32 = arr[10..20]` etc

tuples/ multiple returns and structured decomposition


fn myfn() : (i32, i32, *u8) {
    return 42, 99, "Hello, world!";

    // We might want to consider a simpler, builtin if we dont want to f with `,` in expressions
    // return many(42, 99, "Hello, world!");
}

fn main() :i32 {

    // new keyword let, which i also wanted to do typelss defines (implicit from expression eval)
    let answer, fav, txt = myfn();

    // we sould also want to ensure that we have a special `_`
    let _, _, txt = myfn(); // so we can just ignore some

    // another power of let would be:
    let x = "Some inferred string";
}