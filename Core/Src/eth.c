

/* 包含头文件 ----------------------------------------------------------------*/
#include "eth.h"
//#include "lan8720.h"
#include "string.h"
//#include "usart/bsp_debug_usart.h"

/* 私有类型定义 --------------------------------------------------------------*/

/* 私有宏定义 ----------------------------------------------------------------*/

#if defined ( __ICCARM__ ) /*!< IAR Compiler */
  #pragma data_alignment=4
#endif
__ALIGN_BEGIN ETH_DMADescTypeDef  DMARxDscrTab[ETH_RXBUFNB] __ALIGN_END;/* Ethernet Rx MA Descriptor */

#if defined ( __ICCARM__ ) /*!< IAR Compiler */
  #pragma data_alignment=4
#endif
__ALIGN_BEGIN ETH_DMADescTypeDef  DMATxDscrTab[ETH_TXBUFNB] __ALIGN_END;/* Ethernet Tx DMA Descriptor */

#if defined ( __ICCARM__ ) /*!< IAR Compiler */
  #pragma data_alignment=4
#endif
__ALIGN_BEGIN uint8_t Rx_Buff[ETH_RXBUFNB][ETH_RX_BUF_SIZE] __ALIGN_END; /* Ethernet Receive Buffer */

#if defined ( __ICCARM__ ) /*!< IAR Compiler */
  #pragma data_alignment=4
#endif
__ALIGN_BEGIN uint8_t Tx_Buff[ETH_TXBUFNB][ETH_TX_BUF_SIZE] __ALIGN_END; /* Ethernet Transmit Buffer */


/* 私有变量 ------------------------------------------------------------------*/
ETH_HandleTypeDef heth;

uint8_t RecvLength=0;
uint32_t current_pbuf_idx =0;

/* 扩展变量 ------------------------------------------------------------------*/

/* 私有函数原形 --------------------------------------------------------------*/

/* 函数体 --------------------------------------------------------------------*/
/**
  * 函数功能: ETH初始化
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 无
  */
//void MX_ETH_Init(void)
//{
//  uint32_t regvalue = 0;
//
//  uint8_t macaddress[6]= { MAC_ADDR0, MAC_ADDR1, MAC_ADDR2, MAC_ADDR3, MAC_ADDR4, MAC_ADDR5 };
//
//  heth.Instance = ETH;
//  heth.Init.MACAddr = macaddress;
//  heth.Init.AutoNegotiation = ETH_AUTONEGOTIATION_ENABLE;
//  heth.Init.Speed = ETH_SPEED_100M;
//  heth.Init.DuplexMode = ETH_MODE_FULLDUPLEX;
//  heth.Init.MediaInterface = ETH_MEDIA_INTERFACE_RMII;
//  heth.Init.RxMode = ETH_RXINTERRUPT_MODE;
//  heth.Init.ChecksumMode = ETH_CHECKSUM_BY_HARDWARE;
//  heth.Init.PhyAddress = LAN8720_PHY_ADDRESS;
//
//  /* configure ethernet peripheral (GPIOs, clocks, MAC, DMA) */
//  if (HAL_ETH_Init(&heth) == HAL_OK)
//  {
//    printf("ETH初始化成功\n");
//  }
//  else
//  {
//    printf("ETH初始化失败\n");
//  }
//
//  /* Initialize Tx Descriptors list: Chain Mode */
//  HAL_ETH_DMATxDescListInit(&heth, DMATxDscrTab, &Tx_Buff[0][0], ETH_TXBUFNB);
//
//  /* Initialize Rx Descriptors list: Chain Mode  */
//  HAL_ETH_DMARxDescListInit(&heth, DMARxDscrTab, &Rx_Buff[0][0], ETH_RXBUFNB);
//
//	/* Enable MAC and DMA transmission and reception */
//	HAL_ETH_Start(&heth);
//
//  /**** Configure PHY to generate an interrupt when Eth Link state changes ****/
//  /* Read Register Configuration */
//  HAL_ETH_ReadPHYRegister(&heth, PHY_MICR, &regvalue);
//
//  regvalue |= (PHY_MICR_INT_EN | PHY_MICR_INT_OE);
//
//  /* Enable Interrupts */
//  HAL_ETH_WritePHYRegister(&heth, PHY_MICR, regvalue );
//
//  /* Read Register Configuration */
//  HAL_ETH_ReadPHYRegister(&heth, PHY_MISR, &regvalue);
//
//  regvalue |= PHY_MISR_LINK_INT_EN;
//
//  /* Enable Interrupt on change of link status */
//  HAL_ETH_WritePHYRegister(&heth, PHY_MISR, regvalue);
//
//
//}

