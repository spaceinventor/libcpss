#pragma once

#include "cpss.h"

extern int32_t cpss_put(int node, uint32_t block_id, void * from, int32_t length, csp_timestamp_t * timestamp, rpc_protocol_t protocol, int timeout);
