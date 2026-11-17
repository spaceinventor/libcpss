#pragma once

#include <cpss.h>
#include <rpc_cpss.h>

int32_t cpss_get(cpss_config_t *config, void *to, uint32_t length, csp_timestamp_t *timestamp);
