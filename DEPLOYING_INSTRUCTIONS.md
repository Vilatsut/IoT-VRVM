# Deploying instructions

These instrcutions will go through how to deploy the project. The sensor nodes and border routet will be run at the iot-lab.info IoT-testbed, and the backend running at a cloud that provides a public IPv6 address.

## Cloning the repo

RIOT OS is included to this repository as a submodule. After cloning this repository the submodules must be downloaded using the command

```
git submodule update --init
```

Check out to correct branch (release 2023.07 is used)

```
cd RIOT
git checkout 2023.07-branch
```

Roll back to previous version of LPSXXX driver. This is because the current version of the driver seems to be buggy as it return negative values if using the develop branch and values like 170C if using the release branch.

```
git checkout 947efa7 drivers/lpsxxx/
```


## Install python dependencies

It is recommended (but not neccessary) to make a seperated python virtual environment. To install python dependencies run the following under 'root' directory:

```
pip install -r requirements.txt
```

## Building firmware

Correct tools must be instelled to build the RIOT OS firmware. Using Ubuntu-based distro the required tools can be installed using the following commands:

```
sudo apt install git gcc-arm-none-eabi make gcc-multilib libstdc++-arm-none-eabi-newlib openocd gdb-multiarch doxygen wget unzip python3-serial protobuf-compiler
```

### Building locally to check for errors etc.
To build the firmware for target iotlab-m3 run the following under the 'node' directory:

```
make all
```

To build the firmware for linux (native) use the following command:

```
make all BOARD=native
```

## Running experiment in iot-lab.info

### Spawning nodes in iot-lab
It's required to have access to iot-lab.info to run the experiment. Install iotlab-cli and authenticate to the service (refer to IoT-lab documentation). Run the experiment with the following commands
```
iotlab-experiment submit -n "node" -d 120 -l 2,archi=m3:at86rf231+site=grenoble
iotlab-experiment wait --timeout 30 --cancel-on-timeout
iotlab-experiment --jmespath="items[*].network_address | sort(@)" get --nodes
```

### Build the coap node on iot-lab
To run the client node with the default COAP server address specified in the Makefile, use command:
```
make BOARD=iotlab-m3 DEFAULT_CHANNEL=17 DEFAULT_PAN_ID=0x34EF IOTLAB_NODE=m3-<ID>.grenoble.iot-lab.info flash
```
To run it with a different COAP_SERVER address, change the value in the 'Makefile' or give COAP_SERVER flag as follows:
```
make BOARD=iotlab-m3 COAP_SERVER=<IPv6_address> DEFAULT_CHANNEL=17 DEFAULT_PAN_ID=0x34EF IOTLAB_NODE=m3-<ID>.grenoble.iot-lab.info flash
```
### Build the border router node
To flash the border router node, use command:
```
make -C ../RIOT/examples/gnrc_border_router/ BOARD=iotlab-m3 ETHOS_BAUDRATE=500000 DEFAULT_CHANNEL=17 DEFAULT_PAN_ID=0x34EF IOTLAB_NODE=m3-<ID2>.grenoble.iot-lab.info flash
```
### Sanity check connection
Open two SSH connections to IoT-lab SSH-frontend:
```
ssh <login>@grenoble.iot-lab.info
```
In the first one give following command:
```
nc m3-<ID> 20000
```
In the other SSH session give command:
```
sudo ethos_uhcpd.py m3-<ID2> tap3 2001:660:5307:3103::/64
```
It's possible to run the experiment at another site, and current DEFAULT_CHANNEL, DEFAULT_PAN_ID, tap3 and IPv6 address can be in use. Refer to [IOT-LAB IPv6 example](https://www.iot-lab.info/learn/tutorials/riot/riot-public-ipv6-m3/) if something is broken.

### Example outputs

```shell
user@grenoble:~$ nc m3-98 20000
gcoap: timeout for msg ID 6101
gcoap: timeout for msg ID 6105
gcoap: response Success, code 2.04, empty payload
```
```shell
user@grenoble:~$ sudo ethos_uhcpd.py m3-99 tap3 2001:660:5307:3103::/64
net.ipv6.conf.tap3.forwarding = 1
net.ipv6.conf.tap3.accept_ra = 0
Switch from 'root' to 'user'
Joining IPv6 multicast group...
entering loop...
Switch from 'root' to 'user'
----> ethos: sending hello.
----> ethos: activating serial pass through.
----> ethos: hello reply received
----> ethos: hello reply received
uhcp_client(): no reply received
got packet from <ipv6> port 12345
```
## Building, configuring and running the backend

Requirements for the virtual machine (VM) running in the cloud service:

<ul>
    <li>Ubuntu 22.04</li>
    <li>Public IPv6 address</li>
    <li>Open ports: 8086 (InfluxDb), 3000 (Grafana), 5683 (CoAP), 22 (SSH for setuping)
</ul>

We don't cover how to deploy and setup the VM to the cloud provider, as many different cloud services can be used and everyone has different ways to deploy the VM. Refer to your cloud providers documentation. We used AWS EC2 cloud.

SSH to your VM. Clone the repository to the VM.

If Docker isn't installed, install it using these instructions: https://docs.docker.com/engine/install/ubuntu/#install-using-the-repository

Make copies from the compose.yaml.example and coap-server/influxdb_config.ini.example files and remove .example suffixes. This .example pattern is used not to leak any credentials to git repository in accident. In compose.yaml file change variables DOCKER_INFLUXDB_INIT_USERNAME and DOCKER_INFLUXDB_INIT_PASSWORD as you like. 

To build and run the backend in whole (CoAP backend + influxdb + Grafana) run the following command:
```
docker compose up
```

Backend isn't fully functional yet. We need to generate influxDB API tokens to authenticate our services to access the database.

### Generate influxDB API token

Log in to http://localhost:8086 (or your cloud IP instead of localhost) and log in to influxDB using credentials configured in compose.yaml. Click "Arrow Up" icon at the left navigation bar -> "API Tokens". Click "Generate API Token" and then "All Access API token". Write "InfluxDB" to description and click "Save". Copy the API Token to the coap-server/influxdb_config.ini file. Run the following commands:

```
docker compose down
docker compose up
```

> NOTE! It's bad habit to generate an all-access token for a service that just requires write access to the database. It's used here to simplify things as this is a proof-of-consept design for course project, but don't do this in production as it can lead to security issues.

### Configure Grafana

Connect the database to grafana:

Log in to http://localhost:3000 (or your cloud IP instead of localhost). Use the default log-in credentials (user: admin, password: admin). 

Open the menu on top left and click "Connections". Search for "InfluxDB" and add it as a "New Data Source". 
Select Query Language as "FLUX". 

Enter URL "http://influxdb:8086" and enable "Basic auth". (// may need to test if connecting to cloud IP instead. Remove this comment if it works when connecting to cloud IP!!!).

Enter InfluxDB's username and password. Then input the organization's name (by default it's "my-org") and enter the token we set-up in InfluxDB previously. For the Default Bucket enter "iot-course".

Click "Save & Test".

(Dashboard instructions will be added later)
