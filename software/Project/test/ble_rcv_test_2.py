import asyncio
import time
from bleak import BleakScanner, BleakClient
from bleak.exc import BleakError

print('ble receiver test')

target_name = 'Ard_BLE_33_S'
svc_uuid = '00001234-0000-1000-8000-00805f9b34fb'
read_chr_uuid = '00001111-0000-1000-8000-00805f9b34fb'
write_chr_uuid = '00002222-0000-1000-8000-00805f9b34fb'
write_base = bytearray([0xA0])


class Connection:
    client: BleakClient = None

    def __init__(
            self,
            loop: asyncio.AbstractEventLoop,
            read_chr_uuid: str,
            write_chr_uuid: str
    ):
        self.loop = loop
        self.read_chr = read_chr_uuid
        self.write_chr = write_chr_uuid

        self.connected = False
        self.connected_device = None

    def on_disconect(self, client: BleakClient):
        self.connected = False
        print("disconnected from: " + self.connected_device.name)

    def on_notify(self, sender: str, data):
        print("got notification: " + str(data) + " from " + sender)

    async def manager(self):
        print("starting connection manager")
        while True:
            if self.client:
                await self.connect()
            else:
                await self.select_device()
                await asyncio.sleep(15.0, loop=loop)

    async def connect(self):
        if self.connected:
            return
        try:
            await self.client.connect()
            self.connected = await self.client.is_connected()
            if self.connected:
                print("connected to " + self.connected_device.name)
                self.client.set_disconnected_callback(self.on_disconect)
                await self.client.start_notify(
                    self.read_chr_uuid, self.on_notify
                )
                while True:
                    if not self.connected:
                        break
                    await asyncio.sleep(5.0, loop=loop)
            else:
                print("failed to connect to " + str(self.connected_device.name))
        except Exception as e:
            print(e)

    async def cleanup(self):
        if self.client:
            await self.client.stop_notify(read_chr_uuid)
            await self.client.disconnect()

    async def select_device(self):
        print("Bluetooth LE HW warmup")
        await asyncio.sleep(2.0, loop=loop)
        device = await BleakScanner.find_device_by_filter(
            lambda d, ad: d.name and d.name.lower() == target_name.lower()
        )
        self.connected_device = device
        self.client = BleakClient(device.address, loop=self.loop)


if __name__ == "__main__":
    loop = asyncio.get_event_loop()
    connection = Connection(loop, read_chr_uuid, write_chr_uuid)
    try:
        asyncio.ensure_future(main())
        asyncio.ensure_future(connection.manager())
        asyncio.ensure_future(user_console_manager(connection))
        loop.run_forever()
    except KeyboardInterrupt:
        print()
        print("user stopped program")
    finally:
        print("disconnecting")
        loop.run_until_complete(connection.cleanup())
