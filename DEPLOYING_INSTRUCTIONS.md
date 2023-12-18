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

## Building firmware

Correct tools must be instelled to build the RIOT OS firmware. Using Ubuntu-based distro the required tools can be installed using the following commands:
```
sudo apt install git gcc-arm-none-eabi make gcc-multilib libstdc++-arm-none-eabi-newlib openocd gdb-multiarch doxygen wget unzip python3-serial
```
To install python dependencies run the following under 'node' directory:
```
pip install -r requirements.txt
```
To build the firmware for target iotlab-m3 run the following under the 'node' directory:
```
make all
```
To build the firmware for linux (native) use the following command:
```
make all BOARD=native
```

## Running experiment in iot-lab.info

It's required to have access to iot-lab.info to run the experiment. Install iotlab-cli and authenticate to the service (refer to IoT-lab documentation). Run the experiment with the following commands
```
iotlab-experiment submit -n "node" -d 120 -l 2,archi=m3:at86rf231+site=grenoble
iotlab-experiment wait --timeout 30 --cancel-on-timeout
iotlab-experiment --jmespath="items[*].network_address | sort(@)" get --nodes
``` 
```
make BOARD=iotlab-m3 DEFAULT_CHANNEL=17 DEFAULT_PAN_ID=0x34EF IOTLAB_NODE=m3-<ID>.grenoble.iot-lab.info flash

make -C ../RIOT/examples/gnrc_border_router/ BOARD=iotlab-m3 ETHOS_BAUDRATE=500000 DEFAULT_CHANNEL=17 DEFAULT_PAN_ID=0x34EF IOTLAB_NODE=m3-<ID2>.grenoble.iot-lab.info flash
```
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
It's possible to run the experiment at another site, and current DEFAULT_CHANNEL, DEFAULT_PAN_ID, tap3 and IPv6 can be in use. Refer to RIOT-course IPv6 example if something is broken.

## Building and running the backend

Requirements for the virtual machine (VM) running in the cloud service:

<ul>
    <li>Ubuntu 22.04</li>
    <li>Public IPv6 address</li>
    <li>Open ports: 8086 (InfluxDb), 3000 (Grafana), 5683 (CoAP), 22 (SSH for setuping)
</ul>

We don't cover how to deploy and setup the VM to the cloud provider, as many different cloud services can be used and everyone has different ways to deploy the VM. Refer to your cloud providers documentation. We used AWS EC2 cloud.

SSH to your VM. Clone the repository to the VM.

If Docker isn't installed, install it using these instructions: https://docs.docker.com/engine/install/ubuntu/

To build and run the backend in whole (CoAP backend + influxdb + Grafana) run the following command:
```
docker compose up
```
