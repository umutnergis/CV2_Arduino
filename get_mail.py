import imaplib
import email
from email.header import decode_header
import serial
import time

# E-posta ayarlari
EMAIL_USER = 'mertprojedestek@gmail.com'
EMAIL_PASS = 'ssgo osag tavc pjjk'
IMAP_SERVER = 'imap.gmail.com'
IMAP_PORT = 993


last_checked_id = None

def check_email():
    global last_checked_id
    try:
        # IMAP sunucusuna bağlan
        mail = imaplib.IMAP4_SSL(IMAP_SERVER, IMAP_PORT)
        mail.login(EMAIL_USER, EMAIL_PASS)
        mail.select('inbox')

        # 'KapiyiAc' konulu yeni e-posta var mi kontrol et
        status, messages = mail.search(None, '(UNSEEN SUBJECT "DOORON")')
        mail_ids = messages[0].split()

        if mail_ids:
            # Eğer daha önce kontrol edilmiş bir ID varsa, bunu kullanarak filtreleme yap
            if last_checked_id:
                new_mail_ids = [mail_id for mail_id in mail_ids if int(mail_id) > int(last_checked_id)]
            else:
                new_mail_ids = mail_ids

            if new_mail_ids:
                last_checked_id = new_mail_ids[-1]
                return True
        return False
    except Exception as e:
        print(f"E-posta kontrolü başarisiz: {e}")
        return False

if __name__ == "__main__":
    while True:
        if check_email():
            print("Kapiyi açma sinyali gönderildi.")
        else:
            print("Yeni 'KapiyiAc' e-postasi yok.")
        
        time.sleep(1)  # 60 saniye bekle ve tekrar kontrol et
