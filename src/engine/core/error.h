#pragma once

#include <hclib.h>

#define EITHER(TypeA, TypeB) struct { union {TypeA a; TypeB b;}; bool isA; }
#define EITHER_GET(E) ({typeof(E) _e = (E); _e.isA ? _e.a : _e.b;})

#define RESULT(Type) EITHER(Type, i32)