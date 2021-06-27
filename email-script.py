
import smtplib
import base64
import sys
import ssl

print("Mesajul pe care vrei sa il trimiti este: ", sys.argv)
arg1 = str(' '.join(sys.argv[1:]))
print('Mesjaul este: ', arg1)

port = 465
smtp_server = "smtp.gmail.com"
sender_email = "pcd999pcd@gmail.com"
receiver_email = "dan.tomoiu99@e-uvt.ro"
password = 'Ferari99'
message = arg1

context = ssl.create_default_context()

with smtplib.SMTP_SSL(smtp_server,port, context=context) as server:
    server.login(sender_email, password)
    server.sendmail(sender_email,receiver_email,message)


#python -m smtpd -c DebuggingServer -n localhost:1025