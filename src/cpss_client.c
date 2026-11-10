#include "vmem/vmem_client.h"
#include "rpc_cpss_client.h"
#include "rpc_cpssv2_client.h"

static int32_t write_data(int node, uint64_t vaddr, void *from, uint32_t size, rpc_cpss_protocol_t protocol, int timeout) {
    if (node == 0) {
        vmem_write(vaddr, from, size);
    } else if (protocol == RPC_CPSS_PROTOCOL_RDP || protocol == RPC_CPSS_PROTOCOL_XTX ||
               protocol == RPC_CPSS_PROTOCOL_V2_RDP || protocol == RPC_CPSS_PROTOCOL_V2_XTX) {
        if (vmem_upload_progress(node, timeout, vaddr, from, size, 2, NULL) != size) {
            return -4;
        }
    } else {
        return -3;
    }

    return 0;
}

static int32_t cpssv1_put(int node, uint32_t block_id, void * from, uint32_t length, csp_timestamp_t * timestamp, rpc_cpss_protocol_t protocol, int timeout) {
    rpc_cpss_put_block_element_request_t put_request;
    rpc_cpss_put_block_element_response_t put_response;
    rpc_cpss_put_block_element_init(&put_request, block_id, length);

    if (timestamp && timestamp->tv_sec != 0) {
        put_request.timestamp_s = timestamp->tv_sec;
        put_request.timestamp_ns = timestamp->tv_nsec;
    }

    if (rpc_cpss_put_block_element(node, timeout, &put_request, &put_response) < 0) {
        return -1;
    }

    if (put_response.result < 0) {
        return -2;
    }

    int result = write_data(node, put_response.vaddr, from, put_response.size_actual, protocol, timeout);
    if (result < 0) {
        return result;
    }

    rpc_cpss_put_block_element_complete_request_t put_complete_request;
    rpc_cpss_put_block_element_complete_response_t put_complete_response;
    rpc_cpss_put_block_element_complete_init(&put_complete_request, block_id, put_response.vaddr, put_response.size_actual);

    if (rpc_cpss_put_block_element_complete(node, timeout, &put_complete_request, &put_complete_response) < 0) {
        return -1;
    }

    return put_response.size_actual;
}

static int32_t cpssv2_put(int node, uint32_t block_id, void * from, uint32_t length, csp_timestamp_t * timestamp, rpc_cpss_protocol_t protocol, int timeout) {
    rpc_cpssv2_put_block_element_request_t put_request;
    rpc_cpssv2_put_block_element_response_t put_response;
    rpc_cpssv2_put_block_element_init(&put_request, block_id, length);

    if (timestamp && timestamp->tv_sec != 0) {
        put_request.timestamp_s = timestamp->tv_sec;
        put_request.timestamp_ns = timestamp->tv_nsec;
    }

    if (rpc_cpssv2_put_block_element(node, timeout, &put_request, &put_response) < 0) {
        return -1;
    }

    if (put_response.result < 0) {
        return -2;
    }

    int result = write_data(node, put_response.vaddr, from, put_response.size_actual, protocol, timeout);
    if (result < 0) {
        return result;
    }

    rpc_cpssv2_put_block_element_complete_request_t put_complete_request;
    rpc_cpssv2_put_block_element_complete_response_t put_complete_response;
    rpc_cpssv2_put_block_element_complete_init(&put_complete_request, block_id, put_response.vaddr, put_response.size_actual);

    if (rpc_cpssv2_put_block_element_complete(node, timeout, &put_complete_request, &put_complete_response) < 0) {
        return -1;
    }

    return put_response.size_actual;
}

int32_t cpss_put(int node, uint32_t block_id, void * from, uint32_t length, csp_timestamp_t * timestamp, rpc_cpss_protocol_t protocol, int timeout) {
    switch (protocol) {
        case RPC_CPSS_PROTOCOL_RDP:
        case RPC_CPSS_PROTOCOL_DTP:
        case RPC_CPSS_PROTOCOL_XTX:
            return cpssv1_put(node, block_id, from, length, timestamp, protocol, timeout);
        case RPC_CPSS_PROTOCOL_V2_RDP:
        case RPC_CPSS_PROTOCOL_V2_DTP:
        case RPC_CPSS_PROTOCOL_V2_XTX:
            return cpssv2_put(node, block_id, from, length, timestamp, protocol, timeout);
        default:
            break;
    }

    return -1;
}

