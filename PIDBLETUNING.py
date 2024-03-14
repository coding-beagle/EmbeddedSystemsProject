# this is a GUI BLE app for interacting with the buggy over BLE
# the bulk of complexity stems from the BLEAK API expecting async calls,
# and the gui library of tkinter having to deal with this
# to get around this we use multithreading and the asyncio library
# also a bit of regexing for completenesssjokdfojkjj'd

from bleak import BleakScanner, BleakClient
from icecream import ic
import asyncio
import customtkinter as ctk
import threading
import tkinter as tk
import re
import math
import struct

found_devices = []
CHARACTERISTIC_UUID = "0000ffe1-0000-1000-8000-00805f9b34fb" 

# ble handling functionality

class BLEDeviceManager:
    def __init__(self, device_name, response_callback = None):
        self.device_name = device_name
        self.client = None
        self.connected = False
        self.response_callback = response_callback
        self.consoleReference = None
        self.rps1 = 0.0
        self.rps2 = 0.0

    async def notification_handler(self, sender, data):
        # convert data to a string or process it as needed
        # check if the message matches the expected RPS format
        try:
            message = data.decode('utf-8')
        except UnicodeDecodeError:
            message = "Bad decoding!"

        if self.response_callback:
            self.response_callback(message)

    async def connect(self):
        if self.client is None:
            self.consoleReference.insert(tk.END, f"\nFinding devices\n")
            self.consoleReference.see(tk.END)
            devices = await BleakScanner.discover()
            self.consoleReference.insert(tk.END, f"\nFound devices:\n")

            for found_device in devices:
                self.consoleReference.insert(tk.END, f"Device: {found_device.name}\n")

            device_address = next((d.address for d in devices if d.name == self.device_name), None)
            if device_address is not None:
                self.client = BleakClient(device_address)
                self.consoleReference.insert(tk.END, f"\nDevice found at: {device_address}")
                self.consoleReference.see(tk.END)
        if not self.connected:
            try:
                self.consoleReference.insert(tk.END, f"\nConnecting to: {device_address}")
                await self.client.connect()
                self.connected = await self.client.is_connected()
                self.consoleReference.insert(tk.END, "\nSuccesfully Connected!")
                self.consoleReference.insert(tk.END, "\nGet BLEing!")
                self.consoleReference.see(tk.END)
                if self.connected:
                # sub to notifs
                    await self.subscribe_to_notifications("0000ffe1-0000-1000-8000-00805f9b34fb", self.notification_handler)
            except Exception as e:
                self.consoleReference.insert(tk.END, f"\nDevice Not Found!")
                self.consoleReference.see(tk.END)
                print(f"Failed to connect to the device: {e}")

    async def disconnect(self):
        if self.client is not None and self.connected:
            self.consoleReference.insert(tk.END, "\n\nAttempting to disconnect")
            self.consoleReference.see(tk.END)
            await self.client.disconnect()
            self.connected = False
            self.consoleReference.insert(tk.END, "\nSuccesfully Disconnected!")
            self.consoleReference.see(tk.END)
            print("Disconnected from the BLE device.")
            self.client = None

    async def send_command(self, command_value, delay_after=0.0):
        if self.connected:
            command_bytes = command_value.to_bytes(1, byteorder='little')
            await self.client.write_gatt_char(CHARACTERISTIC_UUID, command_bytes)
            print(f"Command {command_value} sent.")
            if delay_after > 0:
                await asyncio.sleep(delay_after)  # wait for a specified delay after sending the command
        else:
            print("Device is not connected.")

    async def subscribe_to_notifications(self, characteristic_uuid, handler):
        if self.connected:
            await self.client.start_notify(characteristic_uuid, handler)
            print(f"Subscribed to notifications for {characteristic_uuid}")
        else:
            print("Device is not connected.")
    
    async def send_command_with_argument(self, command, arg=None, delay_between=0.1):
        # ensure there's a connection
        if not self.connected:
            print("Device not connected.")
            return
        
        # convert and send the command
        command_bytes = command.to_bytes(1, byteorder='little')
        await self.client.write_gatt_char(CHARACTERISTIC_UUID, command_bytes)
        print(f"Command {command} sent.")
        
        # if an argument is provided, wait a bit and then send it
        if arg is not None:
            if(isinstance(arg, int)):
                await asyncio.sleep(delay_between)  # wait for the device to process the command
                arg_bytes = arg.to_bytes(1, byteorder='little')
                await self.client.write_gatt_char(CHARACTERISTIC_UUID, arg_bytes)
                print(f"Argument {arg} sent.")
            if(isinstance(arg, list)):
                print(f"Called, {arg}")
                for i in arg:
                    await asyncio.sleep(delay_between)  # wait for the device to process the command
                    arg_bytes = i.to_bytes(1, byteorder='little')
                    await self.client.write_gatt_char(CHARACTERISTIC_UUID, arg_bytes)
                print(f"Argument {arg} sent.")

