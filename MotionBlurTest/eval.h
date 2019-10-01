#pragma once

#include <stddef.h>

double CalcExpression(char *str);
double CalcExpressionWithRule(char *str, char *strVarName[], double fVarValue[], size_t nRuleCount);