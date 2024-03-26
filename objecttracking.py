import numpy as np 
import cv2
from imutils import contours
from skimage import measure
import imutils
import csv

vid = cv2.VideoCapture(1 ,cv2.CAP_DSHOW)

min_area = 300
firstFrame = None

output_data = []
frame_count = 1

while(1):
    _, frame = vid.read()
	
    # frame = frame if args.get("video", None) is None else frame[1]
	# if the frame could not be grabbed, then we have reached the end
	# of the video
    if frame is None:
        break
	# resize the frame, convert it to grayscale, and blur it
    frame = imutils.resize(frame, width=500)
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    gray = cv2.GaussianBlur(gray, (21, 21), 0)
	# if the first frame is None, initialize it
    if firstFrame is None:
        firstFrame = gray
        continue
    # compute the absolute difference between the current frame and
	# first frame
    frameDelta = cv2.absdiff(firstFrame, gray)
    thresh = cv2.threshold(frameDelta, 50, 255, cv2.THRESH_BINARY)[1]
	# dilate the thresholded image to fill in holes, then find contours
	# on thresholded image
    # thresh = cv2.erode(thresh, None, iterations=2)
    thresh = cv2.dilate(thresh, None, iterations=3)
    cnts = cv2.findContours(thresh.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    cnts = imutils.grab_contours(cnts)
	# loop over the contours
    for c in cnts:
        # if the contour is too small, ignore it
        if cv2.contourArea(c) < min_area:
            continue
        # compute the bounding box for the contour, draw it on the frame,
        # and update the text
        (x, y, w, h) = cv2.boundingRect(c)
        output_data.append([frame_count, x+w/2, y+h/2])     # add data into an array
        cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)
    
    cv2.imshow("Frame", frame)

    frame_count += 1

    if(cv2.waitKey(1) & 0xFF == ord('x')):
        break

vid.release()
cv2.destroyAllWindows()

file_name = input("File Name: ")

if file_name != "":

    with open(f"results/{file_name}.csv", mode='w', newline='') as file:
        writer = csv.writer(file)
        writer.writerows(output_data)

    print(f"Data Printed Succesfully in results/{file_name}.csv")