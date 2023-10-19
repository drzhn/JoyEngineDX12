#ifndef ASSERT_H
#define ASSERT_H

#ifdef _DEBUG
#include "Log.h"

#ifdef _MSC_VER
#include <intrin.h>
#include <intsafe.h>
#define DEBUG_BREAK __debugbreak()
#else
#error Unsupported compiler
#endif

#define BREAK(expr) \
	Logger::LogFormat("Error: %s %s:%d\n", #expr, __FILE__, __LINE__); \
	DEBUG_BREAK;
#define ASSERT(expr) if (expr) {} else { BREAK(expr) }
#define ASSERT_DESC(expr, message) \
if (expr) {} else {\
	Logger::LogFormat("Error: %s %s %s:%d\n", message, #expr, __FILE__, __LINE__);\
	DEBUG_BREAK;}
#define ASSERT_SUCC(expr) {\
HRESULT expressionResult = expr; \
if (FAILED(expressionResult)) {\
	Logger::LogFormat("HRESULT: %X\n", expressionResult);\
	BREAK(expr);\
}}

#else
#define ASSERT(expr)
#define ASSERT_DESC(expr, message)
#define ASSERT_SUCC(expr) expr;
#endif //DEBUG

#endif //ASSERT_H
