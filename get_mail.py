import imaplib
import email
from email.header import decode_header
import serial
import time

# Email settings
EMAIL_USER = 'mertprojedestek@gmail.com'
EMAIL_PASS = 'ssgo osag tavc pjjk'
IMAP_SERVER = 'imap.gmail.com'
IMAP_PORT = 993


last_checked_id = None

def check_email():
    global last_checked_id
    try:
        # Connect to the IMAP server
        mail = imaplib.IMAP4_SSL(IMAP_SERVER, IMAP_PORT)
        mail.login(EMAIL_USER, EMAIL_PASS)
        mail.select('inbox')

        # Check for new emails with the subject 'DOORON'
        status, messages = mail.search(None, '(UNSEEN SUBJECT "DOORON")')
        mail_ids = messages[0].split()

        if mail_ids:
            # If a previously checked ID exists, use it to filter
            if last_checked_id:
                new_mail_ids = [mail_id for mail_id in mail_ids if int(mail_id) > int(last_checked_id)]
            else:
                new_mail_ids = mail_ids

            if new_mail_ids:
                last_checked_id = new_mail_ids[-1]
                return True
        return False
    except Exception as e:
        print(f"Email check failed: {e}")
        return False

if __name__ == "__main__":
    while True:
        if check_email():
            print("Door opening signal sent.")
        else:
            print("No new 'DOORON' email.")
        
        time.sleep(1)  # Wait 60 seconds and check again
