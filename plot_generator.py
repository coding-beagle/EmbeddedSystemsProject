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

PIXELS_PER_M = 1/300

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
    frames_converted_to_s = [i/30.0 for i in data["Frame"]]
    if(not(view_raw)):
        for index,i in enumerate(frames_converted_to_s):
            if(index == 0):
                continue
            data["Displacement"].append(abs((data["X_mid"][index] - data["X_mid"][index - 1] + data["Y_mid"][index] - data["Y_mid"][index - 1])/(i - frames_converted_to_s[index-1]))*PIXELS_PER_M)    
    z = np.polyfit(frames_converted_to_s[0:-1], data["Displacement"], 10)
    p = np.poly1d(z)
    axis[1].plot(frames_converted_to_s[0:-1], data["Displacement"])        
    axis[1].plot(frames_converted_to_s[0:-1], p(frames_converted_to_s[0:-1]))        
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
    
