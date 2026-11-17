#pragma once

#include <cpss.h>
#include <rpc_cpss.h>

/**
 * @brief Retrieves data from a single element.
 *
 * If the requested length exceeds the block's element size,
 * the length will be truncated to fit the available data.
 *
 * @param config    The configuration data.
 * @param to        Pointer to the destination buffer.
 * @param length    The maximum number of bytes to read.
 * @param timestamp Pointer to an optional timestamp to be set on the element.
 * @return          The number of bytes successfully read from the element (or a negative error code on failure).
 */
int32_t cpss_get(cpss_config_t *config, void *to, uint32_t length, csp_timestamp_t *timestamp);
