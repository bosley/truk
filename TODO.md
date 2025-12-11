0 setup c code libs and cmake auto gen cpp files for emitter
1 update main t generate project structures
2 setup new test suite using bash to transpile a series of .truk -> .c -> compile -> run and validate output with `expect` scripts to balidate data utput (means we need a print and/or to return specific codes) 

Once we have the core types allocations/dallsocations structs and arrays/etc all tested and verified
THEN we can start to have FUN




..... we could abandon the projects idea and just do raw mix-in with c. 

just 

./truk -i /include/dirs -t main.truk 

then have a "extern "C"" mechanism to just include c files to be used in our compilation

that adds a potential lot of complexity we might not want to support but its worth cnsiering