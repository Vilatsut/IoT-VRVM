#include "net/gcoap.h"
#include "net/sock/util.h"

#include "od.h"
#include "coap_wrapper.h"

#define _LAST_REQ_PATH_MAX (32)
static char _last_req_path[_LAST_REQ_PATH_MAX];

// Taken from RIOT-OS course examples
static void _resp_handler (const gcoap_request_memo_t *memo, coap_pkt_t *pdu,
                            const sock_udp_ep_t *remote)
{
    (void)remote;

    if (memo->state == GCOAP_MEMO_TIMEOUT) {
        printf("gcoap: timeout for msg ID %02u\n", coap_get_id(pdu));
        return;
    }
    else if (memo->state == GCOAP_MEMO_ERR) {
        printf("gcoap: error in response\n");
        return;
    }

    coap_block1_t block;
    if (coap_get_block2(pdu, &block) && block.blknum == 0) {
        puts("--- blockwise start ---");
    }

    char *class_str = (coap_get_code_class(pdu) == COAP_CLASS_SUCCESS)
                            ? "Success" : "Error";
    printf("gcoap: response %s, code %1u.%02u", class_str,
                                                coap_get_code_class(pdu),
                                                coap_get_code_detail(pdu));
    if (pdu->payload_len) {
        unsigned content_type = coap_get_content_type(pdu);
        if (content_type == COAP_FORMAT_TEXT
                || content_type == COAP_FORMAT_LINK
                || coap_get_code_class(pdu) == COAP_CLASS_CLIENT_FAILURE
                || coap_get_code_class(pdu) == COAP_CLASS_SERVER_FAILURE) {
            /* Expecting diagnostic payload in failure cases */
            printf(", %u bytes\n%.*s\n", pdu->payload_len, pdu->payload_len,
                                                          (char *)pdu->payload);
        }
        else {
            printf(", %u bytes\n", pdu->payload_len);
            od_hex_dump(pdu->payload, pdu->payload_len, OD_WIDTH_DEFAULT);
        }
    }
    else {
        printf(", empty payload\n");
    }

    /* ask for next block if present */
    if (coap_get_block2(pdu, &block)) {
        if (block.more) {
            unsigned msg_type = coap_get_type(pdu);
            if (block.blknum == 0 && !strlen(_last_req_path)) {
                puts("Path too long; can't complete blockwise");
                return;
            }

            gcoap_req_init(pdu, (uint8_t *)pdu->hdr, CONFIG_GCOAP_PDU_BUF_SIZE,
                           COAP_METHOD_GET, _last_req_path);
            if (msg_type == COAP_TYPE_ACK) {
                coap_hdr_set_type(pdu->hdr, COAP_TYPE_CON);
            }
            block.blknum++;
            coap_opt_add_block2_control(pdu, &block);
            int len = coap_opt_finish(pdu, COAP_OPT_FINISH_NONE);
            gcoap_req_send((uint8_t *)pdu->hdr, len, remote,
                           _resp_handler, memo->context);
        }
        else {
            puts("--- blockwise complete ---");
        }
    }
}

// Has lots of impact from RIOT course CoAP example
static size_t _send_coap(uint8_t *buf, size_t len)
{
    ipv6_addr_t addr;
    sock_udp_ep_t remote;

    remote.family = AF_INET6;

    /* parse for interface */
    char *iface = ipv6_addr_split_iface(COAP_SERVER);
    if (!iface) 
    {
        if (gnrc_netif_numof() == 1) 
        {
            /* assign the single interface found in gnrc_netif_numof() */
            remote.netif = (uint16_t)gnrc_netif_iter(NULL)->pid;
        }
        else 
        {
            remote.netif = SOCK_ADDR_ANY_NETIF;
        }
    }
    else 
    {
        int pid = atoi(iface);
        if (gnrc_netif_get_by_pid(pid) == NULL) {
            puts("Interface not valid");
            return 0;
        }
        remote.netif = pid;
    }

    if (ipv6_addr_from_str(&addr, COAP_SERVER) == NULL) {
        puts("Unable to parse destination IPv6 address");
        return 0;
    }

    memcpy(&remote.addr.ipv6[0], &addr.u8[0], sizeof(addr.u8));

    remote.port = CONFIG_GCOAP_PORT;

    return gcoap_req_send(buf, len, &remote, _resp_handler, NULL);
}

size_t coap_handler_impl(void)
{
    uint8_t buf[CONFIG_GCOAP_PDU_BUF_SIZE];
    coap_pkt_t pdu;
    int len = 0;
    char msg[5] = "Test";

    // @TODO Hardcoded PUT and URI. Make something more general
    gcoap_req_init(&pdu, &buf[0], CONFIG_GCOAP_PDU_BUF_SIZE, COAP_METHOD_PUT, "/large-update");

    // Needed if we want the request be confirmable
    // coap_hdr_set_type(pdu.hdr, COAP_TYPE_CON)

    coap_opt_add_format(&pdu, COAP_FORMAT_TEXT);

    len = coap_opt_finish(&pdu, COAP_OPT_FINISH_PAYLOAD);
    if (pdu.payload_len >= sizeof(msg))
    {
        memcpy(pdu.payload, &msg[0], sizeof(msg));
        len += sizeof(msg);
    }
    else
    {
        puts("Msg buffer too small");
        return 1;
    }

    return _send_coap(&buf[0], len);
}