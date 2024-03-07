import serial
import time
import keyboard as kb
import numpy as np
import matplotlib.pyplot as plt
from icecream import ic

class ReadLine:
    def __init__(self, s):
        self.buf = bytearray()
        self.s = s

    def readline(self):
        i = self.buf.find(b"\n")
        if i >= 0:
            r = self.buf[:i+1]
            self.buf = self.buf[i+1:]
            return r
        while True:
            try:
                i = max(1, min(2048, self.s.in_waiting))
            except:
                return
            data = self.s.read(i)
            i = data.find(b"\n")
            if i >= 0:
                r = self.buf + data[:i+1]
                self.buf[0:] = data[i+1:]
                return r
            else:
                self.buf.extend(data)

# Main script
port = 'COM9'  # Serial port to use
try:
    ser = serial.Serial(port, 9600)
except:
    ser = None
rl = ReadLine(ser)

xpoints = np.array([])
ypoints1 = np.array([])
ypoints2 = np.array([])

plt.ion()  # Enable interactive mode
fig, axs = plt.subplots(2, 2)

line1, = axs[0][0].plot(xpoints, ypoints1, 'g-', label='Sensor 1 Values')  # Initial plot on the primary y-axis
line2, = axs[0][1].plot(xpoints, ypoints2, 'b-', label='Sensor 2 Values')  # Initial plot on the secondary y-axis
line3, = axs[1][0].plot(xpoints, ypoints1, 'g-', label='Sensor 3 Values')  
line4, = axs[1][1].plot(xpoints, ypoints2, 'b-', label='Sensor 4 Values')  

axs[0][0].set_xlabel('Time')
axs[0][1].set_xlabel('Time')
axs[1][0].set_xlabel('Time')
axs[1][1].set_xlabel('Time')

axs[0][0].set_ylabel('Sensor 1 Values', color='g')
axs[0][1].set_ylabel('Sensor 2 Values', color='b')
axs[1][0].set_ylabel('Sensor 3 Values', color='g')
axs[1][1].set_ylabel('Sensor 4 Values', color='b')

line_count = 0  # To alternate between y-axes
last_update_time = time.time()

try:
    while True:
        response = rl.readline()
        if response:
            sensor_value = response.decode("utf-8").strip()
            if line_count % 4 == 0:
                try:
                    ypoints1 = np.append(ypoints1, float(sensor_value))
                except:
                    pass
                current_xpoints = xpoints[:len(ypoints1)]  # Adjust xpoints to match ypoints1's length
            elif line_count % 4 == 1:
                try:
                    ypoints2 = np.append(ypoints2, float(sensor_value))
                except:
                    pass
                current_xpoints = xpoints[:len(ypoints2)]  # Adjust xpoints to match ypoints2's length
            elif line_count % 4 == 2:
                try:
                    ypoints3 = np.append(ypoints3, float(sensor_value))
                except:
                    pass
                current_xpoints = xpoints[:len(ypoints3)]  # Adjust xpoints to match ypoints2's length
            elif line_count % 4 == 3:
                try:
                    ypoints4 = np.append(ypoints4, float(sensor_value))
                except:
                    pass
                current_xpoints = xpoints[:len(ypoints4)]  # Adjust xpoints to match ypoints2's length
            
            xpoints = np.append(xpoints, len(xpoints) * 0.05)  # Append to xpoints after updating plot
            line_count += 1

        # Update the plot only every 100 milliseconds
        if time.time() - last_update_time >= 0.1:
            line1.set_xdata(xpoints[:len(ypoints1)])
            line1.set_ydata(ypoints1)
            line2.set_xdata(xpoints[:len(ypoints2)])
            line2.set_ydata(ypoints2)
            line3.set_xdata(xpoints[:len(ypoints3)])
            line3.set_ydata(ypoints3)
            line4.set_xdata(xpoints[:len(ypoints4)])
            line4.set_ydata(ypoints4)

            axs.relim()
            axs.autoscale_view()

            fig.canvas.draw()
            fig.canvas.flush_events()
            last_update_time = time.time()

        if(kb.is_pressed("a")):
            xpoints = np.array([])
            ypoints1 = np.array([])
            ypoints2 = np.array([])

            axs.relim()
            axs.autoscale_view()

            fig.canvas.draw()
            fig.canvas.flush_events()
except KeyboardInterrupt:
    print("Program terminated by user")
finally:
    plt.ioff()  # Turn off interactive mode
    plt.show()