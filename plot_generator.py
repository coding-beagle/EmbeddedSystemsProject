import matplotlib.pyplot as plt
import csv
import numpy as np

file_name = input("File to plot: ")

data = {"Frame": [],
        "X_mid": [],
        "Y_mid": []}

with open(f'results/{file_name}.csv') as file:
    reader = csv.reader(file, delimiter=',')
    for row in reader:
        data["Frame"].append(int(row[0]))
        data["X_mid"].append(float(row[1]))
        data["Y_mid"].append(float(row[2]))

print(data["X_mid"])
print(data["Y_mid"])
plt.scatter(np.array(data["X_mid"]), np.array(data["Y_mid"]), marker='x')
plt.waitforbuttonpress()
    