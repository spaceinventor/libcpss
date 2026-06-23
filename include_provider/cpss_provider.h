#pragma once

#include "cpss.h"

extern int cpss_put(int node, uint32_t block_id, void * from, uint32_t length, rpc_protocol_t protocol, int timeout);
