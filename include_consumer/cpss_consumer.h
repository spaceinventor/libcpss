#pragma once

#include <cpss.h>
#include <rpc_cpss.h>

int32_t cpss_get(cpss_instance_t *instance, void *to, uint32_t number_of_elements, csp_timestamp_t *timestamp);
