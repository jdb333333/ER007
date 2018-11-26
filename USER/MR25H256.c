#include "MR25H256.h"


#define WREN    0x06
#define WRDI    0x04
#define RDSR    0x05
#define WRSR    0x01
#define READ    0x03
#define WRITE   0x02
#define SLEEP   0xb9
#define WAKE    0xab
#define DUMMY   0xff

/*******************************************************************************
* Function Name  : MR25H256_Config
* Description    : Configures the SPI BUS.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MR25H256_Init(void)
{
  SPI_InitTypeDef    SPI_InitStructure;
  GPIO_InitTypeDef GPIO_InitStruct;  	
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	
  GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_UP;  

  GPIO_InitStruct.GPIO_Pin=GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5;
  GPIO_Init(GPIOB,&GPIO_InitStruct);
   	  
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI1);  
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_SPI1);	
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI1);
	
	//CS
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_UP;  

	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_4;
  GPIO_Init(GPIOA,&GPIO_InitStruct); 

	MRAM_SET_CS();
												   
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; 
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master; 
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b; 
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;	//spi mode 0
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft; 
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;//  60MHz/2
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB; 
  SPI_InitStructure.SPI_CRCPolynomial = 7; 
  SPI_Init(SPI1,&SPI_InitStructure); 
  SPI_Cmd(SPI1,ENABLE); 


}
/*******************************************************************************
* Function Name  : SPI_MRAM_WR
* Description    : Write and Read data with SPI BUS.
* Input          : uint8_t data --- the send command
* Output         : uint8_t recv --- the receive data
* Return         : None
*******************************************************************************/
uint8_t SPI_MRAM_WR(uint8_t data)
{
	  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
		SPI_I2S_SendData(SPI1, data);
		while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
		return SPI_I2S_ReceiveData(SPI1);
}
/*******************************************************************************
* Function Name  : MRAM_RDSR
* Description    : The Read Status Register (RDSR) command allows the Status Register to be read.
* Input          : None
* Output         : None
* Return         : uint8_t status --- Read the status of the MR25H256.
*******************************************************************************/
uint8_t MRAM_RDSR(void)
{
    uint8_t status;

    MRAM_CLR_CS();
		//xputs("jdb--->MRAM_RDSR_START\r\n");

    SPI_MRAM_WR(RDSR);
    status = SPI_MRAM_WR(DUMMY);
	
		//xputs("jdb--->MRAM_RDSR_END\r\n");
    MRAM_SET_CS();

    return status;
}
/*******************************************************************************
* Function Name  : SPI_Write_Read
* Description    : Sets the Write Enable Latch (WEL) bit in the status register to 1.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MRAM_WREN(void)
{
    MRAM_CLR_CS();
		//xputs("jdb--->MRAM_WREN_START\r\n");

    SPI_MRAM_WR(WREN);

		//xputs("jdb--->MRAM_WREN_END\r\n");
    MRAM_SET_CS();
}
/*******************************************************************************
* Function Name  : MRAM_WRDI
* Description    : Resets the WEL bit in the status register to 0.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MRAM_WRDI(void)
{

    MRAM_CLR_CS();
		//xputs("jdb--->MRAM_WRDI_START\r\n");

    SPI_MRAM_WR(WRDI);

		//xputs("jdb--->MRAM_WRDI_END\r\n");
    MRAM_SET_CS();
}
/*******************************************************************************
* Function Name  : MRAM_WRSR
* Description    : New values to be written to the Status Register.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MRAM_WRSR(uint8_t cmd)
{
    MRAM_WREN();

    MRAM_CLR_CS();
		//xputs("jdb--->MRAM_WRSR_START\r\n");
	
    SPI_MRAM_WR(WRSR);
    SPI_MRAM_WR(cmd);
	
		//xputs("jdb--->MRAM_WRSR_END\r\n");
    MRAM_SET_CS();

    MRAM_WRDI();
}
/*******************************************************************************
* Function Name  : MRAM_READ
* Description    : The Read Data Bytes (READ) command allows data bytes to be
                   read starting at an address specified by the 16-bit address.
* Input          : uint32_t Addr
                   uint16_t Len
* Output         : uint8_t * Recv
* Return         : None
*******************************************************************************/
uint8_t MRAM_READ(uint8_t * Recv, uint32_t Addr, uint16_t Len)
{
    uint16_t i;

		if ((Addr + Len > MRAM_SIZE)
				||(Len == 0))
			return (ERROR);	
	
	
    MRAM_CLR_CS();

    SPI_MRAM_WR(READ);
		SPI_MRAM_WR((Addr>>16)&0xff);
    SPI_MRAM_WR((Addr>>8)&0xff);
    SPI_MRAM_WR(Addr&0xff);
    for(i=0;i<Len;i++)
    {
        Recv[i] = SPI_MRAM_WR(DUMMY);
    }
		
    MRAM_SET_CS();
		
		return (SUCCESS);
}
/*******************************************************************************
* Function Name  : MRAM_WRITE
* Description    : The Write Data Bytes (WRITE) command allows data bytes to
                   be written starting at an address specified by	the	16-bit
                   address.
* Input          : uint8_t * Send
                   uint32_t Addr
                   uint16_t Len
* Output         : None
* Return         : None
*******************************************************************************/
uint8_t MRAM_WRITE(uint8_t * Send, uint32_t Addr, uint16_t Len)
{
    uint16_t i;
	
		if ((Addr + Len > MRAM_SIZE)
				||(Len == 0))
			return (ERROR);	
		
    MRAM_WREN();
	
    MRAM_CLR_CS();

    SPI_MRAM_WR(WRITE);
		SPI_MRAM_WR((Addr>>16)&0xff);
    SPI_MRAM_WR((Addr>>8)&0xff);
    SPI_MRAM_WR(Addr&0xff);

    for(i=0;i<Len;i++)
    {
        SPI_MRAM_WR(Send[i]);
    }

    MRAM_SET_CS();

    MRAM_WRDI();
		
		return (SUCCESS);
}
/*******************************************************************************
* Function Name  : MRAM_SLEEP
* Description    : The Enter Sleep Mode (SLEEP).
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MRAM_SLEEP(void)
{

    MRAM_CLR_CS();
		//xputs("jdb--->MRAM_SLEEP_START\r\n");

    SPI_MRAM_WR(SLEEP);

		//xputs("jdb--->MRAM_SLEEP_END\r\n");
    MRAM_SET_CS();
}
/*******************************************************************************
* Function Name  : MRAM_WAKE
* Description    : 	Exit Sleep Mode (WAKE).
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MRAM_WAKE(void)
{

    MRAM_CLR_CS();
		//xputs("jdb--->MRAM_WAKE_START\r\n");

    SPI_MRAM_WR(WAKE);

		//xputs("jdb--->MRAM_WAKE_END\r\n");
    MRAM_SET_CS();
}

