APPLICATION = vrvm-node

BOARD ?= iotlab-m3

RIOTBASE ?=$(CURDIR)/../RIOT

DEVELHELP ?= 1


COAP_SERVER ?= 2a01:111:f100:8000::4134:941d
CFLAGS += -DCOAP_SERVER='"$(COAP_SERVER)"'

# Utilities
USEMODULE += ztimer_msec
USEMODULE += shell
USEMODULE += shell_cmds_default

# Sensors
USEMODULE += lps331ap

# Networking
USEMODULE += netdev_default
USEMODULE += auto_init_gnrc_netif

# Activate ICMPv6 error messages
USEMODULE += gnrc_icmpv6_error

# Specify the mandatory networking module for a IPv6 routing node
USEMODULE += gnrc_ipv6_router_default

# Add a routing protocol
USEMODULE += gnrc_rpl
USEMODULE += auto_init_gnrc_rpl

# Additional networking modules that can be dropped if not needed
USEMODULE += gnrc_icmpv6_echo
USEMODULE += shell_cmd_gnrc_udp

# Enable DNS
USEMODULE += sock_dns

# Add also the shell, some shell commands
USEMODULE += ps
USEMODULE += netstats_l2
USEMODULE += netstats_ipv6
USEMODULE += netstats_rpl

USEMODULE += od
USEMODULE += fmt

# COAP
USEMODULE += gcoap

# External packages
USEPKG += nanopb

USE_ZEP ?= 0

# set the ZEP port for native (not sure is this really needed in this application)
ZEP_PORT_BASE ?= 17754
ifeq (1,$(USE_ZEP))
  TERMFLAGS += -z [::1]:$(ZEP_PORT_BASE)
  USEMODULE += socket_zep

  ifneq (,$(ZEP_MAC))
    TERMFLAGS += --eui64=$(ZEP_MAC)
  endif
endif

include $(RIOTBASE)/Makefile.include

include $(RIOTMAKE)/default-radio-settings.inc.mk