#ifndef ASSERT_H
#define ASSERT_H

#ifdef _DEBUG
#include "Log.h"

#ifdef _MSC_VER
#include <intrin.h>
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
#define ASSERT_SUCC(expr) if (FAILED(expr)) {BREAK(expr)}

#else
#define ASSERT(expr)
#define ASSERT_DESC(expr, message)
#define ASSERT_SUCC(expr) expr;
#endif //DEBUG

#endif //ASSERT_H
