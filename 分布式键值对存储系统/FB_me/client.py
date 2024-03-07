import rpyc
import logging
import uuid

class KeyValueClient:
    def __init__(self, host, port, log_file, username):
        self.client_id = str(uuid.uuid4())
        self.username=username
        self.conn = rpyc.connect(host, port, config={"allow_public_attrs": True})
        self.service = self.conn.root
        # 使用RPC方法设置client_id
        logging.basicConfig(filename=log_file, level=logging.INFO, 
                            format='%(asctime)s %(levelname)s:%(message)s')
        self.login()
        self.run_command_loop()

    def put(self, key, value):
        result = self.service.put(key, value)
        logging.info(f"Client {self.username}: Put key: {key}, value: {value}, result: {result}")
        return result

    def get(self, key):
        value = self.service.get(key)
        logging.info(f"Client {self.username}: Get key: {key}, returned value: {value}")
        return value

    def delete(self, key):
        result = self.service.delete(key)
        logging.info(f"Client {self.username}: Delete key: {key}, result: {result}")
        return result

    def get_all(self):
        all_data = self.service.getall()
        logging.info(f"Client {self.username}: Get all data")
        return all_data

    def delete_all(self):
        result = self.service.delall()
        logging.info(f"Client {self.username}: Delete all data, result: {result}")
        return result
    
    def find_user_logs(self):
        with open('client_log.log', 'r') as file:
            for line in file:
                if f"Client {self.username}:" in line:
                    print(line.strip())

    def login(self):
        self.service.login(self.username)

    def logout(self):
        self.service.logout(self.username)

    def run_command_loop(self):
        print(f"Client ID: {self.username}")
        while True:
            try:
                command = input("Enter command (put/get/delete/getall/delall/getlog): ").split()
                if command[0] == 'put' and len(command) == 3:
                    self.put(command[1], command[2])
                elif command[0] == 'get' and len(command) == 2:
                    print(self.get(command[1]))
                elif command[0] == 'delete' and len(command) == 2:
                    self.delete(command[1])
                elif command[0] == 'getall':
                    print(self.get_all())
                elif command[0] == 'delall':
                    self.delete_all()
                elif command[0] == 'getlog':
                    self.find_user_logs()
                else:
                    print("Invalid command or arguments")
            except KeyboardInterrupt:
                self.logout()
                print("Exiting client")
                break
            except Exception as e:
                print(f"An error occurred: {e}")

if __name__ == "__main__":
    users = {} 
    with open('user.txt') as file:
        for i in file.readlines():
            i = i.split()
            users[i[0]] = i[1]

    #用户登录
    username = input('Please input your username:')
    password = input('Please input your password:')
    if username in users.keys() and users[username] == password:
        client = KeyValueClient("localhost", 18812, "client_log.log", username)
    else:
        print('Your username or password has something wrong.Please try again!')

        
