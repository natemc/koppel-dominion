#pragma once

////////////////////////////////////////////////////////////////////////////////
// Handy macros for creating concise lambdas.
////////////////////////////////////////////////////////////////////////////////

#define L0(e) [&](                  ){ return e; }
#define L1(e) [&](auto&& x          ){ return e; }
#define L2(e) [&](auto&& x, auto&& y){ return e; }

// No-capture versions for use at file scope
#define LNC0(e) [](                  ){ return e; }
#define LNC1(e) [](auto&& x          ){ return e; }
#define LNC2(e) [](auto&& x, auto&& y){ return e; }
