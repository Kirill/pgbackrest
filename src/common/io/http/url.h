/***********************************************************************************************************************************
HTTP URL

Parse a URL into component parts.
***********************************************************************************************************************************/
#ifndef COMMON_IO_HTTP_URL_H
#define COMMON_IO_HTTP_URL_H

/***********************************************************************************************************************************
Object type
***********************************************************************************************************************************/
typedef struct HttpUrl HttpUrl;

#include "common/type/param.h"
#include "common/type/object.h"
#include "common/type/string.h"

/***********************************************************************************************************************************
HTTP protocol type
***********************************************************************************************************************************/
typedef enum
{
    httpProtocolTypeAny = 0,
    httpProtocolTypeHttp = 1,
    httpProtocolTypeHttps = 2,
} HttpProtocolType;

/***********************************************************************************************************************************
Constructors
***********************************************************************************************************************************/
typedef struct HttpUrlNewParseParam
{
    VAR_PARAM_HEADER;
    HttpProtocolType type;                                          // Expected protocol type (httpProtocolTypeAny if any)
} HttpUrlNewParseParam;

#define httpUrlNewParseP(url, ...)                                                                                                 \
    httpUrlNewParse(url, (HttpUrlNewParseParam){VAR_PARAM_INIT, __VA_ARGS__})

HttpUrl *httpUrlNewParse(const String *const url, HttpUrlNewParseParam param);

/***********************************************************************************************************************************
Getters/setters
***********************************************************************************************************************************/
typedef struct HttpUrlPub
{
    const String *url;                                              // Original URL
    HttpProtocolType type;                                          // Protocol type, e.g. http
    const String *host;                                             // Host
    unsigned int port;                                              // Port
    const String *path;                                             // Path
} HttpUrlPub;

// Protocol type
__attribute__((always_inline)) static inline HttpProtocolType
httpUrlProtocolType(const HttpUrl *const this)
{
    return THIS_PUB(HttpUrl)->type;
}

// Host
__attribute__((always_inline)) static inline const String *
httpUrlHost(const HttpUrl *const this)
{
    return THIS_PUB(HttpUrl)->host;
}

// Path
__attribute__((always_inline)) static inline const String *
httpUrlPath(const HttpUrl *const this)
{
    return THIS_PUB(HttpUrl)->path;
}

// Port
__attribute__((always_inline)) static inline unsigned int
httpUrlPort(const HttpUrl *const this)
{
    return THIS_PUB(HttpUrl)->port;
}

// URL (exactly as originally passed)
__attribute__((always_inline)) static inline const String *
httpUrl(const HttpUrl *const this)
{
    return THIS_PUB(HttpUrl)->url;
}

/***********************************************************************************************************************************
Destructor
***********************************************************************************************************************************/
__attribute__((always_inline)) static inline void
httpUrlFree(HttpUrl *const this)
{
    objFree(this);
}

/***********************************************************************************************************************************
Macros for function logging
***********************************************************************************************************************************/
String *httpUrlToLog(const HttpUrl *this);

#define FUNCTION_LOG_HTTP_URL_TYPE                                                                                               \
    HttpUrl *
#define FUNCTION_LOG_HTTP_URL_FORMAT(value, buffer, bufferSize)                                                                  \
    FUNCTION_LOG_STRING_OBJECT_FORMAT(value, httpUrlToLog, buffer, bufferSize)

#endif
