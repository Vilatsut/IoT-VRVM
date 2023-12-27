# IoT-VRVM

RIOT and FIT-IOT-LAB based example project for learning.

## Introduction to project

The purpose of the project is to implement a whole IoT pipeline from sensors to cloud. IoT Testbed ([iot-lab.info](https://iot-lab.info)) is used to deploy the sensor nodes in a real environment with real hardware. Sensor nodes uses 6LoWPAN to communicate with a border router (also deployed in the IoT Testped) and the border router routes the packages from sensor nodes to internet.

![Infrastructure](/images/infrastructure.png)
Image 1. The infrastructure of the IoT pipeline.

## Sensor nodes

Sensor nodes uses [RIOT OS](https://www.riot-os.org/) as the operating system. RIOT OS provides lots of the required features (device drivers, communication stacks, board files...) that simplifies the firmware development. Firmware is written using the C programming language.

Sensors sends current temperature and air pressure to the cloud. Sensor nodes use CoAP (Constrained Application Protocol) to send the sensor data. Sensor data is encoded using Google protobuf protocol to ensure that no unneccessary overhead is used in the communication.

### IoT-LAB M3

The board used to run the sensor node firmware is IoT-LAB M3. It is designed for the IoT-LAB testped and is based on ARM Cortex M3 micro-controller. It has Atmel AT86RF231 radio chip that provides wireless connectivity to the board. IoT-LAB M3 has 4 different sensors connected over an I2C bus, and we are using one of them (LPS331AP) in our firmware. For more information about the IoT-LAB M3 board refer to [IoT-LAB documentation](https://www.iot-lab.info/docs/boards/iot-lab-m3/).

## Border router

Border router uses the same IoT-LAB M3 board as the actual sensor nodes. The difference is that it is running a different firmware. The border router receives ethernet packets from the sensors over the 6LoWPAN and routes them to the internet. As IoT-LAB M3 doesn't have an ethernet interface, it uses the serial interface to forward the packets. The computer controlling the border router is running a counterpart software that receives ethernet packets from the serial interface and forwards them to the internet via ethernet interface.

## Cloud

Cloud is running in a virtual machine that is hosted in the [AWS](https://aws.amazon.com/). It is built using a microservices architecture and consists of three different services: CoAP backend, Influxdb and Grafana. CoAP backend receives CoAP messages from the sensor nodes and decodes the protobuf encoded payloads and then it sends the decoded data to the Influxdb. The stored data can be it can be querried from the influxDB and visualized by using the Grafana.