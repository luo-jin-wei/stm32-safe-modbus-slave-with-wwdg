#include "frame_parser.h"
#include <string.h>

/* 超长帧丢弃计数器定义 */
volatile uint32_t g_frame_err_overlen_cnt = 0;

/* 根据功能码确定请求帧的数据区长度 */
static uint8_t get_data_expected(uint8_t func)
{
    switch (func) {
        case 0x03:  // 读保持寄存器请求：起始地址(2) + 寄存器数量(2) = 4
        case 0x06:  // 写单个寄存器请求：起始地址(2) + 写入值(2) = 4
            return 4;
        default:
            return 4;  // 默认按 4 字节处理（异常时超时机制会清掉）
    }
}

void parser_init(FrameParser_t *parser)
{
    memset(parser, 0, sizeof(FrameParser_t));
    parser->state = STATE_IDLE;
}

bool parser_feed(FrameParser_t *parser, uint8_t byte)
{
    parser->rx_count++;

    /* 第一层保护：整帧总长度超限，强制丢弃 */
    if (parser->rx_count > MAX_FRAME_TOTAL_LEN) {
        parser_init(parser);
        g_frame_err_overlen_cnt++;
        return false;
    }

    switch (parser->state) {

        case STATE_IDLE:
            /* Modbus 从机只响应指定地址（默认 0x01） */
            if (byte == 0x01) {
                parser->addr = byte;
                parser->rx_count = 1;
                parser->state = STATE_ADDR;
            }
            break;

        case STATE_ADDR:
            parser->func = byte;
            parser->data_expected = get_data_expected(byte);
            parser->data_len = 0;
            parser->state = STATE_FUNC;
            break;

        case STATE_FUNC:
            /* 第二层保护：非法功能码直接丢弃（可选） */
            /* 开始收数据区 */
            parser->data[parser->data_len++] = byte;
            if (parser->data_len >= parser->data_expected) {
                parser->state = STATE_DATA;
                parser->crc_count = 0;
            }
            break;

        case STATE_DATA:
            /* 收 CRC（2 字节） */
            if (parser->crc_count == 0) {
                parser->crc_low = byte;
                parser->crc_count = 1;
            } else {
                parser->crc_high = byte;
                parser->state = STATE_IDLE;
                return true;  // 一帧完整接收完毕
            }
            break;

        default:
            parser_init(parser);
            break;
    }
    return false;
}