loop = None  # global variable to hold the loop reference 
# used for debuggging multithreadingjfnjkhjkljlkopdgdkln

def start_asyncio_loop():
    global loop
    print("Starting asyncio loop in a separate thread.")
    loop = asyncio.new_event_loop()
    asyncio.set_event_loop(loop)
    loop.run_forever()

def run_coroutine_threadsafe(coroutine):
    global loop
    if loop is None:
        print("Event loop is not running.")
        return
    asyncio.run_coroutine_threadsafe(coroutine, loop)

# gui functionality

class Root(ctk.CTk):     
    def on_close(self):
        # to make sure all threads close correctly
        # schedule the disconnect coroutine and wait for it to complete
        asyncio.run_coroutine_threadsafe(self.ble_manager.disconnect(), asyncio.get_event_loop())
        # give a moment for the disconnect coroutine to initiate
        asyncio.get_event_loop().call_later(5, self.destroy)
        exit()

    def update_response(self, response):
        # adds response of ble to console window
        self.TBcommandLog.insert(tk.END, f"\nResponse: {response}")
        self.TBcommandLog.see(tk.END)
    
    def send_command_with_text(self, number, string):
        self.TBcommandLog.insert(tk.END ,f"\n{string}")
        self.TBcommandLog.see(tk.END)
        run_coroutine_threadsafe(self.ble_manager.send_command(number))

    def send_entry(self, e=None):
        value_to_send = self.commandEntry.get()
        self.TBcommandLog.insert(tk.END , f"\nSending: {int(value_to_send)}")
        self.TBcommandLog.see(tk.END)
        run_coroutine_threadsafe(self.ble_manager.send_command(int(value_to_send)))

    def float_to_bytes_le(self, f):
        # Convert the float to 4 bytes in little-endian format
        bytes_le = struct.pack('<f', f)
        return list(struct.unpack('4B', bytes_le))

    async def send_motor_speeds_async(self, val_1, val_2):
        # await self.ble_manager.send_command(95)
        await self.ble_manager.send_command(34, 0.1)
        await self.ble_manager.send_command_with_argument(40, val_1, 0.3)
        await self.ble_manager.send_command_with_argument(50, val_2, 0.3)
        await self.ble_manager.send_command(33, 0.1)
        # await self.ble_manager.send_command(95)

    def send_motor_speeds(self):
        input_1 = self.float_to_bytes_le(float(self.entryMotor1Speed.get()))
        input_2 = self.float_to_bytes_le(float(self.entryMotor2Speed.get()))

        self.TBcommandLog.insert(tk.END , f"\nMotor 1 Speed = {float(self.entryMotor1Speed.get())}")
        self.TBcommandLog.insert(tk.END , f"\nMotor 2 Speed = {float(self.entryMotor2Speed.get())}")
        self.TBcommandLog.see(tk.END)

        run_coroutine_threadsafe(self.send_motor_speeds_async(input_1, input_2))

    async def send_motor1_PIDs_async(self, val_1, val_2, val_3):
        await self.ble_manager.send_command(34, 0.1)
        await self.ble_manager.send_command_with_argument(44, val_1, 0.2)
        await self.ble_manager.send_command_with_argument(45, val_2, 0.2)
        await self.ble_manager.send_command_with_argument(46, val_3, 0.2)
        await self.ble_manager.send_command(33, 0.1)

    def send_motor1_PIDs(self):
        input_1 = self.float_to_bytes_le(float(self.entryP1.get()))
        input_2 = self.float_to_bytes_le(float(self.entryI1.get()))
        input_3 = self.float_to_bytes_le(float(self.entryD1.get()))

        self.TBcommandLog.insert(tk.END , f"\nSetting P1 = {(self.entryP1.get())}")
        self.TBcommandLog.insert(tk.END , f"\nSetting I1 = {(self.entryI1.get())}")
        self.TBcommandLog.insert(tk.END , f"\nSetting D1 = {(self.entryD1.get())}")
        self.TBcommandLog.see(tk.END)

        run_coroutine_threadsafe(self.send_motor1_PIDs_async(input_1, input_2, input_3))

    async def send_motor2_PIDs_async(self, val_1, val_2, val_3):
        await self.ble_manager.send_command(34, 0.1)
        await self.ble_manager.send_command_with_argument(54, val_1, 0.2)
        await self.ble_manager.send_command_with_argument(55, val_2, 0.2)
        await self.ble_manager.send_command_with_argument(56, val_3, 0.2)
        await self.ble_manager.send_command(33, 0.1)

    def send_motor2_PIDs(self):
        input_1 = self.float_to_bytes_le(float(self.entryP2.get()))
        input_2 = self.float_to_bytes_le(float(self.entryI2.get()))
        input_3 = self.float_to_bytes_le(float(self.entryD2.get()))

        self.TBcommandLog.insert(tk.END , f"\nSetting P2 = {(self.entryP2.get())}")
        self.TBcommandLog.insert(tk.END , f"\nSetting I2 = {(self.entryI2.get())}")
        self.TBcommandLog.insert(tk.END , f"\nSetting D2 = {(self.entryD2.get())}")
        self.TBcommandLog.see(tk.END)

        run_coroutine_threadsafe(self.send_motor2_PIDs_async(input_1, input_2, input_3))
    
    async def send_navigational_PIDs_async(self, val_1, val_2, val_3):
        await self.ble_manager.send_command(34, 0.1)
        await self.ble_manager.send_command_with_argument(64, val_1, 0.2)
        await self.ble_manager.send_command_with_argument(65, val_2, 0.2)
        await self.ble_manager.send_command_with_argument(66, val_3, 0.2)
        await self.ble_manager.send_command(33, 0.1)

    def send_navigational_PIDs(self):
        input_1 = self.float_to_bytes_le(float(self.entryP3.get()))
        input_2 = self.float_to_bytes_le(float(self.entryI3.get()))
        input_3 = self.float_to_bytes_le(float(self.entryD3.get()))

        self.TBcommandLog.insert(tk.END , f"\nSetting P3 = {(self.entryP3.get())}")
        self.TBcommandLog.insert(tk.END , f"\nSetting I3 = {(self.entryI3.get())}")
        self.TBcommandLog.insert(tk.END , f"\nSetting D3 = {(self.entryD3.get())}")
        self.TBcommandLog.see(tk.END)

        run_coroutine_threadsafe(self.send_navigational_PIDs_async(input_1, input_2, input_3))

    def set_speeds_and_send(self, speed_1, speed_2):
        self.entryMotor1Speed.delete(0, tk.END)
        self.entryMotor2Speed.delete(0, tk.END)

        self.entryMotor1Speed.insert(0, speed_1)
        self.entryMotor2Speed.insert(0, speed_2)

        self.send_motor_speeds()


    def __init__(self):
        super().__init__()
        self.enabled = True
        self.ble_manager = BLEDeviceManager("ZhipengBL")
        self.ble_manager.response_callback = self.update_response
        
        self.protocol("WM_DELETE_WINDOW", self.on_close)

        ## Geometry and Theme Settings
        ctk.set_appearance_mode("dark")   
        ctk.set_default_color_theme("dark-blue")
    
        self.geometry("500x500")
        self.title("WQY BLE")

        self.resizable(False, False)
    
        self.labelTitle = ctk.CTkLabel(self,text="WQY BLE APP",font=('roboto', -30))
        self.labelTitle.place(x=20, y=15)
    
        self.TBcommandLog = ctk.CTkTextbox(self,height=220)
        self.TBcommandLog.place(x=20.0, y=90)
        self.TBcommandLog.bind("<Key>", lambda e: "break")

        self.ble_manager.consoleReference = self.TBcommandLog

        self.labelConsoleLog = ctk.CTkLabel(self,text="Console Log")
        self.labelConsoleLog.place(x=20, y=60)
    
        self.commandEntry = ctk.CTkEntry(self)
        self.commandEntry.place(x=20, y=320)
        self.commandEntry.bind("<Return>", self.send_entry)
    
        ## PID 1 ADJUSTERS
        self.buttonSendCommand = ctk.CTkButton(self,text="SEND",width=30, command=self.send_entry)
        self.buttonSendCommand.place(x=175, y=320)

        self.labelMotorPIDs = ctk.CTkLabel(self, text='Motor 1 PID Values')
        self.labelMotorPIDs.place(x=250, y=85)

        self.entryP1 = ctk.CTkEntry(self, width=50)
        self.entryP1.place(x=250, y=110)

        self.entryI1 = ctk.CTkEntry(self, width=50)
        self.entryI1.place(x=310, y=110)
        
        self.entryD1 = ctk.CTkEntry(self, width=50)
        self.entryD1.place(x=370, y=110)

        self.buttonSendM1PIDs = ctk.CTkButton(self, text="Send", width=40, command=self.send_motor1_PIDs)
        self.buttonSendM1PIDs.place(x=430, y=110)

        ## PID 1 ADJUSTERS END

        ## PID 2 ADJUSTERS

        self.labelMotorPIDs = ctk.CTkLabel(self, text='Motor 2 PID Values')
        self.labelMotorPIDs.place(x=250, y=140)

        self.entryP2 = ctk.CTkEntry(self, width=50)
        self.entryP2.place(x=250, y=165)

        self.entryI2 = ctk.CTkEntry(self, width=50)
        self.entryI2.place(x=310, y=165)
        
        self.entryD2 = ctk.CTkEntry(self, width=50)
        self.entryD2.place(x=370, y=165)

        self.buttonSendM2PIDs = ctk.CTkButton(self, text="Send", width=40, command=self.send_motor2_PIDs)
        self.buttonSendM2PIDs.place(x=430, y=165)

        ## PID 2 ADJUSTERS END

        ## PID 3 ADJUSTERS

        self.labelMotorPIDs = ctk.CTkLabel(self, text='Navigational PID Values')
        self.labelMotorPIDs.place(x=250, y=195)

        self.entryP3 = ctk.CTkEntry(self, width=50)
        self.entryP3.place(x=250, y=220)

        self.entryI3 = ctk.CTkEntry(self, width=50)
        self.entryI3.place(x=310, y=220)
        
        self.entryD3 = ctk.CTkEntry(self, width=50)
        self.entryD3.place(x=370, y=220)

        self.buttonSendNAVIPIDs = ctk.CTkButton(self, text="Send", width=40, command=self.send_navigational_PIDs)
        self.buttonSendNAVIPIDs.place(x=430, y=220)

        ## PID 3 ADJUSTERS END

        ## Desired Speed Adjusters

        self.labelMotorSpeeds = ctk.CTkLabel(self, text='Motor Speed Set')
        self.labelMotorSpeeds.place(x=250, y=250)

        self.entryMotor1Speed = ctk.CTkEntry(self, width=60)
        self.entryMotor1Speed.place(x=250, y=275)

        self.entryMotor2Speed = ctk.CTkEntry(self, width=60)
        self.entryMotor2Speed.place(x=320, y=275)

        self.buttonSendSpeeds = ctk.CTkButton(self, text="Send", width=50, command=self.send_motor_speeds)
        self.buttonSendSpeeds.place(x=390, y=275)

        ## Desired Speed Adjusters End
    
        self.Label_2 = ctk.CTkLabel(self,text="PID Settings")
        self.Label_2.place(x=250.0, y=59)

        self.button_kill_speeds = ctk.CTkButton(self, text="Stop", width=50, command=lambda: self.set_speeds_and_send("0.0","0.0"))
        self.button_kill_speeds.place(x=250, y=320)

        self.buttonForwards2 = ctk.CTkButton(self, text="2.0 FW", width=50, command=lambda: self.set_speeds_and_send("2.0", "-2.0"))
        self.buttonForwards2.place(x=310, y=320)

        self.buttonForwards4 = ctk.CTkButton(self, text="4.0 FW", width=50, command=lambda: self.set_speeds_and_send("4.0", "-4.0"))
        self.buttonForwards4.place(x=370, y=320)

        self.buttonForwards4 = ctk.CTkButton(self, text="6.0 FW", width=50, command=lambda: self.set_speeds_and_send("6.0", "-6.0"))
        self.buttonForwards4.place(x=250, y=355)
    
        self.buttonToggleLED = ctk.CTkButton(self,text="Toggle LED",width=100, command=lambda: self.send_command_with_text(99, "Toggling LED"))
        self.buttonToggleLED.place(x=20, y=355)

        self.buttonToggleLED = ctk.CTkButton(self,text="Toggle Stop",width=110, command=lambda: self.send_command_with_text(88, "Toggling Stop Condition"))
        self.buttonToggleLED.place(x=130, y=355)

        self.buttonToggleLED = ctk.CTkButton(self,text="Toggle Print Encoders / Line Sensors", width=100, command=lambda: self.send_command_with_text(22, f"Toggling Encoder / Line Sensor Serial Print"))
        self.buttonToggleLED.place(x=20, y=390)

        self.buttonToggleLED = ctk.CTkButton(self,text="Toggle Serial Print", width=60, command=lambda: self.send_command_with_text(23, f"Toggling Serial Print"))
        self.buttonToggleLED.place(x=20, y=425)
    
        self.buttonRotate = ctk.CTkButton(self, text="Rotate 180 Degrees", command=lambda: self.send_command_with_text(77, "Rotating 180 degrees"))
        self.buttonRotate.place(x=310, y=355)

        self.buttonEnable = ctk.CTkButton(self, text="Enable", width=70, command=lambda: self.send_command_with_text(95, "Toggle Motors On/Off"))
        self.buttonEnable.place(x=250, y=390)

        self.buttonUseSensors = ctk.CTkButton(self, text="Use Sensors", width=70, command=lambda: self.send_command_with_text(35, "Toggle Sensor Usage"))
        self.buttonUseSensors.place(x=328, y=390)

        self.buttonConnect = ctk.CTkButton(self,text="Connect", command=lambda: run_coroutine_threadsafe(self.ble_manager.connect()), width=80)
        self.buttonConnect.place(x=338.0, y=16)

        self.buttonDisconnect = ctk.CTkButton(self,text="Disconnect", command=lambda: run_coroutine_threadsafe(self.ble_manager.disconnect()), width= 80)
        self.buttonDisconnect.place(x=250.0, y=16)


threading.Thread(target=start_asyncio_loop, daemon=True).start()

root = Root()
root.mainloop()

