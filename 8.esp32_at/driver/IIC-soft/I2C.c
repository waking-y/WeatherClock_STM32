#include "I2C_desc.h"
#include "I2C.h"

static void i2c_delay(void)
{
    volatile uint32_t i = 200;
    while (i--)
        ;
}

/*
 * Function: i2c_init
 * Description: Initialize the I2C interface
 */
void i2c_init(i2c_handle_t i2c)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_StructInit(&GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = i2c->SCL_Pin;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(i2c->SCL_Port, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = i2c->SDA_Pin;
    GPIO_Init(i2c->SDA_Port, &GPIO_InitStruct);

    GPIO_SetBits(i2c->SCL_Port, i2c->SCL_Pin);
    GPIO_SetBits(i2c->SDA_Port, i2c->SDA_Pin);
}

/*
 * Function: i2c_start
 * Description: Generate I2C start condition
 */
void i2c_start(i2c_handle_t i2c)
{
    GPIO_SetBits(i2c->SDA_Port, i2c->SDA_Pin);
    GPIO_SetBits(i2c->SCL_Port, i2c->SCL_Pin);
    i2c_delay();
    GPIO_ResetBits(i2c->SDA_Port, i2c->SDA_Pin); // SCL保持高电平，SDA由高变低
    i2c_delay();
    GPIO_ResetBits(i2c->SCL_Port, i2c->SCL_Pin);
}

/*
 * Function: i2c_stop
 * Description: Generate I2C stop condition
 */
void i2c_stop(i2c_handle_t i2c)
{
    GPIO_ResetBits(i2c->SCL_Port, i2c->SCL_Pin);
    GPIO_ResetBits(i2c->SDA_Port, i2c->SDA_Pin);
    i2c_delay();
    GPIO_SetBits(i2c->SCL_Port, i2c->SCL_Pin);
    i2c_delay();
    GPIO_SetBits(i2c->SDA_Port, i2c->SDA_Pin);// SCL保持高电平，SDA由低变高
    i2c_delay();
}

/*
 * Function: i2c_wait_ack
 * Description: Wait for I2C acknowledge
 */
uint8_t i2c_wait_ack(i2c_handle_t i2c)
{
    uint8_t ack = 0;

    GPIO_SetBits(i2c->SDA_Port, i2c->SDA_Pin);
    i2c_delay();
    GPIO_SetBits(i2c->SCL_Port, i2c->SCL_Pin);
    i2c_delay();

    if (GPIO_ReadInputDataBit(i2c->SDA_Port, i2c->SDA_Pin) == 1)
    {
        ack = 1; // 无应答 NACK
    }
    else
    {
        ack = 0; // 应答 ACK
    }

    GPIO_ResetBits(i2c->SCL_Port, i2c->SCL_Pin);
    return ack;
}

/*
 * Function: i2c_ack
 * Description: Send I2C acknowledge
 */
void i2c_ack(i2c_handle_t i2c)
{
    GPIO_ResetBits(i2c->SCL_Port, i2c->SCL_Pin);
    GPIO_ResetBits(i2c->SDA_Port, i2c->SDA_Pin); //产生应答
    i2c_delay();
    GPIO_SetBits(i2c->SCL_Port, i2c->SCL_Pin);
    i2c_delay();
    GPIO_ResetBits(i2c->SCL_Port, i2c->SCL_Pin);
}

/*
 * Function: i2c_nack
 * Description: Send I2C non-acknowledge
 */
void i2c_nack(i2c_handle_t i2c)
{
    GPIO_ResetBits(i2c->SCL_Port, i2c->SCL_Pin);
    GPIO_SetBits(i2c->SDA_Port, i2c->SDA_Pin); //产生非应答
    i2c_delay();
    GPIO_SetBits(i2c->SCL_Port, i2c->SCL_Pin);
    i2c_delay();
    GPIO_ResetBits(i2c->SCL_Port, i2c->SCL_Pin);
}

/*
 * Function: i2c_send_byte
 * Description: Send a byte of data over I2C
 */
void i2c_send_byte(i2c_handle_t i2c, uint8_t byte)
{
    uint8_t i;
    GPIO_ResetBits(i2c->SCL_Port, i2c->SCL_Pin); // SCL保持低电平，发送数据
    for(i = 0; i < 8; i++)
    {
        if(byte & 0x80) // 先发送最高位
            GPIO_SetBits(i2c->SDA_Port, i2c->SDA_Pin);
        else
            GPIO_ResetBits(i2c->SDA_Port, i2c->SDA_Pin);
        
        byte <<= 1; // 左移一位，准备发送下一位
        i2c_delay();
        GPIO_SetBits(i2c->SCL_Port, i2c->SCL_Pin); // SCL保持高电平，数据被从机读取
        i2c_delay();
        GPIO_ResetBits(i2c->SCL_Port, i2c->SCL_Pin);
    }
}

/*
 * Function: i2c_read_byte
 * Description: Read a byte of data over I2C
 */
uint8_t i2c_read_byte(i2c_handle_t i2c, uint8_t ack)
{
    uint8_t i, receive = 0;

    GPIO_SetBits(i2c->SDA_Port, i2c->SDA_Pin); // 释放SDA线，准备接收数据
    for(i = 0; i < 8; i++)
    {
        GPIO_ResetBits(i2c->SCL_Port, i2c->SCL_Pin);
        i2c_delay();
        GPIO_SetBits(i2c->SCL_Port, i2c->SCL_Pin); // SCL保持高电平，从机将数据放在SDA线上
        i2c_delay();
        receive <<= 1; // 左移一位，为接收下一位数据做准备
        if(GPIO_ReadInputDataBit(i2c->SDA_Port, i2c->SDA_Pin))
            receive |= 0x01; // 读取到1
            
        i2c_delay();
    }

    if(ack)
        i2c_ack(i2c); // 发送ACK
    else
        i2c_nack(i2c); // 发送NACK

    return receive;
}
