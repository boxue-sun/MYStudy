import paramiko
import time
import threading
from pyftpdlib.authorizers import DummyAuthorizer
from pyftpdlib.handlers import FTPHandler
from pyftpdlib.servers import FTPServer
import os
import sys
import json
import datetime
ssh = paramiko.SSHClient()
ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
authorizer = DummyAuthorizer()

def generate_password():
    current_date = datetime.datetime.now()
    return f"bJCw@@{current_date.strftime('%Y%m')}"

def update_password_periodically():
    while True:
        current_password = generate_password()
        print(f"current passwd: {current_password}")
        if authorizer.user_table["MECFTPBJCW"]["pwd"] != current_password:
            authorizer.user_table["MECFTPBJCW"]["pwd"] = current_password
        time.sleep(60*5)

def ssh_command(command):
    try:
        ssh.exec_command(command)
    except paramiko.AuthenticationException:
        print("Authentication failed, please verify your credentials.")
    except paramiko.SSHException as ssh_exception:
        print(f"SSH connection failed: {ssh_exception}")
    except Exception as e:
        print(f"An unexpected error occurred: {e}")

def download_file():
    try:
        sftp = ssh.open_sftp()
        sftp.get("/var/ptppartlog.txt", "/var/ptppartlog.txt")
        sftp.close()
    except paramiko.AuthenticationException:
        print("Authentication failed, please verify your credentials.")
    except Exception as e:
        print(f"An unexpected error occurred: {e}")

def start_ftpserver(ftpusername, ftppassword, ftpport, ftpdir):
    current_username = "MECFTPBJCW"
    current_password = generate_password()
    FTP_ROOT_DIR = ftpdir
    authorizer.add_user(current_username, current_password, FTP_ROOT_DIR, perm='elradfmw')
    handler = FTPHandler
    handler.authorizer = authorizer
    handler.passive_ports = range(10000, 10100)
    server = FTPServer(('0.0.0.0', ftpport), handler)
    print("FTP server start")
    server.serve_forever()

def journalctl_download(host, username, password, command, command_temp1, command_temp2):
    while True:
        print("reconnect")
        try:
            ssh.connect(host, username=username, password=password)
        except paramiko.AuthenticationException:
            print("authentication failed")
        except paramiko.SSHException as ssh_exception:
            print("ssh connectionfailed")
        except Exception as e:
            print("an unexpected error occurred")
        if ssh.get_transport().is_active():
            ssh_command(command_temp1)
            ssh_command(command_temp2)
            print("success")
            break
        time.sleep(1)

    while True:
        ssh_command(command)
        time.sleep(1)
        download_file()

if __name__ == "__main__":
    # 检查是否提供了必要的参数数量
    if len(sys.argv) != 2:
        # 打印用法信息
        print("用法: python PTPPartlog.py configFilePath")
        sys.exit(1)  # 以非零状态码退出，表示错误

    config_file = sys.argv[1]
    # 读取并解析JSON配置文件
    with open(config_file, 'r') as file:
        config = json.load(file)

    username = None
    password = None
    ftpusername = None
    ftppassword = None
    ftpport = None
    ftpdir = None
    host = '127.0.0.1'
    ptplogconfiger = config.get('F_ptpLogParam')
    if ptplogconfiger:
        username = ptplogconfiger.get('A_localSshUserName')
        password = ptplogconfiger.get('B_localSshPassword')
        ftpusername = ptplogconfiger.get('C_ftpUserName')
        ftppassword = ptplogconfiger.get('D_ftpPassword')
        ftpport = ptplogconfiger.get('E_ftpPort')
        ftpdir = ptplogconfiger.get('F_ftpDir')

    command = 'echo ' + password + ' | sudo -S journalctl -u phc2sys -r -n 10 > /var/ptppartlog.txt'
    command_temp1 = 'echo ' + password + ' | sudo -S touch /var/ptppartlog.txt'
    command_temp2 = 'echo ' + password + ' | sudo -S chmod a+w /var/ptppartlog.txt'
    
    # journalctl_thread = threading.Thread(target=journalctl_download, args=(host, username, password, command, command_temp1, command_temp2))
    ftpserver_thread = threading.Thread(target=start_ftpserver, args=(ftpusername, ftppassword, ftpport, ftpdir))
    interval = 1
    updaterpwd_thread = threading.Thread(target=update_password_periodically)
    # journalctl_thread.start()
    ftpserver_thread.start()
    updaterpwd_thread.start()
    # journalctl_thread.join()
    # print("journalctl_thread out")
    ftpserver_thread.join()
    print("ftpserver_thread out")
    updaterpwd_thread.join()
    print("updaterpwd_thread out")
