/**
 * 路径及文件名：  /SourceCode/config.h
 * 说明：          引脚定义，模块中的功能控制。所有对引脚的修改和对模块功能是否启用的控制都必须在本文件中完成。
 */

/*配置定义开始*/
#define FUNCTION_DISABLED                0                      //禁用功能
#define FUNCTION_ENABLED                 1                      //启用功能
/*配置定义结束*/

/*电平定义开始*/
#define HIGH                             1                      //高电平
#define LOW                              0                      //低电平
/*电平定义结束*/

/*1602液晶显示屏配置开始*/
#define LCD1602_FUNCTION_BACKLIGHT       FUNCTION_ENABLED       //背光功能
#define LCD1602_PIN_BACKLIGHT            P2^3                   //背光引脚
#define LCD1602_PIN_ENABLE               P2^2                   //使能引脚
#define LCD1602_PIN_READWRITE            P2^1                   //读写选择引脚
#define LCD1602_PIN_REGISTER             P2^0                   //命令数据选择引脚
#define LCD1602_PORT_BUS                 P0                     //总线端口
#define LCD1602_STATUS_BACKLIGHT_OFF     HIGH                   //背光关
#define LCD1602_STATUS_BACKLIGHT_ON      LOW                    //背光开
/*1602液晶显示屏配置结束*/

/*键盘配置开始*/
#define KEYBOARD_PORT_SCAN               P1                     //扫描端口
/*键盘配置结束*/

/**
 * 以上为 RF 开发板 + LCD1602 时的配置
 * 晶振频率：  22.1184MHz
 */
