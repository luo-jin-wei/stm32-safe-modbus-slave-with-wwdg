#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Modbus RTU 从机压力测试脚本（多帧版）
功能：发送多组不同的 Modbus 请求帧，统计回复情况
依赖：pyserial
安装：pip install pyserial
"""

import serial
import serial.tools.list_ports
import time
import struct
import random

# ============ 配置参数（用户可修改） ============
PORT = 'COM5'           # 串口号
BAUDRATE = 115200
TIMEOUT = 0.05           # 接收超时（秒）
FRAME_INTERVAL = 0.05   # 帧间隔（秒）
TOTAL_FRAMES = 500      # 总发送帧数

# ============ 定义测试帧列表 ============
# 每一帧：描述, 十六进制字节串
# 注意：帧必须包含正确的 CRC
TEST_FRAMES = [
    ("读保持寄存器 0x0000", "01 03 00 00 00 01 84 0A"),
    ("读保持寄存器 0x0001", "01 03 00 01 00 01 D5 CA"),
    ("写单个寄存器 0x0000 值 0x0001", "01 06 00 00 00 01 48 0A"),
    ("写单个寄存器 0x0001 值 0x0002", "01 06 00 01 00 02 19 CA"),
    ("非法功能码 0x05", "01 05 00 00 00 01 8C 0A"),
    ("非法地址 0xFF", "01 03 00 FF 00 01 84 3A"),
    ("超长数据长度 0x80", "01 03 80 00 00 01 84 0A"),  # 异常帧
]

# ============ 发送模式 ============
# 'sequence' : 按顺序循环
# 'random'   : 随机选取
# 'weighted' : 加权随机（可自定义）
SEND_MODE = 'sequence'

# ============ 统计变量 ============
sent_count = 0
recv_count = 0
crc_error_count = 0
timeout_count = 0
frame_stats = {}  # 按帧统计

# ============ 辅助函数 ============
def hex_str_to_bytes(hex_str):
    """将十六进制字符串（如 '01 03 00 00 00 01 84 0A'）转换为字节数组"""
    hex_str = hex_str.replace(' ', '').replace('\n', '').replace('\r', '')
    return bytes.fromhex(hex_str)

def crc16_modbus(data):
    """计算 Modbus CRC16"""
    crc = 0xFFFF
    for byte in data:
        crc ^= byte
        for _ in range(8):
            if crc & 0x0001:
                crc = (crc >> 1) ^ 0xA001
            else:
                crc >>= 1
    return crc

def is_frame_valid(frame):
    """检查帧的 CRC 是否正确"""
    if len(frame) < 3:
        return False
    data = frame[:-2]
    recv_crc = struct.unpack('<H', frame[-2:])[0]
    calc_crc = crc16_modbus(data)
    return recv_crc == calc_crc

# ============ 帧选择器 ============
def select_frame(index):
    """根据发送模式选择下一帧"""
    if SEND_MODE == 'sequence':
        return TEST_FRAMES[index % len(TEST_FRAMES)]
    elif SEND_MODE == 'random':
        return random.choice(TEST_FRAMES)
    else:
        return TEST_FRAMES[index % len(TEST_FRAMES)]

# ============ 主逻辑 ============
def main():
    global sent_count, recv_count, crc_error_count, timeout_count

    # 1. 列出可用串口
    ports = serial.tools.list_ports.comports()
    print("可用串口列表:")
    for p in ports:
        print(f"  {p.device} - {p.description}")
    print()

    # 2. 打开串口
    try:
        ser = serial.Serial(PORT, BAUDRATE, timeout=TIMEOUT)
        print(f"已打开串口 {PORT}，波特率 {BAUDRATE}")
    except Exception as e:
        print(f"打开串口失败: {e}")
        return

    # 3. 清空缓冲区
    ser.reset_input_buffer()
    ser.reset_output_buffer()

    print(f"发送模式: {SEND_MODE}")
    print(f"测试帧数: {len(TEST_FRAMES)} 种")
    print(f"总发送帧数: {TOTAL_FRAMES}，间隔 {FRAME_INTERVAL*1000:.0f}ms")
    print("请确保板子已烧录并运行...")
    time.sleep(1)

    # 4. 初始化统计
    for desc, hex_str in TEST_FRAMES:
        frame_stats[desc] = {'sent': 0, 'recv': 0, 'crc_err': 0, 'timeout': 0}

    start_time = time.time()

    for i in range(TOTAL_FRAMES):
        # 选择帧
        desc, hex_str = select_frame(i)
        frame_bytes = hex_str_to_bytes(hex_str)

        # 发送
        ser.write(frame_bytes)
        sent_count += 1
        frame_stats[desc]['sent'] += 1

        # 等待回复
        reply = ser.read(256)
        if len(reply) == 0:
            timeout_count += 1
            frame_stats[desc]['timeout'] += 1
        else:
            recv_count += 1
            frame_stats[desc]['recv'] += 1
            if not is_frame_valid(reply):
                crc_error_count += 1
                frame_stats[desc]['crc_err'] += 1

        # 间隔
        time.sleep(FRAME_INTERVAL)

        # 进度显示（每50帧打印一次）
        if (i+1) % 50 == 0:
            print(f"已发送 {i+1} 帧，收到 {recv_count} 帧，超时 {timeout_count} 帧")

    elapsed = time.time() - start_time

    # 5. 打印总报告
    print("\n========== 总体压测结果 ==========")
    print(f"总发送帧数:    {sent_count}")
    print(f"收到回复帧数:  {recv_count}")
    print(f"超时帧数:      {timeout_count}")
    print(f"CRC 错误帧数:  {crc_error_count}")
    print(f"成功率:        {recv_count/sent_count*100:.2f}%")
    print(f"总耗时:        {elapsed:.2f} 秒")
    print(f"平均帧间隔:    {elapsed/sent_count*1000:.2f} ms")
    print("==================================")

    # 6. 打印每帧详细统计
    print("\n========== 每帧详细统计 ==========")
    print(f"{'帧描述':<20} {'发送':>6} {'收到':>6} {'CRC错':>6} {'超时':>6}")
    for desc, stats in frame_stats.items():
        print(f"{desc:<20} {stats['sent']:>6} {stats['recv']:>6} {stats['crc_err']:>6} {stats['timeout']:>6}")

    ser.close()

if __name__ == '__main__':
    main()
