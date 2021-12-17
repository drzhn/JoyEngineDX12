#ifndef ASSERT_H
#define ASSERT_H

#ifdef _DEBUG

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <iostream>
#include <sstream>

#ifdef _MSC_VER
#include <intrin.h>
#define DEBUG_BREAK __debugbreak()
#else
#endif

#define BREAK(expr) \
	std::stringstream ss; \
	ss << "Error: " << #expr << " " <<  __FILE__ <<  ":" << __LINE__ << std::endl; \
	OutputDebugStringA(ss.str().c_str()); \
	DEBUG_BREAK;
#define ASSERT(expr) if (expr) {} else { BREAK(expr) }
#define ASSERT_DESC(expr, message) if (expr) {} else { std::cerr << message << " " << #expr << " " <<  __FILE__ <<  ":" << __LINE__ << std::endl; DEBUG_BREAK;}
#define ASSERT_SUCC(expr) if (FAILED(expr)) {BREAK(expr)}

#else
#define ASSERT(expr)
#define ASSERT_DESC(expr, message)
#define ASSERT_SUCC(expr) expr;
#endif //DEBUG

#endif //ASSERT_H
