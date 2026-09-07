import os
import sys

# 设置环境变量，避免编码问题
os.environ["PYTHONIOENCODING"] = "utf-8"
os.environ["ALLURE_NO_ANALYTICS"] = "1"
os.environ["PYTHONPATH"] = "./utils"
os.environ["PYTHONPATH"] = "./resource"
os.environ["PYTHONPATH"] = "./"

def main():
    # 获取命令行参数（跳过脚本自身）
    test_path = sys.argv[1] if len(sys.argv) > 1 else "."

    # 构建 pytest 命令
    pytest_cmd = f"pytest {test_path} --alluredir=./reports"
    print(f"Running: {pytest_cmd}")
    os.system(pytest_cmd)

    # 启动 Allure 报告服务
    os.system("allure serve ./reports --port 9998")

if __name__ == "__main__":
    main()
