import cv2

for i in range(100):
    cap = cv2.VideoCapture(i)
    if cap.isOpened():
        print(f"Camera index {i} is open.") # Find camera index.
        cap.release()