static int32_t cpssv1_get(void * to, int node, uint32_t block_id, csp_timestamp_t * timestamp, rpc_cpss_protocol_t protocol, int timeout) {

    rpc_cpss_get_block_element_request_t get_request;
    rpc_cpss_get_block_element_response_t get_response;
    rpc_cpss_get_block_element_init(&get_request, block_id);

    int32_t result = rpc_cpss_get_block_element(node, timeout, &get_request, &get_response);

    if (result < 0 || get_response.size_actual == 0) {
        return -1;
    }

    timestamp->tv_sec = get_response.timestamp_s;
    timestamp->tv_nsec = get_response.timestamp_ns;

    if (node == 0) {
        vmem_read(to, get_response.vaddr, get_response.size_actual);
    } else if (protocol == RPC_CPSS_PROTOCOL_RDP) {
        vmem_download_progress(node, timeout, get_response.vaddr, get_response.size_actual, to, 2, 1, NULL);
    } else {
        return -3;
    }

    rpc_cpss_get_block_element_complete_request_t get_complete_request;
    rpc_cpss_get_block_element_complete_response_t get_complete_response;
    rpc_cpss_get_block_element_complete_init(&get_complete_request, block_id, get_response.vaddr, get_response.size_actual);

    rpc_cpss_get_block_element_complete(node, timeout, &get_complete_request, &get_complete_response);

    if (get_complete_response.result < 0) {
        return -2;
    }

    return get_response.size_actual;
}

static int32_t cpssv2_get(void * to, int node, uint32_t block_id, uint32_t number_of_elements, csp_timestamp_t * timestamp, rpc_cpss_protocol_t protocol, int timeout) {

    rpc_cpssv2_get_block_elements_request_t get_request;
    rpc_cpssv2_get_block_elements_response_t get_response;
    rpc_cpssv2_get_block_elements_init(&get_request, block_id, number_of_elements);

    int32_t result = rpc_cpssv2_get_block_elements(node, timeout, &get_request, &get_response);
    if (result < 0 || get_response.num_elements_actual == 0) {
        return -1;
    }

    if (timestamp) {
        timestamp->tv_sec = 0;
        timestamp->tv_nsec = 0;
    }

    if (node == 0) {
        vmem_read(to, get_response.vaddr, get_response.num_elements_actual);
    } else if (protocol == RPC_CPSS_PROTOCOL_RDP) {
        vmem_download_progress(node, timeout, get_response.vaddr, get_response.num_elements_actual, to, 2, 1, NULL);
    } else {
        return -3;
    }

    rpc_cpssv2_drop_elements_request_t drop_request;
    rpc_cpssv2_drop_elements_response_t drop_response;
    rpc_cpssv2_drop_elements_init(&drop_request, block_id, get_response.num_elements_actual);

    if (rpc_cpssv2_drop_elements(node, timeout, &drop_request, &drop_response) < 0) {
        return -2;
    }

    return get_response.num_elements_actual;
}

int32_t cpss_get(void * to, int node, uint32_t block_id, uint32_t number_of_elements, csp_timestamp_t * timestamp, rpc_cpss_protocol_t protocol, int timeout) {
    switch (protocol) {
        case RPC_CPSS_PROTOCOL_RDP:
        case RPC_CPSS_PROTOCOL_DTP:
        case RPC_CPSS_PROTOCOL_XTX:
            return cpssv1_get(to, node, block_id, timestamp, protocol, timeout);
        case RPC_CPSS_PROTOCOL_V2_RDP:
        case RPC_CPSS_PROTOCOL_V2_DTP:
        case RPC_CPSS_PROTOCOL_V2_XTX:
            return cpssv2_get(to, node, block_id, number_of_elements, timestamp, protocol, timeout);
        default:
            break;
    }

    return -1;
}
