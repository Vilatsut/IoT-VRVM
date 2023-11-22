#include <stdio.h>

#include "lpsxxx.h"
#include "lpsxxx_params.h"

#include "ztimer.h"
#include "shell.h"
#include "mutex.h"
#include "msg.h"

#include "net/gcoap.h"
#include "net/sock/util.h"

#include "od.h"

// Global variables
static lpsxxx_t lpsxxx;

#define MAIN_QUEUE_SIZE     (8)
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];

#define _LAST_REQ_PATH_MAX (32)
static char _last_req_path[_LAST_REQ_PATH_MAX];


// Taken from RIOT-OS course examples
static void _resp_handler (const gcoap_request_memo_t *memo, coap_pkt_t *pdu,
                            const sock_udp_ep_t *remote)
{
    (void)memo;
    (void)pdu;
    (void)remote;

    puts("COAP response received");

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

static int coap_handler(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

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

    _send_coap(&buf[0], len);

    return 0;
}

static int sensor_handler(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    int16_t temperature = 0;
    uint16_t pressure = 0;

    if (lpsxxx_read_temp(&lpsxxx, &temperature) != LPSXXX_OK)
    {
        puts("LPS331AP temperature read failed");
    }

    if (lpsxxx_read_pres(&lpsxxx, &pressure) != LPSXXX_OK)
    {
       puts("LPS331AP pressure read failed");
    }

    printf("Temperature %i.%u°C, Pressure %uhPa \n", temperature / 100, 
            temperature % 100, pressure);
    return 0;
}

static const shell_command_t shell_commands[] =
{
    { "sensors", "Print lps331ap values", sensor_handler },
    { "sensor_coap", "Send sensor values as POST request over CoAP to predefined IPv6 address", coap_handler},
    { NULL, NULL, NULL},
};

int main(void)
{
    puts("Application starts");

    if (lpsxxx_init(&lpsxxx, &lpsxxx_params[0]) != LPSXXX_OK)
    {
        puts("LPS331AP init failed");
    }
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}