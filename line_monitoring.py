import serial
import time
import keyboard as kb
import numpy as np
import matplotlib.pyplot as plt
from icecream import ic
from collections import deque

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
port = 'COM21'  # Serial port to use
ser = serial.Serial(port, 9600)
rl = ReadLine(ser)

max_length = 200  # Maximum number of points to plot
xpoints = deque(maxlen=max_length)
ypoints = [deque(maxlen=max_length) for _ in range(4)]  # Use deque for efficient appends

plt.ion()  # Enable interactive mode
fig, axs = plt.subplots(2, 2)  # Create 4 subplots in a 2x2 grid
axs = axs.flatten()  # Flatten the 2x2 grid into a 1D array for easy iteration

lines = [axs[i].plot(xpoints, ypoints[i], label=f'Sensor {i+1}')[0] for i in range(4)]  # Initialize 4 lines for plotting

for i, ax in enumerate(axs):
    ax.set_xlabel('Time')
    ax.set_ylabel(f'Sensor {i+1} Readings')

time_step = 0.05  # Time step between data points
line_count = 0
last_update_time = time.time()

try:
    while True:
        response = rl.readline()
        try:
            if response:
                decoded_response = response.decode("utf-8").strip()
                label, value_str = decoded_response.split('=')
                index = int(label.strip()[-1]) - 1  # Assuming labels are 'S1', 'S2', ..., convert to 0-based index
                value = float(value_str)

                ypoints[index].append(value)
                if index == 0:  # Update xpoints only for the first dataset
                    if len(xpoints) < max_length:
                        xpoints.append(len(xpoints) * time_step)
                    else:
                        xpoints.append((len(xpoints) - 1 + time_step) * time_step)
        except:
            pass

        # Update the plots only every 100 milliseconds
        if time.time() - last_update_time >= 0.1:
            for i in range(4):
                lines[i].set_xdata(np.arange(len(ypoints[i])) * time_step)
                lines[i].set_ydata(list(ypoints[i]))

                axs[i].relim()
                axs[i].autoscale_view()

            fig.canvas.draw()
            fig.canvas.flush_events()
            last_update_time = time.time()

        if kb.is_pressed("a"):  # Reset data
            xpoints.clear()
            for ydeque in ypoints:
                ydeque.clear()

            for i, ax in enumerate(axs):
                ax.cla()  # Clear the subplot
                # Re-setup the axes, including labels and any other configurations
                ax.set_xlabel('Time')
                ax.set_ylabel(f'Line Sensor {i+1} data')
                # Re-create the plot lines for each subplot after clearing
                lines[i] = ax.plot(xpoints, ypoints[i], label=f'Line Sensor {i+1}')[0]
                # Optionally, if you had titles or legends, set them up again here
                ax.relim()
                ax.autoscale_view()

except KeyboardInterrupt:
    print("Program terminated by user")
finally:
    plt.ioff()  # Turn off interactive mode
    plt.show()
