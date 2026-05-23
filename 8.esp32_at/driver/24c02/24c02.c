#include "24c02_desc.h"
#include "24c02.h"

/*
 * Function: at24c02_wait_write_cycle
 * Description: Wait for the write cycle to complete (typical 5ms for AT24C02)-5ms
 */
static void at24c02_wait_write_cycle(void)
{
    volatile uint32_t i = 200000;
    while (i--){}
}

/*
 * Function: at24c02_init
 * Description: Initialize the AT24C02 EEPROM
 */
void at24c02_init(at24c02_handle_t dev)
{
    if(dev && dev->i2c_bus)
        i2c_init(dev->i2c_bus);
}

/*
 * Function: at24c02_read_byte
 * Description: Read a byte of data from the AT24C02 EEPROM
 */
uint8_t at24c02_read_byte(at24c02_handle_t dev, uint8_t read_addr)
{
    uint8_t temp = 0;

    i2c_start(dev->i2c_bus);
    i2c_send_byte(dev->i2c_bus, dev->device_addr);
    if(i2c_wait_ack(dev->i2c_bus))
    {
        i2c_stop(dev->i2c_bus);
        return 0; // 从机无响应
    }

    i2c_send_byte(dev->i2c_bus, read_addr%256);
    if(i2c_wait_ack(dev->i2c_bus))
    {
        i2c_stop(dev->i2c_bus);
        return 0; // 从机无响应
    }

    i2c_start(dev->i2c_bus);
    i2c_send_byte(dev->i2c_bus, dev->device_addr | 0x01); // 发送读命令
    if(i2c_wait_ack(dev->i2c_bus))
    {
        i2c_stop(dev->i2c_bus);
        return 0; // 从机无响应
    }

    temp = i2c_read_byte(dev->i2c_bus, 0); // 读取数据，不发送ACK
    i2c_stop(dev->i2c_bus);

    return temp;
}

/*
 * Function: at24c02_write_byte
 * Description: Write a byte of data to the AT24C02 EEPROM
 */
void at24c02_write_byte(at24c02_handle_t dev, uint8_t write_addr, uint8_t data)
{
    i2c_start(dev->i2c_bus);
    i2c_send_byte(dev->i2c_bus, dev->device_addr);//发送写地址
    if(i2c_wait_ack(dev->i2c_bus))
    {
        i2c_stop(dev->i2c_bus);
        return; // 从机无响应
    }

    i2c_send_byte(dev->i2c_bus, write_addr%256);//发送字地址
    if(i2c_wait_ack(dev->i2c_bus))
    {
        i2c_stop(dev->i2c_bus);
        return; // 从机无响应
    }

    i2c_send_byte(dev->i2c_bus, data);//发送数据
    if(i2c_wait_ack(dev->i2c_bus))
    {
        i2c_stop(dev->i2c_bus);
        return; // 从机无响应
    }

    i2c_stop(dev->i2c_bus);
    at24c02_wait_write_cycle(); // 等待写入周期完成
}


/**
 * @brief 页写 (Page Write)：连续写入多个字节
 * @note  AT24C02 的一页为 8 个字节。
 * 注意硬件限制：连续写入的数据不能跨越 8 字节的页边界，否则会翻卷覆盖本页开头的数据！
 */
void at24c02_write_page(at24c02_handle_t dev, uint8_t write_addr, uint8_t *pBuffer, uint8_t num_to_write)
{
    i2c_start(dev->i2c_bus);
    i2c_send_byte(dev->i2c_bus, dev->device_addr);//发送写地址
    if(i2c_wait_ack(dev->i2c_bus))
    {
        i2c_stop(dev->i2c_bus);
        return; // 从机无响应
    }

    i2c_send_byte(dev->i2c_bus, write_addr%256);//发送字地址
    if(i2c_wait_ack(dev->i2c_bus))
    {
        i2c_stop(dev->i2c_bus);
        return; // 从机无响应
    }   

    while(num_to_write--)
    {
        i2c_send_byte(dev->i2c_bus, *pBuffer);//发送数据
        if(i2c_wait_ack(dev->i2c_bus))
        {
            i2c_stop(dev->i2c_bus);
            return; // 从机无响应
        }
        pBuffer++;
    }

    i2c_stop(dev->i2c_bus);
    at24c02_wait_write_cycle(); // 等待写入周期完成
}

/**
 * @brief 连续读 (Sequential Read)：从指定地址连续读取一片数据
 */
void at24c02_read_buffer(at24c02_handle_t dev, uint16_t read_addr, uint8_t *pBuffer, uint16_t num_to_read) {
    i2c_start(dev->i2c_bus);
    i2c_send_byte(dev->i2c_bus, dev->device_addr); // 发送写地址 (Dummy Write)
    i2c_wait_ack(dev->i2c_bus);
    
    i2c_send_byte(dev->i2c_bus, read_addr % 256);  // 发送字地址
    i2c_wait_ack(dev->i2c_bus);
    
    i2c_start(dev->i2c_bus);                       // 重复起始条件
    i2c_send_byte(dev->i2c_bus, dev->device_addr | 0x01); // 发送读地址
    i2c_wait_ack(dev->i2c_bus);
    
    while(num_to_read) {
        if (num_to_read == 1) {
            *pBuffer = i2c_read_byte(dev->i2c_bus, 0); // 最后一个字节，主机回复 NACK
        } else {
            *pBuffer = i2c_read_byte(dev->i2c_bus, 1); // 还有数据要读，主机回复 ACK
        }
        pBuffer++;
        num_to_read--;
    }
    i2c_stop(dev->i2c_bus);
}
