import rpyc
from sqlitedict import SqliteDict
from rpyc.utils.server import ThreadedServer
import datetime

class KeyValueService(rpyc.Service):
    def __init__(self):
        # 初始化数据库
        self.db = SqliteDict('./my_db.sqlite', autocommit=True)
    
    clients = {}

    def exposed_login(self, username):
        current_time = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S,%f')[:-3]
        print(f"{current_time} client: {username} logged in")

    def exposed_logout(self, username):
        current_time = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S,%f')[:-3]
        print(f"{current_time} client: {username} logged out")

    def on_connect(self, conn):
        pass

    def on_disconnect(self, conn):
        pass

    def exposed_set_client_id(self, client_id):
        # 设置客户端ID
        self.clients[self.conn] = client_id
        print(f"User: {client_id} is connected")

    def exposed_put(self, key, value):
        try:
            self.db[key] = value
            return True
        except Exception as e:
            # 在异常情况下返回错误信息
            return str(e)

    def exposed_get(self, key):
        try:
            return self.db.get(key, None)
        except Exception as e:
            return str(e)

    def exposed_delete(self, key):
        try:
            if key in self.db:
                del self.db[key]
                return True
            return False
        except Exception as e:
            return str(e)

    def exposed_getall(self):
        try:
            return dict(self.db)
        except Exception as e:
            return str(e)

    def exposed_delall(self):
        try:
            self.db.clear()
            return True
        except Exception as e:
            return str(e)

if __name__ == "__main__":
    # 启动服务器
    server = ThreadedServer(KeyValueService, port=18812)
    print("Server is working now!")
    server.start()

