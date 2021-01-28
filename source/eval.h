#ifndef EVAL_H
#define EVAL_H

#include "types.h"

//Print error on screen (function from ui.h)
extern void error_dialog(const char* message);

//Calculate the function at point 'calcPoint' 
double calc(const char* eqBegin, int eqLength, double calcPoint, bool* stop); 
// eqBegin = pointer to equation string, eqLength = Lenght of egation string, calcPoint = x, stop = True if equation is undefined

#endif