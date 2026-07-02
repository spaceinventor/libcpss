#pragma once

#include "cpss.h"

extern int32_t cpss_get(void * to, int node, uint32_t block_id, int32_t length, csp_timestamp_t * timestamp, rpc_protocol_t protocol, int timeout);
