#pragma once 

#include <fmt/core.h>

// TODO: place all of the helper functions to genersate c code here
// for instance: a astatic function that returns a struct definition in c using proper
// typedefs but taking in formatting for the string that is to be used so we can use libfmt
// to template the c code int "blocks" or "chunks" that the emitter can call upon
// given the contextual information
// we should make a series of functions that we can compose the templates with so things
// like defining variables of specific types just map directly to what we need
// for instance we can have a program-start chunk that has all the includes we know we need.
// we should typedef all the ints and C-types we plan on using along with helper/safety functions
// and all functions we need to "interact" with types the way our bilitns do (Alloc/free/etc)
// the typechecker should have already resolved symbols and ensured uniqueness so we shouldnt 
// have to be concerned with that here, just generate the code. we should place debug lines in
// (the macr extension) to map source lines 