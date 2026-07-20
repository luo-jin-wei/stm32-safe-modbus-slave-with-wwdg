#ifndef FRAME_PARSER_H
#define FRAME_PARSER_H

#include <stdint.h>
#include <stdbool.h>

/* 软件计数器相关配置 */
#define MAX_FRAME_DATA_LEN  128     // 单帧最大允许数据长度（字节）
#define MAX_FRAME_TOTAL_LEN 260     // 整帧最大总长度（addr+func+len+data+crc，留一点余量）
extern volatile uint32_t g_frame_err_overlen_cnt;



// 定义状态机的所有状态
typedef enum {
    STATE_IDLE,    // 空闲，等待帧头
    STATE_ADDR,    // 已收到地址
    STATE_FUNC,    // 已收到功能码
    STATE_LEN,     // 已收到长度
    STATE_DATA,    // 正在接收数据区
    STATE_CRC      // 正在接收CRC校验
} ParserState_t;

// 帧解析器的上下文结构体（存放一帧的临时数据）
typedef struct {
    uint8_t addr;          // 地址
    uint8_t func;          // 功能码
    uint8_t len;           // 数据长度
    uint8_t data[256];     // 数据区（最大256字节）
    uint16_t data_index;   // 当前已接收的数据区字节数
    uint8_t crc_high;      // CRC高字节
    uint8_t crc_low;       // CRC低字节
    uint8_t crc_count;     // 已收的CRC字节数（0或1）
		uint16_t rx_count; 
    ParserState_t state;   // 当前状态
} FrameParser_t;

// 初始化解析器（清空所有字段）
void parser_init(FrameParser_t *parser);

// 喂一个字节给状态机
// 返回 true：表示已经拆出一帧完整数据
// 返回 false：表示还在等待更多字节
bool parser_feed(FrameParser_t *parser, uint8_t byte);

#endif


