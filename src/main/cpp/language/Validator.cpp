#include "language/Validator.hpp"
#include <print> // std::print, stderr, std::terminate()

void phudMacroAssert(const char* const expression, const bool expressionReturnValue,
                     const char* const fileName, const char* const functionName, int line,
                     std::string_view errorMessage) {
  if (!expressionReturnValue) {
    std::print(stderr,
               "Assertion failed in file {}: {}\nFunction {}\nExpression:\t{}\nExpected:\t{}\n",
               fileName, line, functionName, expression, errorMessage);
    std::terminate(); // stops the program with cleaning up
  }
}

void validation::require(bool mustBeTrue, std::string_view errorMessage) {
  phudAssert(mustBeTrue, errorMessage);
}
