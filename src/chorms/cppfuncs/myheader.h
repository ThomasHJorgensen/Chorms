////////////////
// 1. generic //
////////////////

#include <windows.h>
#include <cmath>
#include <stdio.h>
#include <string>
#include <omp.h>

#include "nlopt-2.4.2-dll64\nlopt.h"

///////////////
// 2. custom //
///////////////

#define EXPORT extern "C" __declspec(dllexport)

#define MAX(X,Y) ((X)>(Y)?(X):(Y))
#define MIN(X,Y) ((X)<(Y)?(X):(Y))
#define BOUND(X,A,B) MIN(MAX(X,A),B)
#define HUGE_VAL 1000000000000.0

#define man 1
#define woman 2

double halflog2pi = 0.5*std::log(2.0*3.14159265358979323846);

////////////////
// 3. structs //
////////////////

#include "par_struct.cpp"
#include "sol_struct.cpp"
#include "sim_struct.cpp"

/////////////////
// 4. includes //
/////////////////
#include "logs.cpp"

#ifndef UTILS
#include "utils.cpp"
#endif