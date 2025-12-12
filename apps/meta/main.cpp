#include "truk/core/core.hpp"
#include <cstring>
#include <fmt/core.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <truk/emitc/emitter.hpp>
#include <truk/ingestion/parser.hpp>
#include <truk/validation/typecheck.hpp>
/*
    The hope is that we can define a more complex truk_object that encompases all primitive
    types and that we can use to build more advanced types. If we do this well in c++ we can
    get low to 0 overhead at c++ compile time and still have some dynamic flexability and
    higher-level objects at the truk-code level

    For instance:

    fn some_scope() :none {

        var x :i32 = 44;
    }

    Thats a primitive, and doesn't leave a scope so it would be flattened by c++

    fn some_scope() :none {

        var x :i32 = 44;

        print("%d", x); // same - still not collapesed

        print("type: %s", x.type_name()) // now its being used as the c++ dynamic object
    }

    even if we dont get it to fully collapse and we have a lightweight c++ class around all types im fine with that
    then we can emit that c++ to the file to be compiled.

    it will allw us to have a base type info available at runtime and we can then easily leverage memory management
    we don't have to respect anything other than procedural flow. we dont have to place things on a stack
    we could even make a fucking unordered_map<stirng, ob> and just call into that at runtime (Bad idea but
    still we COULD)
*/

bool system_init_truk();

/*
    Auto generated prototypes for user code
*/
int user_code_root();

/*
    Global constants that the user defined
*/
// truk_object_i64 USER_GLOBAL_x;

/*
    This will be the actual entrance (obviously)

    We will call their main from here
*/
int main(int argc, char **argv) {
    if (!system_init_truk()) {
        return 1;
    }
  return user_code_root();
}

bool system_init_truk() {

    /*
        Here we generate the initializations of all globals/ runtime stuff
    */

    //USER_GLOBAL_x = 0;

    return true;
}

int user_code_root() {
    
}