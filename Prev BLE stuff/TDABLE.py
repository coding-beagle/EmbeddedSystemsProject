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
    def __init__(self, device_name, response_callback = None, updaterps1cb = None, updaterps2cb = None):
        self.device_name = device_name
        self.client = None
        self.connected = False
        self.response_callback = response_callback
        self.update_rps1_callback = updaterps1cb
        self.update_rps2_callback = updaterps2cb
        self.consoleReference = None
        self.rps1 = 0.0
        self.rps2 = 0.0

    async def notification_handler(self, sender, data):
        # convert data to a string or process it as needed
        # check if the message matches the expected RPS format
        try:
            message = data.decode('utf-8')
        except UnicodeDecodeError:
            message = f"failed to decode"
            print(f"Bad decoded data: {data}")

        match_rps1 = re.match(r"RPS1 = ([\d.]+)", message)
        if match_rps1:
            self.rps1 = float(match_rps1.group(1))
            self.update_rps1_callback(self.rps1)

        match_rps2 = re.match(r"RPS2 = ([\d.]+)", message)
        if match_rps2:
            self.rps2 = float(match_rps2.group(1))
            self.update_rps2_callback(self.rps2)
            
        if(not(match_rps1) and not(match_rps2)):
            if self.response_callback:
                self.response_callback(message)

    async def connect(self):
        if self.client is None:
            self.consoleReference.insert(tk.END, f"Finding devices\n")
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
            exit()

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
    
    async def send_command_with_argument(self, command, arg=None, delay_between=0.15):
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
        # this method updates the GUI with the response
        self.TBcommandLog.insert(tk.END, f"\nResponse: {response}")
        self.TBcommandLog.see(tk.END)

    def toggle_led(self):
        self.TBcommandLog.insert(tk.END ,"\nToggling LED")
        self.TBcommandLog.see(tk.END)
        run_coroutine_threadsafe(self.ble_manager.send_command(99))

    def toggle_RPS_count(self):
        self.TBcommandLog.insert(tk.END ,"\nToggling RPS Counter")
        self.TBcommandLog.see(tk.END)
        run_coroutine_threadsafe(self.ble_manager.send_command(50))

    def enable_disable(self):
        if(self.enabled):
            self.TBcommandLog.insert(tk.END ,"\nDisabling Motors")
            self.TBcommandLog.see(tk.END)
            run_coroutine_threadsafe(self.ble_manager.send_command(13))
            self.enabled = False
        else:
            self.enabled = False
            self.TBcommandLog.insert(tk.END ,"\nEnabling Motors")
            self.TBcommandLog.see(tk.END)
            run_coroutine_threadsafe(self.ble_manager.send_command(13))

    def update_rps1(self, val):
        self.labelWheel1SPEED.configure(text=val)
        self.sliderRPS1.set(val)

    def update_rps2(self, val):
        self.labelWheel2SPEED.configure(text=val)
        self.sliderRPS2.set(val)

    async def set_motor1_speed_async(self, val):
        self.changing_motor1_speed = True
        await self.ble_manager.send_command_with_argument(19, (val))
        self.changing_motor1_speed = False

    def set_motor1_speed(self, val):
        if(not(self.changing_motor1_speed)):
            run_coroutine_threadsafe(self.set_motor1_speed_async(int(val)))

    async def set_motor2_speed_async(self, val):
        self.changing_motor2_speed = True
        await self.ble_manager.send_command_with_argument(20, (val))
        self.changing_motor2_speed = False

    def set_motor2_speed(self, val):
        if(not(self.changing_motor2_speed)):
            run_coroutine_threadsafe(self.set_motor2_speed_async(int(val)))

    async def set_motor1_direction_async(self, val):
        await self.ble_manager.send_command_with_argument(14, (val))

    def set_motor1_direction(self, val):
        run_coroutine_threadsafe(self.set_motor1_direction_async(int(val)))

    async def set_motor2_direction_async(self, val):
        await self.ble_manager.send_command_with_argument(15, (val))

    def set_motor2_direction(self, val):
        run_coroutine_threadsafe(self.set_motor2_direction_async(int(val)))

    def send_entry(self, e=None):
        value_to_send = self.commandEntry.get()
        if("0x" in value_to_send):
            self.TBcommandLog.insert(tk.END , f"\nSending: {int(value_to_send, 16)}")    # allow us to send hex values through terminal
            run_coroutine_threadsafe(self.ble_manager.send_command(int(value_to_send.split("0x")[1], 16)))
        else:
            self.TBcommandLog.insert(tk.END , f"\nSending: {int(value_to_send)}")
            run_coroutine_threadsafe(self.ble_manager.send_command(int(value_to_send)))
        self.TBcommandLog.see(tk.END)

    def float_to_bytes_le(self, f):
        # Convert the float to 4 bytes in little-endian format
        bytes_le = struct.pack('<f', f)
        return list(struct.unpack('4B', bytes_le))

    async def send_float_async(self, f):
        await self.ble_manager.send_command_with_argument(33, f)

    def send_float(self):
        input = float(self.entryFloatInput.get())
        run_coroutine_threadsafe(self.send_float_async(self.float_to_bytes_le(input)))

    def __init__(self):
        super().__init__()
        self.enabled = True
        self.ble_manager = BLEDeviceManager("ZhipengBL")
        self.ble_manager.response_callback = self.update_response
        self.ble_manager.update_rps1_callback = self.update_rps1
        self.ble_manager.update_rps2_callback = self.update_rps2

        self.changing_motor1_speed = False
        self.changing_motor2_speed = False
        
        self.protocol("WM_DELETE_WINDOW", self.on_close)
        self.lock = asyncio.Lock()

        ## Geometry and Theme Settings
        ctk.set_appearance_mode("dark")   
        ctk.set_default_color_theme("dark-blue")
    
        self.geometry("500x500")
        self.title("WQY BLE")

        self.resizable(False, False)
    
        self.labelTitle = ctk.CTkLabel(self,text="WQY BLE APP",font=('roboto', -30))
        self.labelTitle.place(x=23, y=16)
    
        self.TBcommandLog = ctk.CTkTextbox(self,height=220)
        self.TBcommandLog.place(x=23.0, y=89)
        self.TBcommandLog.bind("<Key>", lambda e: "break")

        self.ble_manager.consoleReference = self.TBcommandLog

        self.labelConsoleLog = ctk.CTkLabel(self,text="Console Log")
        self.labelConsoleLog.place(x=25, y=59)
    
        self.commandEntry = ctk.CTkEntry(self)
        self.commandEntry.place(x=22, y=320)
        self.commandEntry.bind("<Return>", self.send_entry)
    
        self.buttonSendCommand = ctk.CTkButton(self,text="SEND",width=30, command=self.send_entry)
        self.buttonSendCommand.place(x=177, y=320)
    
        self.Label_2 = ctk.CTkLabel(self,text="Motor Control")
        self.Label_2.place(x=250.0, y=59)
    
        self.sliderMotor1Speed = ctk.CTkSlider(self,from_=0, to=100, number_of_steps=100, command=self.set_motor1_speed)
        self.sliderMotor1Speed.place(x=250, y=130)
        self.sliderMotor1Speed.set(100)
    
        self.buttonMotor1DirFW = ctk.CTkButton(self,text="Forward",width=40, command=lambda: self.set_motor1_direction(1))
        self.buttonMotor1DirFW.place(x=247, y=162)
    
        self.buttonMotor1DirBW = ctk.CTkButton(self,text="Backward",width=40, command=lambda: self.set_motor1_direction(0))
        self.buttonMotor1DirBW.place(x=319, y=162)
    
        self.buttonMotor2DirBW = ctk.CTkButton(self,text="Backward",width=40, command=lambda: self.set_motor2_direction(1))
        self.buttonMotor2DirBW.place(x=319.0, y=277)
    
        self.buttonMotor2DirFW = ctk.CTkButton(self,text="Forward",width=40, command=lambda: self.set_motor2_direction(0))
        self.buttonMotor2DirFW.place(x=247.0, y=277)
    
        self.buttonToggleLED = ctk.CTkButton(self,text="Toggle LED",width=50, command=self.toggle_led)
        self.buttonToggleLED.place(x=247, y=320)
    
        self.buttonEnableDisable = ctk.CTkButton(self,text="Toggle Motors",width=50, command=self.enable_disable)
        self.buttonEnableDisable.place(x=338, y=320)
    
        self.sliderRPS1 = ctk.CTkSlider(self, state='disabled', from_=0, to=16.0, border_width=10.0)
        self.sliderRPS1.place(x=23.0, y=399)
    
        self.sliderRPS2 = ctk.CTkSlider(self, state='disabled', from_=0, to=16.0, border_width=10.0)
        self.sliderRPS2.place(x=23.0, y=456)
    
        self.labelWheel1RPS = ctk.CTkLabel(self,text="Wheel 1 RPS")
        self.labelWheel1RPS.place(x=24, y=366)
    
        self.labelWheel2RPS = ctk.CTkLabel(self,text="Wheel 2 RPS")
        self.labelWheel2RPS.place(x=25, y=425)
    
        self.labelWheel1SPEED = ctk.CTkLabel(self)
        self.labelWheel1SPEED.place(x=164, y=366)
    
        self.labelWheel2SPEED = ctk.CTkLabel(self)
        self.labelWheel2SPEED.place(x=166, y=425)
    
        self.labelMotor2Speed = ctk.CTkLabel(self,text="Motor 2 Speed")
        self.labelMotor2Speed.place(x=250.0, y=207)
    
        self.labelMotor1Speed = ctk.CTkLabel(self,text="Motor 1 Speed")
        self.labelMotor1Speed.place(x=250.0, y=95)
    
        self.sliderMotor2Speed = ctk.CTkSlider(self, from_=0, to=100, number_of_steps=100, command=self.set_motor2_speed)
        self.sliderMotor2Speed.place(x=250.0, y=242.0)
        self.sliderMotor2Speed.set(100)

        self.labelFloatInput = ctk.CTkLabel(self, text="Send Float")
        self.labelFloatInput.place(x=250.0, y=360.0)

        self.entryFloatInput = ctk.CTkEntry(self)
        self.entryFloatInput.place(x=250.0, y=390.0)

        self.buttonSendFloat = ctk.CTkButton(self, width=40, text='Send', command=self.send_float)
        self.buttonSendFloat.place(x=400.0, y=390.0)
    
        self.buttonConnect = ctk.CTkButton(self,text="Connect", command=lambda: run_coroutine_threadsafe(self.ble_manager.connect()), width=80)
        self.buttonConnect.place(x=338.0, y=16)

        self.buttonDisconnect = ctk.CTkButton(self,text="Disconnect", command=lambda: run_coroutine_threadsafe(self.ble_manager.disconnect()), width= 80)
        self.buttonDisconnect.place(x=250.0, y=16)

        self.switchRPSCount = ctk.CTkSwitch(self, text="Toggle RPS Count?", command=self.toggle_RPS_count)
        self.switchRPSCount.place(x=250.0, y=440.0)
        self.switchRPSCount.select()

threading.Thread(target=start_asyncio_loop, daemon=True).start()

root = Root()
root.mainloop()
