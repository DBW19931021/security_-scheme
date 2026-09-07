import requests
import serial
import yaml
import allure
# from utils.logger import logger

# 读取配置
with open("config/config.yaml", "r", encoding="utf-8") as f:  # 指定 UTF-8
    config = yaml.safe_load(f)

API_BASE_URL = config["base_url"]

class DeviceAPI:
    def __init__(self):
        # self.serial_port = serial.Serial(
        #     port=config["device"]["port"],
        #     baudrate=config["device"]["baudrate"],
        #     timeout=config["device"]["timeout"],
        # )
        # logger.info("初始化")
        pass

    @allure.step("发送 HTTP 请求: {endpoint}")
    def send_http_request(self, endpoint, method="GET", data=None):
        url = f"{API_BASE_URL}/{endpoint}"
        # logger.info(f"发送请求: {method} {url}，数据: {data}")
        if method == "GET":
            response = requests.get(url)
        elif method == "POST":
            response = requests.post(url, json=data)
        else:
            raise ValueError("不支持的 HTTP 方法")
        allure.attach(response.text, name="HTTP 响应", attachment_type=allure.attachment_type.TEXT)
        return response.json()

    @allure.step("发送串口指令: {command}")
    def send_serial_command(self, command):
        # logger.info(f"发送串口指令: {command}")
        # self.serial_port.write(command.encode())
        # response = self.serial_port.readline().decode().strip()
        response = b"0000"
        allure.attach(response, name="串口响应", attachment_type=allure.attachment_type.TEXT)
        return response
