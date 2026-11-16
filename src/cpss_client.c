#include <vmem/vmem_client.h>

#include <include_consumer/cpss_consumer.h>
#include <include_provider/cpss_provider.h>

#include "rpc_cpss_client.h"
#include "rpc_cpssv3_client.h"

static int32_t write_data(cpss_config_t *config, uint64_t vaddr, void *from, uint32_t size) {
    if (config->node == 0) {
        vmem_write(vaddr, from, size);
    } else if (config->protocol == RPC_CPSS_PROTOCOL_RDP || config->protocol == RPC_CPSS_PROTOCOL_XTX) {
        vmem_client_config_t vmem_config = { .node = config->node, .timeout = config->timeout };
        vmem_config.mtu = config->version == 3 ? config->mtu : VMEM_SERVER_MTU;
        if (vmem_client_upload(&vmem_config, vaddr, from, size) != size) {
            return -4;
        }
    } else {
        return -3;
    }

    return 0;
}

static int32_t cpssv2_put(cpss_config_t *config, void *from, uint32_t length, csp_timestamp_t *timestamp) {
    rpc_cpss_put_block_element_request_t put_request;
    rpc_cpss_put_block_element_response_t put_response;
    rpc_cpss_put_block_element_init(&put_request, config->block_id, length);

    if (timestamp && timestamp->tv_sec != 0) {
        put_request.timestamp_s = timestamp->tv_sec;
        put_request.timestamp_ns = timestamp->tv_nsec;
    }

    if (rpc_cpss_put_block_element(config->node, config->timeout, &put_request, &put_response) < 0) {
        return -1;
    }

    if (put_response.result < 0) {
        return -2;
    }

    int result = write_data(config, put_response.vaddr, from, put_response.size_actual);
    if (result < 0) {
        return result;
    }

    rpc_cpss_put_block_element_complete_request_t put_complete_request;
    rpc_cpss_put_block_element_complete_response_t put_complete_response;
    rpc_cpss_put_block_element_complete_init(&put_complete_request, config->block_id, put_response.vaddr, put_response.size_actual);

    if (rpc_cpss_put_block_element_complete(config->node, config->timeout, &put_complete_request, &put_complete_response) < 0) {
        return -1;
    }

    return put_response.size_actual;
}

static int32_t cpssv3_put(cpss_config_t *config, void *from, uint32_t length, csp_timestamp_t *timestamp) {
    rpc_cpssv3_element_put_request_t put_request;
    rpc_cpssv3_element_put_response_t put_response;
    rpc_cpssv3_element_put_init(&put_request, config->block_id, length);

    if (timestamp && timestamp->tv_sec != 0) {
        put_request.timestamp_s = timestamp->tv_sec;
        put_request.timestamp_ns = timestamp->tv_nsec;
    }

    if (rpc_cpssv3_element_put(config->node, config->timeout, &put_request, &put_response) < 0) {
        return -1;
    }

    if (put_response.result < 0) {
        return -2;
    }

    int result = write_data(config, put_response.vaddr, from, put_response.size_actual);
    if (result < 0) {
        return result;
    }

    rpc_cpssv3_element_put_complete_request_t put_complete_request;
    rpc_cpssv3_element_put_complete_response_t put_complete_response;
    rpc_cpssv3_element_put_complete_init(&put_complete_request, config->block_id, put_response.vaddr, put_response.size_actual);

    if (rpc_cpssv3_element_put_complete(config->node, config->timeout, &put_complete_request, &put_complete_response) < 0) {
        return -1;
    }

    return put_response.size_actual;
}

int32_t cpss_put(cpss_config_t *config, void *from, uint32_t length, csp_timestamp_t *timestamp) {
    switch (config->version) {
        case 1:
        case 2:
            return cpssv2_put(config, from, length, timestamp);
        case 3:
            return cpssv3_put(config, from, length, timestamp);
        default:
            break;
    }

    return -1;
}

