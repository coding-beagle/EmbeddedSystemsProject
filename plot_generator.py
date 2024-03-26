import matplotlib.pyplot as plt
import csv
import numpy as np

file_name = input("File to plot: ")
view_raw = True if input("View Raw (y/n)").lower() == "y" else False

data = {"Frame": [],
        "X_mid": [],
        "Y_mid": [],
        "Displacement": []}

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

        if(not(view_raw)):
            for index,i in enumerate(data["Frame"]):
                if(index == 0):
                    continue
                data["Displacement"].append(abs((data["X_mid"][index] - data["X_mid"][index - 1] + data["Y_mid"][index] - data["Y_mid"][index - 1])/(i - data["Frame"][index-1])))
        z = np.polyfit(data["Frame"][0:-1], data["Displacement"], 10)
        p = np.poly1d(z)
        axis[1].plot(data["Frame"][0:-1], data["Displacement"])        
        axis[1].plot(data["Frame"][0:-1], p(data["Frame"][0:-1]))
else:
    plt.scatter(np.array(data["X_mid"]), np.array(data["Y_mid"]), marker='x')

plt.waitforbuttonpress()
    
