import matplotlib.pyplot as plt
import csv
import numpy as np
from matplotlib.backend_bases import MouseButton

def on_move(event):
    # if event.inaxes:
    #     print(f'data coords {event.xdata} {event.ydata},',
    #           f'pixel coords {event.x} {event.y}')
    return

def save():
    data.pop('Displacement')
    print(data)
    with open(f"file_edited.csv", mode='w', newline='') as file_2:
        fieldnames = ['Frame', 'X Pos', 'Y Pos']
        writer = csv.DictWriter(file_2, fieldnames=fieldnames)
        writer.writerows(data)

    print(f"Data Printed Succesfully at file_edited.csv")

def on_click(event):
    if event.button is MouseButton.LEFT:
        print(f'Clicked at {event.xdata}, {event.ydata}')
        for index_x, x_val in enumerate(data["X_mid"]):
            for index_y, y_val in enumerate(data["Y_mid"]):
                if(abs(event.xdata - x_val) < 2.0 and abs(event.ydata - y_val) < 2.0):
                    x_data = data["X_mid"].pop(index_x)
                    y_data = data["Y_mid"].pop(index_y)
                    print(f"Successfully removed {x_data}, {y_data}")
                    break
        plt.disconnect(binding_id)
    if event.button is MouseButton.RIGHT:
        save()

file_name = input("File to plot: ")
view_raw = True if input("View Raw (y/n)").lower() == "y" else False
editable = True if input("Edit Plot? (y/n) ").lower() == "y" else False

data = {"Frame": [],
        "X_mid": [],
        "Y_mid": [],
        "Displacement": []}


def find_index_of_item_in_array(item, array):
    for index, i in enumerate(array):
        if(i == item): return index

def remove_large_velocity_spikes(frame_array, velocity_array):
    last_velocity = 0.0
    output_velocity = velocity_array.copy()
    output_frame = frame_array.copy()
    indexes = []
    for index, i in enumerate(velocity_array):
        # print(velocity_array)
        if i > 2.0:
            indexes.append(index)
        if(abs(i - last_velocity) > 0.3):
            if(not(index in indexes)): indexes.append(index)
        last_velocity = i
    
    for i in sorted(indexes, reverse=True):
        output_frame.pop(i)
        output_velocity.pop(i)
    return (output_frame, output_velocity)

PIXELS_PER_M = 2.2/640
FPS = 1/15.0

with open(f'results/{file_name}.csv') as file:
    reader = csv.reader(file, delimiter=',')
    last_frame = 0
    same_frame_x_list = []
    same_frame_y_list = []
    if(not(view_raw)):
        for row in reader:
            frame = int(row[0])  # Convert frame count to integer
            if last_frame is not None and frame != last_frame:
                if same_frame_x_list and same_frame_y_list:  # Check if lists are not empty
                    data["Frame"].append(last_frame)
                    data["X_mid"].append(sum(same_frame_x_list) / len(same_frame_x_list))
                    data["Y_mid"].append(sum(same_frame_y_list) / len(same_frame_y_list))
                same_frame_x_list = []
                same_frame_y_list = []
            same_frame_x_list.append(float(row[1]))
            same_frame_y_list.append(float(row[2]))
            last_frame = frame
    else:
        for row in reader:
            data["Frame"].append(int(row[0]))
            data["X_mid"].append(float(row[1]))
            data["Y_mid"].append(float(row[2]))
    
print(data["X_mid"])
print(data["Y_mid"])

if(not(view_raw)):
    figure, axis = plt.subplots(2)

    axis[0].plot(np.array(data["X_mid"]), np.array(data["Y_mid"]), marker='x')
    frames_converted_to_s = [i*FPS for i in data["Frame"]]
    if(not(view_raw)):
        for index,i in enumerate(frames_converted_to_s):
            if(index == 0):
                continue
            data["Displacement"].append(abs((data["X_mid"][index] - data["X_mid"][index - 1] + data["Y_mid"][index] - data["Y_mid"][index - 1])/(i - frames_converted_to_s[index-1]))*PIXELS_PER_M)    
    
    x_dat, y_dat = remove_large_velocity_spikes(frames_converted_to_s[0:-1], data["Displacement"])
    axis[1].plot(x_dat, y_dat)
    z = np.polyfit(x_dat, y_dat, 15)
    p = np.poly1d(z)        
    axis[1].plot(x_dat, p(x_dat))        
    plt.show()
        
else:
    if(editable):
        binding_id = plt.connect('motion_notify_event', on_move)
        plt.connect('button_press_event', on_click)
        plt.scatter(np.array(data["X_mid"]), np.array(data["Y_mid"]), marker='x')
       
    else:
        plt.scatter(np.array(data["X_mid"]), np.array(data["Y_mid"]), marker='x')
    plt.show()

# plt.waitforbuttonpress()
    
