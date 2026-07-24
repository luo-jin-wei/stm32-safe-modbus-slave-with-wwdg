#ifndef FRAME_PARSER_H
#define FRAME_PARSER_H

#include <stdint.h>
#include <stdbool.h>

/* 软件计数器相关配置 */
#define MAX_FRAME_DATA_LEN  128     // 单帧最大允许数据长度（字节）
#define MAX_FRAME_TOTAL_LEN 260     // 整帧最大总长度（addr+func+data+crc，留一点余量）

/* 超长帧丢弃计数器（可在 main.c 中读取显示） */
extern volatile uint32_t g_frame_err_overlen_cnt;

/* 帧解析状态机 */
typedef enum {
    STATE_IDLE,    // 空闲，等待帧头地址
    STATE_ADDR,    // 已收到地址
    STATE_FUNC,    // 已收到功能码，准备收数据区
    STATE_DATA,    // 正在接收数据区
    STATE_CRC      // 正在接收 CRC
} ParserState_t;

/* 帧解析器结构体（适配 Modbus RTU 格式） */
typedef struct {
    uint8_t addr;          // 设备地址
    uint8_t func;          // 功能码
    uint8_t data[256];     // 数据区缓冲区
    uint8_t data_len;      // 数据区实际长度
    uint8_t data_expected; // 期望收到的数据区长度（由功能码决定）
    uint8_t crc_high;      // CRC 高字节
    uint8_t crc_low;       // CRC 低字节
    uint8_t crc_count;     // 已收到的 CRC 字节数
    uint16_t rx_count;     // 软件计数器：从帧头开始已接收的字节数
    ParserState_t state;   // 当前状态
} FrameParser_t;

void parser_init(FrameParser_t *parser);
bool parser_feed(FrameParser_t *parser, uint8_t byte);

#endif
