## Minimum viable example to get the BLE Module to Work through Python's BLEAK api

from bleak import BleakScanner, BleakClient
from icecream import ic
import asyncio

found_devices = []
CHARACTERISTIC_UUID = "0000ffe1-0000-1000-8000-00805f9b34fb" 

async def run():
    devices = await BleakScanner.discover()
    for d in devices:
        if(d.name is not None):
            found_devices.append(d)

async def main(value):
    hm10_address = 0
    for device in found_devices:
        if(device.name == "ZhipengBL"):
            hm10_address = device.address
    if(hm10_address == 0):
        exit("No devices found with name ZhipengBL")
    ic(hm10_address)
    async with BleakClient(hm10_address) as client:
        ic(client.read_gatt_char)
        command_bytes = value.to_bytes(1, byteorder='little')
        await client.write_gatt_char(CHARACTERISTIC_UUID, command_bytes)
        print(f"Command {value} sent to HM10")

loop = asyncio.get_event_loop()
loop.run_until_complete(run())

asyncio.run(main(99))


