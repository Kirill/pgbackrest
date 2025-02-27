/***********************************************************************************************************************************
TLS Server

A simple TLS server intended to expose services by accepting TLS over an IoSession.
***********************************************************************************************************************************/
#ifndef COMMON_IO_TLS_SERVER_H
#define COMMON_IO_TLS_SERVER_H

#include "common/io/server.h"
#include "common/io/tls/client.h"
#include "common/time.h"

/***********************************************************************************************************************************
Io server type
***********************************************************************************************************************************/
#define IO_SERVER_TLS_TYPE                                          IO_CLIENT_TLS_TYPE

/***********************************************************************************************************************************
Statistics constants
***********************************************************************************************************************************/
#define TLS_STAT_SERVER                                             "tls.server"            // Servers created
    STRING_DECLARE(TLS_STAT_SERVER_STR);

/***********************************************************************************************************************************
Constructors
***********************************************************************************************************************************/
IoServer *tlsServerNew(
    const String *host, const String *caFile, const String *keyFile, const String *certFile, TimeMSec timeout);

#endif
