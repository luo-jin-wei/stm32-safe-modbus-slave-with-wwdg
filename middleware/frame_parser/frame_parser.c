#include "frame_parser.h"
#include <string.h>  // 用到 memset

// 初始化：把状态机复位到 IDLE 状态
void parser_init(FrameParser_t *parser) {
    // 把整个结构体清零
    memset(parser, 0, sizeof(FrameParser_t));
    parser->state = STATE_IDLE;
}

// 核心函数：喂字节
bool parser_feed(FrameParser_t *parser, uint8_t byte) {
    // 使用 switch 根据当前状态决定如何处理这个字节
    switch (parser->state) {

        // ========== 1. 空闲状态 ==========
        case STATE_IDLE:
            // 假设我们的协议固定从机地址是 0x01（Modbus 标准）
            // 如果不是 0x01，直接丢弃（忽略噪声）
            if (byte == 0x01) {
                parser->addr = byte;        // 存地址
                parser->state = STATE_ADDR; // 切到下一个状态
            }
            // 注意：这里如果 byte != 0x01，什么都不做，保持 IDLE
            break;

        // ========== 2. 接收功能码 ==========
        case STATE_ADDR:
            parser->func = byte;
            parser->state = STATE_FUNC;
            break;

        // ========== 3. 接收长度 ==========
        case STATE_FUNC:
            parser->len = byte;
            parser->data_index = 0;          // 重置数据区指针
            parser->state = STATE_LEN;
            break;

        // ========== 4. 接收数据区 ==========
        case STATE_LEN:
            // 把当前字节存入 data 数组
            parser->data[parser->data_index++] = byte;

            // 判断是否收够了 len 个字节
            if (parser->data_index >= parser->len) {
                parser->state = STATE_DATA;  // 数据收完了，切到 CRC 状态
                parser->crc_count = 0;       // 重置 CRC 计数器
            }
            break;

        // ========== 5. 接收 CRC 校验（2个字节） ==========
        case STATE_DATA:
            if (parser->crc_count == 0) {
                parser->crc_high = byte;     // 存 CRC 高字节
                parser->crc_count = 1;
            } else {
                parser->crc_low = byte;      // 存 CRC 低字节
                // ★★★ 关键：到这里，一帧完整数据已经收完了 ★★★
                // 立刻复位状态机，准备接收下一帧
                parser->state = STATE_IDLE;
                return true;  // 返回 true，告诉主循环“拆出一帧了！”
            }
            break;

        // 默认情况（实际上不会发生）
        default:
            parser_init(parser); // 出错了就复位
            break;
    }
    return false; // 还没收完，继续等待
}

