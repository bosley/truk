0) Change alloc and alloc_array and free free_array to `make` and `delete` like go - infer from type the array or not etc/ might need to extend TS a bit

1) RXI Void map with type system so that `map` is a primitive, again, like go, but we will use our void map (with TS checking it) using `make(map<*u8, *u8>) `

2) consider adding BOEHM GC - maybe as something that can be enabled/disabled at compile time? this w
