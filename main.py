import cv2
from simple_facerec import SimpleFacerec
import serial
import time
from threading import Thread
import yagmail
import imaplib
from email.header import decode_header

sfr = SimpleFacerec()
sfr.load_encoding_images("images/")

# Email settings
EMAIL_USER = 'mertprojedestek@gmail.com'
EMAIL_PASS = 'ssgo osag tavc pjjk'
IMAP_SERVER = 'imap.gmail.com'
IMAP_PORT = 993

last_checked_id = None

arduino_connect = None
while not arduino_connect:
    try:
        com_port = "COM4"
        arduino_connect = serial.Serial(port=com_port, baudrate=9600, timeout=0.1)
    except:
        print("Connection failed. Please enter a valid port.")

def read_ardunio():
    while True:
        if arduino_connect.in_waiting > 0:
            line = arduino_connect.readline()
            # print(line)
            if line == b'mail\r\n':
                send_mail()
        time.sleep(1)

def send_command():
    #print("Door is opening")
    arduino_connect.write("ok".encode())
    time.sleep(1)

def send_mail():
    try:
        # Take a photo
        ret, frame = cap.read()
        if not ret:
            print("Could not capture camera image.")
            return

        # Save the photo
        photo_path = "temp_photo.jpg"
        cv2.imwrite(photo_path, frame)

        # Create Yagmail client
        yag = yagmail.SMTP(user="mertprojedestek@gmail.com", password="ssgo osag tavc pjjk")

        # Email content
        subject = "Hello Mert"
        contents = ["There is a danger situation.", "Image is attached:", yagmail.inline(photo_path)]
        
        # Send email
        yag.send(to="gmert612@gmail.com", subject=subject, contents=contents)
        print("Email sent successfully!")
    except Exception as e:
        print(f"Error occurred while sending email: {e}")

def check_email():
    global last_checked_id
    try:
        while True:
            # Connect to the IMAP server
            mail = imaplib.IMAP4_SSL(IMAP_SERVER, IMAP_PORT)
            mail.login(EMAIL_USER, EMAIL_PASS)
            mail.select('inbox')

            # Check for new emails with the subject 'DOORON'
            status, messages = mail.search(None, '(UNSEEN SUBJECT "DOORON")')
            mail_ids = messages[0].split()
            time.sleep(0.3)
            print("Checking email")
            if mail_ids:
                # If a previously checked ID exists, use it to filter
                if last_checked_id:
                    new_mail_ids = [mail_id for mail_id in mail_ids if int(mail_id) > int(last_checked_id)]
                else:
                    new_mail_ids = mail_ids

                if new_mail_ids:
                    last_checked_id = new_mail_ids[-1]
                    print("Door opening signal sent.")
                    send_command()
                    return True
                return False
        
    except Exception as e:
        print(f"Email check failed: {e}")
        return False
    

cap = cv2.VideoCapture(0)

recognized_faces = {}
error_count = 0
check_face = False
check_error = False

Thread(target=read_ardunio).start()
Thread(target=check_email).start()
while True:
    ret, frame = cap.read()

    # Identify the face
    face_locations, face_names = sfr.detect_known_faces(frame)
    current_faces = set(face_names)  # Create a set of faces in this frame

    for face_loc, name in zip(face_locations, face_names):
        y1, x2, y2, x1 = face_loc[0], face_loc[1], face_loc[2], face_loc[3]

        cv2.putText(frame, name, (x1, y1 - 10), cv2.FONT_HERSHEY_DUPLEX, 1, (0, 0, 200), 2)
        cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 0, 200), 4)
        
        if name == "Error":
            print(error_count)
            error_count += 1
            if error_count == 10:  # Perform the action only once for an unrecognized face
                print("Face could not be recognized")
                check_error = True
        elif name not in recognized_faces:
            print("Face recognized")
            check_face = True
            recognized_faces[name] = True

    if check_face:
        send_command()
        check_face = False
    
    if check_error:
        send_mail()
        check_error = False
        error_count = 0     
        
    for recognized in list(recognized_faces):
        if recognized not in current_faces:
            del recognized_faces[recognized]

    cv2.imshow("PROJECT", frame)

    key = cv2.waitKey(1)
    if key == 27:  
        break

cap.release()
cv2.destroyAllWindows()