static int32_t cpssv2_get(cpss_config_t *config, void *to, csp_timestamp_t *timestamp) {

    rpc_cpss_get_block_element_request_t get_request;
    rpc_cpss_get_block_element_response_t get_response;
    rpc_cpss_get_block_element_init(&get_request, config->block_id);

    int32_t result = rpc_cpss_get_block_element(config->node, config->timeout, &get_request, &get_response);

    if (result < 0 || get_response.size_actual == 0) {
        return -1;
    }

    timestamp->tv_sec = get_response.timestamp_s;
    timestamp->tv_nsec = get_response.timestamp_ns;

    if (config->node == 0) {
        vmem_read(to, get_response.vaddr, get_response.size_actual);
    } else if (config->protocol == RPC_CPSS_PROTOCOL_RDP) {
        vmem_client_config_t vmem_config = { .node = config->node, .timeout = config->timeout, .mtu = config->mtu };
        vmem_client_download(&vmem_config, get_response.vaddr, get_response.size_actual, to);
    } else {
        return -3;
    }

    rpc_cpss_get_block_element_complete_request_t get_complete_request;
    rpc_cpss_get_block_element_complete_response_t get_complete_response;
    rpc_cpss_get_block_element_complete_init(&get_complete_request, config->block_id, get_response.vaddr, get_response.size_actual);

    rpc_cpss_get_block_element_complete(config->node, config->timeout, &get_complete_request, &get_complete_response);

    if (get_complete_response.result < 0) {
        return -2;
    }

    return get_response.size_actual;
}

static int32_t cpssv3_get(cpss_config_t *config, void *to, uint32_t number_of_elements, csp_timestamp_t *timestamp) {

    rpc_cpssv3_elements_get_request_t get_request;
    rpc_cpssv3_elements_get_response_t get_response;
    rpc_cpssv3_elements_get_init(&get_request, config->block_id, number_of_elements);

    int32_t result = rpc_cpssv3_elements_get(config->node, config->timeout, &get_request, &get_response);
    if (result < 0 || get_response.num_elements_actual == 0) {
        return -1;
    }

    if (timestamp) {
        timestamp->tv_sec = 0;
        timestamp->tv_nsec = 0;
    }

    if (config->node == 0) {
        vmem_read(to, get_response.vaddr, get_response.num_elements_actual);
    } else if (config->protocol == RPC_CPSS_PROTOCOL_RDP) {
        vmem_client_config_t vmem_config = { .node = config->node, .timeout = config->timeout, .mtu = config->mtu };
        vmem_client_download(&vmem_config, get_response.vaddr, get_response.num_elements_actual * get_response.elem_size, to);
    } else {
        return -3;
    }

    rpc_cpssv3_elements_drop_request_t drop_request;
    rpc_cpssv3_elements_drop_response_t drop_response;
    rpc_cpssv3_elements_drop_init(&drop_request, config->block_id);
    drop_request.num_elements = get_response.num_elements_actual;

    if (rpc_cpssv3_elements_drop(config->node, config->timeout, &drop_request, &drop_response) < 0) {
        return -2;
    }

    return get_response.num_elements_actual;
}

int32_t cpss_get(cpss_config_t *config, void *to, uint32_t number_of_elements, csp_timestamp_t *timestamp) {
    switch (config->version) {
        case 1:
        case 2:
            return cpssv2_get(config, to, timestamp);
        case 3:
            return cpssv3_get(config, to, number_of_elements, timestamp);
        default:
            break;
    }

    return -1;
}

int cpss_config_init(cpss_config_t *config, int node, uint32_t block_id, rpc_cpss_protocol_t protocol, int mtu, int timeout, int version) {
    config->node = node;
    config->block_id = block_id;
    config->protocol = protocol;
    config->mtu = mtu;
    config->timeout = timeout;
    config->version = version;
    return 0;
}
