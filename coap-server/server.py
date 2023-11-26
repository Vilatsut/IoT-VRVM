import os
import asyncio
import sqlite3


import aiocoap
import aiocoap.resource as resource
from aiocoap.numbers.codes import Code
from aiocoap.numbers.contentformat import ContentFormat

DB_Name =  "sensor.db"

if not os.path.isfile(DB_Name):
    conn = sqlite3.connect(DB_Name)
    c = conn.cursor()
    c.execute("""CREATE TABLE data (
        Id INTEGER PRIMARY KEY AUTOINCREMENT,
        date_time text,
        mac text,
        field integer,
        data real
        )""")
    conn.commit()
    conn.close()

class DatabaseManager():
    """Class for managing the sqlite3 database containing the temperature readings"""

    def __init__(self):
        self.conn = sqlite3.connect(DB_Name)
        self.conn.execute('pragma foreign_keys = on')
        self.conn.commit()
        self.cur = self.conn.cursor()

    def process_db(self, sql_query, args=()):
        self.cur.execute(sql_query, args)
        self.conn.commit()

    def __del__(self):
        self.cur.close()
        self.conn.close()

class Temperature(resource.Resource):
    """Resource for PUTting temperature data into the database"""

    def __init__(self):
        super().__init__()
        self.content = ""

    async def render_put(self, message):
        print(f"PUT payload: {message.payload}")
        dbm = DatabaseManager()
        dbm.process_db("INSERT INTO data (API_key, date_time, mac, field, data) VALUES (?,?,?,?,?)",
            (
                message.opt.uri_query["date_time"],
                message.opt.uri_query["mac"],
                message.opt.uri_query["field"],
                message.opt.uri_query["data"]
                ))
        del dbm
        return aiocoap.Message(code=Code.CHANGED, payload=message.payload)


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
