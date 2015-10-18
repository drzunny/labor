#ifndef __LABOR_DEF_H__
#define __LABOR_DEF_H__

// Labor's VERSION String
#define LABOR_VERSION "0.0.1"

// Namespace
#define NAMESPACE(what) namespace what {
#define NAMESPACE_END   }

// Some Macro Helpers
#define __S(x) std::string(x)
#define LABOR_ASSERT(x, msg) assert((x) && (msg))

// About Log
#if !defined(LABOR_LOG_BUFFER)
#   define LABOR_LOG_BUFFER 1024
#elif LABOR_LOG_BUFFER <= 0
#   error "log buffer cannot lower-equal than 0"
#endif

// Hashtable
#define Hashtable std::unordered_map

#endif
