#pragma once

#include <cpss.h>

/**
 * @brief Puts data into a single element.
 *
 * If the number of bytes exceeds the block's element size,
 * the length will be truncated to fit the element size.
 *
 * @param config    The configuration data.
 * @param from      Pointer to the source data.
 * @param length    The number of bytes to write.
 * @param timestamp Pointer to an optional timestamp to be set on the element.
 * @return          The number of bytes successfully written to the element (or a negative error code on failure).
 */
int32_t cpss_put(cpss_config_t *config, void *from, uint32_t length, csp_timestamp_t *timestamp);
