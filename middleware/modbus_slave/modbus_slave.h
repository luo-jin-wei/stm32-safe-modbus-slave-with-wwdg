#ifndef MODBUS_SLAVE_H
#define MODBUS_SLAVE_H

#include <stdint.h>
#include "frame_parser.h"

/* 保持寄存器数量 */
#define HOLDING_REG_COUNT 10

/* 从机地址 */
#define MODBUS_SLAVE_ADDR 0x01

/* 异常码 */
#define MODBUS_EX_ILLEGAL_FUNCTION  0x01
#define MODBUS_EX_ILLEGAL_DATA_ADDR 0x02
#define MODBUS_EX_ILLEGAL_DATA_VAL  0x03

/* 寄存器表（全局，SHT30 写，Modbus 读/写） */
extern uint16_t holding_regs[HOLDING_REG_COUNT];

/**
 * @brief  初始化保持寄存器表
 */
void modbus_slave_init(void);

/**
 * @brief  处理 Modbus 请求帧，构造响应帧
 * @param  parser  解析后的帧结构（由 frame_parser 提供）
 * @param  tx_buf  输出缓冲区，用于存放响应帧
 * @param  tx_len  输出响应帧长度
 * @retval 0: 正常响应已构造  1: 地址不匹配(静默丢弃)  2: CRC错误
 */
uint8_t modbus_process_request(FrameParser_t *parser, uint8_t *tx_buf, uint8_t *tx_len);

#endif
