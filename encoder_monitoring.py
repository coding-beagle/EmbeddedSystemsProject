# PLOT ENCODER DATA IN REAL TIME VIA SERIAL

import serial
import time
import keyboard as kb
import numpy as np
import matplotlib.pyplot as plt

# Function to attempt communication with the device
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
            i = max(1, min(2048, self.s.in_waiting))
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
ser = serial.Serial(port, 9600)
rl = ReadLine(ser)

xpoints = np.array([])
ypoints1 = np.array([])
ypoints2 = np.array([])

plt.ion()  # Enable interactive mode
fig, ax1 = plt.subplots()
ax2 = ax1.twinx()  # Create a second y-axis sharing the same x-axis

line1, = ax1.plot(xpoints, ypoints1, 'g-', label='Y1')  # Initial plot on the primary y-axis
line2, = ax2.plot(xpoints, ypoints2, 'b-', label='Y2')  # Initial plot on the secondary y-axis

ax1.set_xlabel('X data')
ax1.set_ylabel('Y1 data', color='g')
ax2.set_ylabel('Y2 data', color='b')

line_count = 0  # To alternate between y-axes

try:
    while True:
        response = rl.readline()
        if response:
            rps = response.decode("utf-8").strip()
            if line_count % 2 == 0:
                ypoints1 = np.append(ypoints1, float(rps))
                current_xpoints = xpoints[:len(ypoints1)]  # Adjust xpoints to match ypoints1's length
                line1.set_xdata(current_xpoints)
                line1.set_ydata(ypoints1)
            elif line_count % 2 == 1:
                ypoints2 = np.append(ypoints2, float(rps))
                current_xpoints = xpoints[:len(ypoints2)]  # Adjust xpoints to match ypoints2's length
                line2.set_xdata(current_xpoints)
                line2.set_ydata(ypoints2)
            else:
                print(response)
            
            xpoints = np.append(xpoints, len(xpoints) * 0.05)  # Append to xpoints after updating plot
            
            ax1.relim()
            ax1.autoscale_view()
            ax2.relim()
            ax2.autoscale_view()

            fig.canvas.draw()
            fig.canvas.flush_events()
            line_count += 1

        if(kb.is_pressed("a")):
            xpoints = np.array([])
            ypoints1 = np.array([])
            ypoints2 = np.array([]) 

            ax1.relim()
            ax1.autoscale_view()
            fig.canvas.draw()
            fig.canvas.flush_events()
except KeyboardInterrupt:
    print("Program terminated by user")
finally:
    plt.ioff()  # Turn off interactive mode
    plt.show()