//void printfBuffer(uint8_t *dat, uint32_t len )
//{
//    //printf("\r\n");
//    for( int i=0; i<len; i++ )
//    {
//        if( (i % 32) == 0 && i != 0 )
//            printf("\r\n");
//        else if( (i % 8) == 0 && i != 0 )
//            printf("  ");
//
//        printf("%0.2X ",*dat++);
//    }
//    printf("\r\n\r\n");
//}

/**
  * 函数功能: ETH引脚初始化
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 无
  */
//void HAL_ETH_MspInit(ETH_HandleTypeDef* ethHandle)
//{
//
//    GPIO_InitTypeDef GPIO_InitStructure;
//    /* 使能GPIO端口时钟 */
//    ETH_GPIO_ClK_ENABLE();
//    /* 使能ETH外设时钟 */
//    ETH_RCC_CLK_ENABLE();
//    /**ETH GPIO Configuration
//    PB2     ------> ETH_RST
//    PC1     ------> ETH_MDC
//    PA1     ------> ETH_REF_CLK
//    PA2     ------> ETH_MDIO
//    PA7     ------> ETH_CRS_DV
//    PC4     ------> ETH_RXD0
//    PC5     ------> ETH_RXD1
//    PG11     ------> ETH_TX_EN
//    PG13     ------> ETH_TXD0
//    PG14     ------> ETH_TXD1
//    */
//    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);
//    GPIO_InitStructure.Pin = GPIO_PIN_2;
//    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
//    GPIO_InitStructure.Pull = GPIO_NOPULL;
//    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
//    GPIO_InitStructure.Alternate = GPIO_AF0_TRACE;
//    HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
//
//    GPIO_InitStructure.Pin = GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5;
//    GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
//    GPIO_InitStructure.Pull = GPIO_NOPULL;
//    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
//    GPIO_InitStructure.Alternate = GPIO_AFx_ETH;
//    HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);
//
//    GPIO_InitStructure.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_7;
//    GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
//    GPIO_InitStructure.Pull = GPIO_NOPULL;
//    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
//    GPIO_InitStructure.Alternate = GPIO_AFx_ETH;
//    HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
//
//    GPIO_InitStructure.Pin = GPIO_PIN_11 | GPIO_PIN_13 | GPIO_PIN_14;
//    GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
//    GPIO_InitStructure.Pull = GPIO_NOPULL;
//    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
//    GPIO_InitStructure.Alternate = GPIO_AFx_ETH;
//    HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);
//
////    HAL_NVIC_SetPriority(ETH_IRQn, 1, 0);
////    HAL_NVIC_EnableIRQ(ETH_IRQn);
//}


/**
  * 函数功能: PHY初始化
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 无
  */
//void PHY_Init(void)
//{
//
//	uint8_t status=0;
//
//	uint32_t duplex, speed = 0;
//	int32_t PHYLinkState;
//
//	status=LAN8720_Init();
//	do
//	{
//		HAL_Delay(100);
//
//		PHYLinkState = LAN8720_GetLinkState();
//
//		printf("LAN8720_GetLinkState = %d \r\n",PHYLinkState);
//
//	}while(PHYLinkState <= LAN8720_STATUS_LINK_DOWN);
//
//	switch (PHYLinkState)
//	{
//	case LAN8720_STATUS_100MBITS_FULLDUPLEX:
//		duplex = ETH_MODE_FULLDUPLEX;
//		speed = ETH_SPEED_100M;
//
//		//printf("LAN8720_STATUS_100MBITS_FULLDUPLEX \r\n");
//		break;
//	case LAN8720_STATUS_100MBITS_HALFDUPLEX:
//		duplex = ETH_MODE_HALFDUPLEX;
//		speed = ETH_SPEED_100M;
//
//		//printf("LAN8720_STATUS_100MBITS_HALFDUPLEX \r\n");
//		break;
//	case LAN8720_STATUS_10MBITS_FULLDUPLEX:
//		duplex = ETH_MODE_FULLDUPLEX;
//		speed = ETH_SPEED_10M;
//
//		//printf("LAN8720_STATUS_10MBITS_FULLDUPLEX \r\n");
//		break;
//	case LAN8720_STATUS_10MBITS_HALFDUPLEX:
//		duplex = ETH_MODE_HALFDUPLEX;
//		speed = ETH_SPEED_10M;
//
//		//printf("LAN8720_STATUS_10MBITS_HALFDUPLEX \r\n");
//		break;
//	default:
//		duplex = ETH_MODE_FULLDUPLEX;
//		speed = ETH_SPEED_100M;
//
//		//printf("ETH_FULLDUPLEX_MODE ETH_SPEED_100M\r\n");
//		break;
//	}
//
//	heth.Init.DuplexMode = duplex;
//	heth.Init.Speed = speed;
//
//	/* ETHERNET MAC Re-Configuration */
//	HAL_ETH_ConfigMAC(&heth, (ETH_MACInitTypeDef *) NULL);
//
//}

