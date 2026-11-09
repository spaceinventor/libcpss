#include "vmem/vmem_client.h"
#include "rpc_cpss_client.h"

typedef struct {
    int node;
    uint32_t block_id;
    rpc_protocol_t protocol;
    int mtu;
    int timeout;
} cpss_provider_context_t;

int cpss_provider_init(cpss_provider_context_t * instance, int node, uint32_t block_id, rpc_protocol_t protocol, int mtu, int timeout) {
    instance->node = node;
    instance->block_id = block_id;
    instance->protocol = protocol;
    instance->mtu = mtu;
    instance->timeout = timeout;

    return 0;
}

int32_t cpss_put(cpss_provider_context_t * instance, void * from, uint32_t length, csp_timestamp_t * timestamp) {

    rpc_cpss_put_block_element_request_t put_request;
    rpc_cpss_put_block_element_response_t put_response;
    rpc_cpss_put_block_element_init(&put_request, instance->block_id, length);

    if (timestamp && timestamp->tv_sec != 0) {
        put_request.timestamp_s = timestamp->tv_sec;
        put_request.timestamp_ns = timestamp->tv_nsec;
    }

    if (rpc_cpss_put_block_element(instance->node, instance->timeout, &put_request, &put_response) < 0) {
        return -1;
    }

    if (put_response.result < 0) {
        return -2;
    }

    if (instance->node == 0) {
        vmem_write(put_response.vaddr, from, put_response.size_actual);
    } else if (instance->protocol == RPC_PROTOCOL_RDP || instance->protocol == RPC_PROTOCOL_XTX) {
        if (vmem_upload_progress(instance->node, instance->timeout, put_response.vaddr, from, put_response.size_actual, 2, NULL) != put_response.size_actual) {
            return -4;
        }
    } else {
        return -3;
    }

    rpc_cpss_put_block_element_complete_request_t put_complete_request;
    rpc_cpss_put_block_element_complete_response_t put_complete_response;
    rpc_cpss_put_block_element_complete_init(&put_complete_request, instance->block_id, put_response.vaddr, put_response.size_actual);

    if (rpc_cpss_put_block_element_complete(instance->node, instance->timeout, &put_complete_request, &put_complete_response) < 0) {
        return -1;
    }

    return put_response.size_actual;
}

int32_t cpss_get(void * to, int node, uint32_t block_id, uint32_t length, csp_timestamp_t * timestamp, rpc_protocol_t protocol, int timeout) {

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
    } else if (protocol == RPC_PROTOCOL_RDP) {
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
