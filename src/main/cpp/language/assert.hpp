#pragma once

#ifndef NDEBUG
void phudMacroAssert(const char* const expression, bool expressionReturnValue,
                     const char* const fileName, const char* const functionName, int line,
                     const char* const errorMessage);
#  define phudAssert(Expr, Msg) phudMacroAssert(#Expr, Expr, __FILE__, __func__, __LINE__, Msg)
#else
#  define phudAssert(Expr, Msg)
#endif // NDEBUG
