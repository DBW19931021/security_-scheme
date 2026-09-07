import logging
import os

def setup_logger(name: str, log_file: str, level=logging.DEBUG) -> logging.Logger:
    """创建并返回一个指定名称和日志文件的 logger"""
    formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s')
    handler = logging.FileHandler(log_file, mode='a', encoding='utf-8')  
    handler.setFormatter(formatter)

    logger = logging.getLogger(name)
    logger.setLevel(level)
    logger.addHandler(handler)
    logger.propagate = False  # 避免日志重复输出

    return logger