/**
  * 函数功能: 数据输出
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 无
  */
void low_level_output(uint8_t *p,uint32_t length)
{
	HAL_StatusTypeDef state;
	__IO ETH_DMADescTypeDef *DmaTxDesc;

	DmaTxDesc = heth.TxDesc;

	if ((DmaTxDesc->Status & ETH_DMATXDESC_OWN) != (uint32_t)RESET)
	{
		goto error;
	}

	/* Copy data to Tx buffer*/
	memcpy((uint8_t *)(DmaTxDesc->Buffer1Addr), (uint8_t *)p, length);

	/* wait for unlocked */
	while (heth.Lock == HAL_LOCKED)
	{
	}

	state = HAL_ETH_TransmitFrame(&heth, length);
	if (state != HAL_OK)
	{
		goto error;
	}

error:

	/* When Transmit Underflow flag is set, clear it and issue a Transmit Poll Demand to resume transmission */
	if ((heth.Instance->DMASR & ETH_DMASR_TUS) != (uint32_t)RESET)
	{
		/* Clear TUS ETHERNET DMA flag */
		heth.Instance->DMASR = ETH_DMASR_TUS;

		/* Resume DMA transmission*/
		heth.Instance->DMATPDR = 0;
	}

}

void low_level_input()
{

}


void HAL_ETH_RxCpltCallback(ETH_HandleTypeDef *heth)
{
	low_level_input();
	printf("rx isr\r\n");

}



void HAL_ETH_TxCpltCallback(ETH_HandleTypeDef *heth)
{
  //printf("tx isr\r\n");

}

int bfin_EMAC_send (void *packet, int length)
{
	memcpy(&Tx_Buff[0][0],packet,length);

	low_level_output(Tx_Buff[0],length);

	return 0;
}

int bfin_EMAC_recv (uint8_t * packet, size_t size)
{
	uint32_t framelength = 0;
//	__IO ETH_DMADescTypeDef *dmarxdesc;
	volatile ETH_DMADescTypeDef *dmarxdesc;
	uint32_t i=0;

	HAL_StatusTypeDef status = HAL_ETH_GetReceivedFrame(&heth);

	if( status == HAL_OK)
	{
			framelength=heth.RxFrameInfos.length;

			memcpy(packet, Rx_Buff[current_pbuf_idx], framelength);

			if(current_pbuf_idx < (ETH_RX_DESC_CNT -1))
			{
					current_pbuf_idx++;
			}
			else
			{
					current_pbuf_idx = 0;
			}

			/* Release descriptors to DMA */
			/* Point to first descriptor */
			dmarxdesc = heth.RxFrameInfos.FSRxDesc;
			/* Set Own bit in Rx descriptors: gives the buffers back to DMA */
			for (i=0; i< heth.RxFrameInfos.SegCount; i++)
			{
				dmarxdesc->Status |= ETH_DMARXDESC_OWN;
				dmarxdesc = (ETH_DMADescTypeDef *)(dmarxdesc->Buffer2NextDescAddr);
			}

			/* Clear Segment_Count */
			heth.RxFrameInfos.SegCount =0;

			/* When Rx Buffer unavailable flag is set: clear it and resume reception */
			if ((heth.Instance->DMASR & ETH_DMASR_RBUS) != (uint32_t)RESET)
			{
				/* Clear RBUS ETHERNET DMA flag */
				heth.Instance->DMASR = ETH_DMASR_RBUS;
				/* Resume DMA reception */
				heth.Instance->DMARPDR = 0;
			}
			return framelength;
	}

	return -1;
}
