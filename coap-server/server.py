import os
import asyncio
from datetime import datetime

# Protocol buffer 
import message_pb2

import aiocoap
import aiocoap.resource as resource
from aiocoap.numbers.codes import Code
from aiocoap.numbers.contentformat import ContentFormat

from influxdb_client import InfluxDBClient, Point, WritePrecision
from influxdb_client.client.write_api import SYNCHRONOUS

BUCKET = "iot-course"

class Temperature(resource.Resource):
    """Resource for PUTting temperature data into the database"""

    def __init__(self):
        super().__init__()
        self.content = ""

    async def render_put(self, message):
        pb_sensorvalues = message_pb2.SensorValues()
        print(f"PUT payload: {message.payload}")

        pb_sensorvalues.ParseFromString(message.payload)
        print(f"Protobuf: {str(pb_sensorvalues)}")

        p = Point("measurement").tag("location", "Grenoble") \
            .field("sensor-id", pb_sensorvalues.sensor_id).field("temperature", pb_sensorvalues.temperature) \
            .field("pressure", pb_sensorvalues.pressure)

        with InfluxDBClient.from_config_file("influxdb_config.ini") as client:
            with client.write_api() as writer:
                writer.write(bucket=BUCKET, record=p)

        return aiocoap.Message(code=Code.CHANGED)


async def main():
    """Set up the server and run forever"""

    root = resource.Site()

    root.add_resource(['.well-known', 'core'],
            resource.WKCResource(root.get_resources_as_linkheader))
    root.add_resource(["temperature"], Temperature())

    await aiocoap.Context.create_server_context(root)

    # Run forever
    await asyncio.get_running_loop().create_future()


if __name__ == "__main__":
    asyncio.run(main())
