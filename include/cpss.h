#pragma once

#include <rpc_cpss.h>

typedef struct {
    int node;
    uint32_t block_id;
    rpc_cpss_protocol_t protocol;
    int timeout;
    int mtu; // Only used for CPSS version 3
    int version; // CPSS version: 2, or 3
} cpss_config_t;
