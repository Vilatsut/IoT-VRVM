import asyncio
import sqlite3

import logging

import aiocoap
import aiocoap.resource as resource
from aiocoap.numbers.contentformat import ContentFormat

DB_Name =  "sensor.db"

class DatabaseManager():
    """Class for managing the sqlite3 database containing the temperature readings"""

    def __init__(self):
	    self.conn = sqlite3.connect(DB_Name)
	    self.conn.execute('pragma foreign_keys = on')
	    self.conn.commit()
	    self.cur = self.conn.cursor()

    def add_del_update_db_record(self, sql_query, args=()):
	    self.cur.execute(sql_query, args)
	    self.conn.commit()
	    return

    def __del__(self):
	    self.cur.close()
	    self.conn.close()

class BlockResource(resource.Resource):
    """Resource for PUTting temperature data into the database"""

    def __init__(self):
        super().__init__()
        self.content = ""

    async def render_put(self, request):
        print("PUT payload: %s" % request.payload)
        return aiocoap.Message(code=aiocoap.CHANGED, payload=self.request.payload)


logging.basicConfig(level=logging.INFO)
logging.getLogger("coap-server").setLevel(logging.DEBUG)


async def main():
    """Set up the server and run forever"""

    root = resource.Site()

    root.add_resource(["temperature"], BlockResource())

    await aiocoap.Context.create_server_context(root)

    # Run forever
    await asyncio.get_running_loop().create_future()


if __name__ == "__main__":
    asyncio.run(main())
