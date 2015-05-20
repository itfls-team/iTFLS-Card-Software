/**
 * 路径及文件名：  /SourceCode/common/common.c
 * 说明：          定义所有的公共函数。
 * 函数列表：      extern void delayMs(unsigned int ms)
 *                 extern void delayNop(void)
 */

#include "../common/common.h"

extern void delayMs(unsigned int ms) //误差：每毫秒约-0.1085微秒
{
    unsigned char a,b,c;
    
    while (ms--) {
        for (c = 2; c > 0; c--) {
            for (b = 4; b > 0; b--) {
                for (a = 113; a > 0; a--) {
                    ;
                }
            }
        }
    }
}

extern void delayNop(void)
{
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
}
/*拼接字符函数，待用
extern void append(unsigned char* string, unsigned char character, unsigned int lenth)
{
	int digit;
	
	for (digit = 0; digit < lenth; digit++) {
		if (string[digit] == '\0') {
			string[digit] = character;
			string[digit + 1] = '\0';
			return;
		}
	}
	
	for (digit = 0; digit < lenth; digit++) {
		string[digit] = string[digit + 1];
	}
	string[lenth - 1] = character;
}
*/
