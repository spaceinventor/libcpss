#pragma once

#include "cpss.h"

extern int cpss_get(void * to, int node, uint32_t block_id, uint32_t length, rpc_protocol_t protocol, int timeout);
