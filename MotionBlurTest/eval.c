#define _CRT_SECURE_NO_WARNINGS

#include "eval.h"

#include <stdbool.h>
#include <string.h>
#include <malloc.h>
#include <limits.h>
#include <stdio.h>
#include <math.h>

#define LaziestOpLevel 1
#define OpCount (sizeof(OpTable) / sizeof(*OpTable))

struct tagOpDescription {
	char chOp;
	int nLevel;
} OpTable[] = {
	{ '+', 1 },
	{ '-', 1 },
	{ '*', 2 },
	{ '/', 2 },
	{ '^', 3 }
};

char* FindBrace(char *str, int nPos) {
	//Assuming the braces are complete
	int nCount;
	nCount = 1;
	switch (str[nPos]) {
	case '(':
		do {
			nPos++;
			if (str[nPos] == '(')
				nCount++;
			if (str[nPos] == ')')
				nCount--;
		} while (nCount > 0);
		return str + nPos;
	case ')':
		do {
			nPos--;
			if (str[nPos] == ')')
				nCount++;
			if (str[nPos] == '(')
				nCount--;
		} while (nCount > 0);
		return str + nPos;
	}
	return NULL;
}

bool IsOp(char ch) {
	int i;
	for (i = 0; i < OpCount; i++) {
		if (OpTable[i].chOp == ch)
			return true;
	}
	return false;
}

int GetOpLevel(char ch) {
	int i;
	for (i = 0; i < OpCount; i++) {
		if (OpTable[i].chOp == ch)
			return OpTable[i].nLevel;
	}
	return INT_MAX;
}

char* FindLaziestOperationToken(char *str, int nBegin, int nEnd) {
	//Range: [nBegin, nEnd)
	char *strTemp;
	int nPosTemp;
	int i;

	//Scan from left to right
	nPosTemp = 0;
	for (i = nBegin; i < nEnd; i++) {
		//Jump to the back brace when the front appears
		strTemp = FindBrace(str, i);
		if (strTemp != NULL && strTemp > str + i) {
			i = strTemp - str;
			continue;
		}

		//Check op token
		if (IsOp(str[i]) && GetOpLevel(str[i]) < GetOpLevel(str[nPosTemp])) {
			nPosTemp = i;
			if (GetOpLevel(str[nPosTemp]) == LaziestOpLevel)
				return str + nPosTemp;
		}
	}

	//Since an valid expression cannot have op in the beginning, we can use this to judge
	//whether the op token can be found
	return nPosTemp == 0 ? NULL : str + nPosTemp;
}

double StringToDouble(char *str, int nBegin, int nEnd) {
	//A valid number must be within the range [nBegin, nEnd)
	//char strTemp[nEnd - nBegin + 1];
	double fNumber;
	char *strTemp;

	strTemp = malloc(nEnd - nBegin + 1);

	strncpy(strTemp, str + nBegin, nEnd - nBegin);
	strTemp[nEnd - nBegin] = '\0';
	sscanf(strTemp, "%lf", &fNumber);

	free(strTemp);

	return fNumber;
}

double VarStringToDouble(char *str, int nBegin, int nEnd, char *strVarName[], double fVarValue[], size_t nRuleCount) {
	//A valid number must be within the range [nBegin, nEnd)
	//char strTemp[nEnd - nBegin + 1];
	double fNumber;
	char *strTemp;

	strTemp = malloc(nEnd - nBegin + 1);

	strncpy(strTemp, str + nBegin, nEnd - nBegin);
	strTemp[nEnd - nBegin] = '\0';

	fNumber = nan("");
	for (size_t i = 0; i < nRuleCount; i++) {
		if (strcmp(strTemp, strVarName[i]) == 0) {
			fNumber = fVarValue[i];
			break;
		}
	}

	free(strTemp);

	return fNumber;
}

double CalcExpression_Inner(char *str, int nBegin, int nEnd, char *strVarName[], double fVarValue[], size_t nRuleCount) {
	//Range: [nBegin, nEnd)
	double fTempVal;
	char *strTemp;

	//Find the laziest op to cut the expression into halves
	//If not found, you may got a expression wrapped with braces or just a single number
	strTemp = FindLaziestOperationToken(str, nBegin, nEnd);
	if (strTemp != NULL) {
		switch (*strTemp) {
		case '+':
			return CalcExpression_Inner(str, nBegin, strTemp - str, strVarName, fVarValue, nRuleCount) +
				CalcExpression_Inner(str, strTemp - str + 1, nEnd, strVarName, fVarValue, nRuleCount);
		case '-':
			return CalcExpression_Inner(str, nBegin, strTemp - str, strVarName, fVarValue, nRuleCount) -
				CalcExpression_Inner(str, strTemp - str + 1, nEnd, strVarName, fVarValue, nRuleCount);
		case '*':
			return CalcExpression_Inner(str, nBegin, strTemp - str, strVarName, fVarValue, nRuleCount) *
				CalcExpression_Inner(str, strTemp - str + 1, nEnd, strVarName, fVarValue, nRuleCount);
		case '/':
			return CalcExpression_Inner(str, nBegin, strTemp - str, strVarName, fVarValue, nRuleCount) /
				CalcExpression_Inner(str, strTemp - str + 1, nEnd, strVarName, fVarValue, nRuleCount);
		case '^':
			return pow(CalcExpression_Inner(str, nBegin, strTemp - str, strVarName, fVarValue, nRuleCount),
				CalcExpression_Inner(str, strTemp - str + 1, nEnd, strVarName, fVarValue, nRuleCount));
		default:
			return nan("");
		}
	}
	else {
		//Check whether it is a pair of braces or is just a number
		if (FindBrace(str, nBegin) == str + nEnd - 1) {
			return CalcExpression_Inner(str, nBegin + 1, nEnd - 1, strVarName, fVarValue, nRuleCount);
		}
		else {
			fTempVal = VarStringToDouble(str, nBegin, nEnd, strVarName, fVarValue, nRuleCount);
			return isnan(fTempVal) ? StringToDouble(str, nBegin, nEnd) : fTempVal;
		}
	}
}

double CalcExpression(char *str) {
	return CalcExpression_Inner(str, 0, strlen(str), NULL, NULL, 0);
}

double CalcExpressionWithRule(char *str, char *strVarName[], double fVarValue[], size_t nRuleCount) {
	return CalcExpression_Inner(str, 0, strlen(str), strVarName, fVarValue, nRuleCount);
}