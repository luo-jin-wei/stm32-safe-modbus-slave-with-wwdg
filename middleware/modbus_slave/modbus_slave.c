#include "modbus_slave.h"
#include "crc16.h"
#include <string.h>

/* 保持寄存器表定义 */
uint16_t holding_regs[HOLDING_REG_COUNT] = {0};

void modbus_slave_init(void)
{
    memset(holding_regs, 0, sizeof(holding_regs));
}

/**
 * @brief  计算一帧的 CRC16（用于验证接收帧）
 * @note   范围：addr + func + data[]
 */
static uint16_t calc_rx_crc(FrameParser_t *parser)
{
    uint8_t buf[260];
    uint8_t len = 0;
    buf[len++] = parser->addr;
    buf[len++] = parser->func;
    for (uint8_t i = 0; i < parser->data_len; i++) {
        buf[len++] = parser->data[i];
    }
    return crc16_modbus(buf, len);
}

/**
 * @brief  把 CRC 追加到 tx_buf 末尾
 */
static void append_crc(uint8_t *tx_buf, uint8_t *tx_len)
{
    uint16_t crc = crc16_modbus(tx_buf, *tx_len);
    tx_buf[(*tx_len)++] = crc & 0xFF;        // CRC_L
    tx_buf[(*tx_len)++] = (crc >> 8) & 0xFF; // CRC_H
}

uint8_t modbus_process_request(FrameParser_t *parser, uint8_t *tx_buf, uint8_t *tx_len)
{
    *tx_len = 0;

    /* 1. 地址过滤：只响应 0x01 */
    if (parser->addr != MODBUS_SLAVE_ADDR) {
        return 1;  // 静默丢弃
    }

    /* 2. CRC 校验 */
    uint16_t rx_crc = ((uint16_t)parser->crc_high << 8) | parser->crc_low;
    if (calc_rx_crc(parser) != rx_crc) {
        return 2;  // CRC 错误，静默丢弃（Modbus 标准不要求回复 CRC 错帧）
    }

    /* 3. 功能码分发 */
    if (parser->func == 0x03) {
        /* ========== 03 读保持寄存器 ========== */
        uint16_t start_addr = ((uint16_t)parser->data[0] << 8) | parser->data[1];
        uint16_t reg_num    = ((uint16_t)parser->data[2] << 8) | parser->data[3];

        /* 异常：非法数据地址 */
        if (start_addr + reg_num > HOLDING_REG_COUNT || reg_num == 0 || reg_num > 125) {
            tx_buf[(*tx_len)++] = MODBUS_SLAVE_ADDR;
            tx_buf[(*tx_len)++] = 0x83;  // func + 0x80
            tx_buf[(*tx_len)++] = MODBUS_EX_ILLEGAL_DATA_ADDR;
            append_crc(tx_buf, tx_len);
            return 0;
        }

        /* 正常响应 */
        tx_buf[(*tx_len)++] = MODBUS_SLAVE_ADDR;
        tx_buf[(*tx_len)++] = 0x03;
        tx_buf[(*tx_len)++] = reg_num * 2;  // 字节数

        for (uint16_t i = 0; i < reg_num; i++) {
            uint16_t val = holding_regs[start_addr + i];
            tx_buf[(*tx_len)++] = (val >> 8) & 0xFF;  // 高字节
            tx_buf[(*tx_len)++] = val & 0xFF;          // 低字节
        }
        append_crc(tx_buf, tx_len);
        return 0;

    } else if (parser->func == 0x06) {
        /* ========== 06 写单个寄存器 ========== */
        uint16_t start_addr = ((uint16_t)parser->data[0] << 8) | parser->data[1];
        uint16_t reg_val    = ((uint16_t)parser->data[2] << 8) | parser->data[3];

        /* 异常：非法数据地址 */
        if (start_addr >= HOLDING_REG_COUNT) {
            tx_buf[(*tx_len)++] = MODBUS_SLAVE_ADDR;
            tx_buf[(*tx_len)++] = 0x86;  // func + 0x80
            tx_buf[(*tx_len)++] = MODBUS_EX_ILLEGAL_DATA_ADDR;
            append_crc(tx_buf, tx_len);
            return 0;
        }

        /* 异常：非法数据值（如报警阈值 > 500） */
        if (start_addr == 0x0002 && reg_val > 500) {
            tx_buf[(*tx_len)++] = MODBUS_SLAVE_ADDR;
            tx_buf[(*tx_len)++] = 0x86;
            tx_buf[(*tx_len)++] = MODBUS_EX_ILLEGAL_DATA_VAL;
            append_crc(tx_buf, tx_len);
            return 0;
        }

        /* 写寄存器 */
        holding_regs[start_addr] = reg_val;

        /* 正常响应：原样返回请求 */
        tx_buf[(*tx_len)++] = MODBUS_SLAVE_ADDR;
        tx_buf[(*tx_len)++] = 0x06;
        tx_buf[(*tx_len)++] = parser->data[0];
        tx_buf[(*tx_len)++] = parser->data[1];
        tx_buf[(*tx_len)++] = parser->data[2];
        tx_buf[(*tx_len)++] = parser->data[3];
        append_crc(tx_buf, tx_len);
        return 0;

    } else {
        /* ========== 非法功能码 ========== */
        tx_buf[(*tx_len)++] = MODBUS_SLAVE_ADDR;
        tx_buf[(*tx_len)++] = parser->func + 0x80;
        tx_buf[(*tx_len)++] = MODBUS_EX_ILLEGAL_FUNCTION;
        append_crc(tx_buf, tx_len);
        return 0;
    }
}
