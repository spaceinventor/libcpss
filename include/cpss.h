#pragma once

#include <rpc_cpss.h>

typedef struct {
    int node;
    uint32_t block_id;
    rpc_cpss_protocol_t protocol;
    int mtu;
    int timeout;
    int version; // CPSS version: 1, 2, or 3
} cpss_instance_t;

int cpss_instance_init(cpss_instance_t *instance, int node, uint32_t block_id, rpc_cpss_protocol_t protocol, int mtu, int timeout, int version);
