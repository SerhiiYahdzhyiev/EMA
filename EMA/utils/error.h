#ifndef EMA_ERROR_UTILS_H
#define EMA_ERROR_UTILS_H

#include <errno.h>
#include <stdio.h>
#include <string.h>

#define ERROR_MESSAGE_BUFFER_SIZE 256

/* ==========================
 *
 *    Useful error macros
 *
 * ==========================
 */

/* Print message to stderr */
#define ERROR_MSG(MSG, ...) do { \
    fprintf(stderr, MSG "\n", ##__VA_ARGS__); \
} while(0)

#define SYS_ERROR_MSG(MSG, ...) do { \
    char error_message[ERROR_MESSAGE_BUFFER_SIZE]; \
    strerror_r(errno, error_message, ERROR_MESSAGE_BUFFER_SIZE); \
    fprintf(stderr, MSG ": %s.\n", ##__VA_ARGS__, error_message); \
} while(0)

/* Assertions */
#define ASSERT(COND, RET) do { \
    if( !(COND) ) \
        return RET; \
} while(0)

#define ASSERT_OR_NULL(COND) ASSERT(COND, NULL)
#define ASSERT_OR_0(COND) ASSERT(COND, 0)
#define ASSERT_OR_1(COND) ASSERT(COND, 1)

/* With message */
#define ASSERT_MSG(COND, RET, MSG, ...) do { \
    if( !(COND) ) \
    { \
        ERROR_MSG(MSG, ##__VA_ARGS__); \
        return RET; \
    } \
} while(0)

#define ASSERT_MSG_OR_0(COND, MSG, ...) \
    ASSERT_MSG(COND, 0, MSG, ##__VA_ARGS__)
#define ASSERT_MSG_OR_1(COND, MSG, ...) \
    ASSERT_MSG(COND, 1, MSG, ##__VA_ARGS__)
#define ASSERT_MSG_OR_NULL(COND, MSG, ...) \
    ASSERT_MSG(COND, NULL, MSG, ##__VA_ARGS__)

#define ASSERT_SYS_MSG(COND, RET, MSG, ...) do { \
    if( !(COND) ) \
    { \
        SYS_ERROR_MSG(MSG, ##__VA_ARGS__); \
        return RET; \
    } \
} while(0)

#define ASSERT_SYS_MSG_OR_0(COND, MSG, ...) \
    ASSERT_SYS_MSG(COND, 0, MSG, ##__VA_ARGS__)
#define ASSERT_SYS_MSG_OR_1(COND, MSG, ...) \
    ASSERT_SYS_MSG(COND, 1, MSG, ##__VA_ARGS__)
#define ASSERT_SYS_MSG_OR_NULL(COND, MSG, ...) \
    ASSERT_SYS_MSG(COND, NULL, MSG, ##__VA_ARGS__)

#endif
