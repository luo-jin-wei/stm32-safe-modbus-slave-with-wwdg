#include "frame_parser.h"
#include <string.h>  // 用到 memset

/* 超长帧丢弃计数器定义 */
volatile uint32_t g_frame_err_overlen_cnt = 0;

// 初始化：把状态机复位到 IDLE 状态
void parser_init(FrameParser_t *parser) {
    // 把整个结构体清零
    memset(parser, 0, sizeof(FrameParser_t));
    parser->state = STATE_IDLE;
}


bool parser_feed(FrameParser_t *parser, uint8_t byte) {
	
		parser->rx_count++;
		if (parser->rx_count > MAX_FRAME_TOTAL_LEN) {
				parser_init(parser);
				g_frame_err_overlen_cnt++;
				return false;
		}
		
		
    // 使用 switch 根据当前状态决定如何处理这个字节
    switch (parser->state) {

        // 1. 空闲状态
        case STATE_IDLE:
            // 假设协议固定从机地址是 0x01（Modbus 标准）
            // 不是 0x01，直接丢弃（忽略噪声）
            if (byte == 0x01) {
                parser->addr = byte;        // 存地址
								parser->rx_count = 1;
                parser->state = STATE_ADDR; // 切到下一个状态
            }
            //byte != 0x01，不干事
            break;

        //2. 接收功能码
        case STATE_ADDR:
            parser->func = byte;
            parser->state = STATE_FUNC;
            break;

        //  3. 接收长度
        case STATE_FUNC:
					if (byte > MAX_FRAME_DATA_LEN) 
						{
								parser_init(parser);
								g_frame_err_overlen_cnt++;
								return false;
						}
            parser->len = byte;
            parser->data_index = 0;          // 重置数据区指针
            parser->state = STATE_LEN;
            break;

        // 4. 接收数据区
        case STATE_LEN:
            // 把当前字节存入 data 数组
            parser->data[parser->data_index++] = byte;

            // 判断是否收够了 len 个字节
            if (parser->data_index >= parser->len) {
                parser->state = STATE_DATA;  // 数据收完了，切到 CRC 状态
                parser->crc_count = 0;       // 重置 CRC 计数器
            }
            break;

        //  5. 接收 CRC 校验（2个字节）
        case STATE_DATA:
            if (parser->crc_count == 0) {
                parser->crc_low = byte;     // 存 CRC 低字节
                parser->crc_count = 1;
            } else {
                parser->crc_high = byte;      // 存 CRC 高字节
                // 到这里，一帧完整数据已经收完了
                // 立刻复位状态机，准备接收下一帧
                parser->state = STATE_IDLE;
                return true;  // 返回 true，告诉主循环拆出一帧了
            }
            break;

        default:
            parser_init(parser); // 出错了就复位
            break;
    }
    return false; // 还没收完，继续等待
}

