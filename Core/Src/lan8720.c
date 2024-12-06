/**
  ******************************************************************************
  * @file    lan8720.c
  * @author  MCD Application Team
  * @brief   This file provides a set of functions needed to manage the LAN742
  *          PHY devices.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "lan8720.h"
//#include "oled.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Component
  * @{
  */

/** @defgroup LAN8720 LAN8720
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/** @defgroup LAN8720_Private_Defines LAN8720 Private Defines
  * @{
  */
#define LAN8720_MAX_DEV_ADDR   ((uint32_t)31U)
/**
  * @}
  */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/** @defgroup LAN8720_Private_Functions LAN8720 Private Functions
  * @{
  */

/**
  * @brief  Register IO functions to component object
  * @param  pObj: device object  of LAN8720_Object_t.
  * @param  ioctx: holds device IO functions.
  * @retval LAN8720_STATUS_OK  if OK
  *         LAN8720_STATUS_ERROR if missing mandatory function
  */
int32_t  LAN8720_RegisterBusIO(lan8720_Object_t *pObj, lan8720_IOCtx_t *ioctx)
{
  if(!pObj || !ioctx->ReadReg || !ioctx->WriteReg || !ioctx->GetTick)
  {
    return LAN8720_STATUS_ERROR;
  }

  pObj->IO.Init = ioctx->Init;
  pObj->IO.DeInit = ioctx->DeInit;
  pObj->IO.ReadReg = ioctx->ReadReg;
  pObj->IO.WriteReg = ioctx->WriteReg;
  pObj->IO.GetTick = ioctx->GetTick;

  return LAN8720_STATUS_OK;
}

/**
  * @brief  Initialize the lan8720 and configure the needed hardware resources (blocking to ensure auto-negotiation is done)
  * @param  pObj: device object LAN8720_Object_t.
  * @retval LAN8720_STATUS_OK  if OK
  *         LAN8720_STATUS_ADDRESS_ERROR if cannot find device address
  *         LAN8720_STATUS_READ_ERROR if cannot read register
  */
 int32_t LAN8720_Init(lan8720_Object_t *pObj)
 {
   uint32_t regvalue = 0, addr = 0;
   int32_t status = LAN8720_STATUS_OK;

   if(pObj->Is_Initialized == 0)
   {
     if(pObj->IO.Init != 0)
     {
       /* GPIO and Clocks initialization */
       pObj->IO.Init();
     }

     /* for later check */
     pObj->DevAddr = LAN8720_MAX_DEV_ADDR + 1;

     /* Get the device address from special mode register */
     for(addr = 0; addr <= LAN8720_MAX_DEV_ADDR; addr ++)
     {
       if(pObj->IO.ReadReg(addr, LAN8720_SMR, &regvalue) < 0)
       {
         status = LAN8720_STATUS_READ_ERROR;
         /* Can't read from this device address
            continue with next address */
         continue;
       }

       if((regvalue & LAN8720_SMR_PHY_ADDR) == addr)
       {
         pObj->DevAddr = addr;
         status = LAN8720_STATUS_OK;
         break;
       }
     }

     if(pObj->DevAddr > LAN8720_MAX_DEV_ADDR)
     {
       status = LAN8720_STATUS_ADDRESS_ERROR;
     }

     /* 软复位 */
     if(pObj->IO.WriteReg(addr, LAN8720_BCR, LAN8720_BCR_SOFT_RESET) < 0){
    	 return LAN8720_STATUS_WRITE_ERROR;
     }
     do {
    	 if (pObj->IO.ReadReg(addr, LAN8720_BCR, &regvalue) < 0) {
    		 return LAN8720_STATUS_READ_ERROR;
    	 }
     } while(regvalue & LAN8720_BCR_SOFT_RESET);  // 等待软复位完成

     /* 开启 Auto-Negotiation */
     if (pObj->IO.ReadReg(addr, LAN8720_BCR, &regvalue) < 0) {
    	 return LAN8720_STATUS_READ_ERROR;
     }
     regvalue |= LAN8720_BCR_AUTONEGO_EN;
     if (pObj->IO.WriteReg(addr, LAN8720_BCR, regvalue) < 0) {
    	 return LAN8720_STATUS_WRITE_ERROR;
     }

     do {
    	 pObj->IO.ReadReg(addr, LAN8720_PHYSCSR, &regvalue);
     } while((regvalue & LAN8720_PHYSCSR_AUTONEGO_DONE) != LAN8720_PHYSCSR_AUTONEGO_DONE); // 等待 auto-negotiation 完成

     /* if device address is matched */
     if(status == LAN8720_STATUS_OK)
     {
       pObj->Is_Initialized = 1;
     }
   }

   return status;
 }

/**
  * @brief  De-Initialize the lan8720 and it's hardware resources
  * @param  pObj: device object LAN8720_Object_t.
  * @retval None
  */
int32_t LAN8720_DeInit(lan8720_Object_t *pObj)
{
  if(pObj->Is_Initialized)
  {
    if(pObj->IO.DeInit != 0)
    {
      if(pObj->IO.DeInit() < 0)
      {
        return LAN8720_STATUS_ERROR;
      }
    }

    pObj->Is_Initialized = 0;
  }

  return LAN8720_STATUS_OK;
}

/**
  * @brief  Disable the LAN8720 power down mode.
  * @param  pObj: device object LAN8720_Object_t.
  * @retval LAN8720_STATUS_OK  if OK
  *         LAN8720_STATUS_READ_ERROR if cannot read register
  *         LAN8720_STATUS_WRITE_ERROR if cannot write to register
  */
int32_t LAN8720_DisablePowerDownMode(lan8720_Object_t *pObj)
{
  uint32_t readval = 0;
  int32_t status = LAN8720_STATUS_OK;

  if(pObj->IO.ReadReg(pObj->DevAddr, LAN8720_BCR, &readval) >= 0)
  {
    readval &= ~LAN8720_BCR_POWER_DOWN;

    /* Apply configuration */
    if(pObj->IO.WriteReg(pObj->DevAddr, LAN8720_BCR, readval) < 0)
    {
      status =  LAN8720_STATUS_WRITE_ERROR;
    }
  }
  else
  {
    status = LAN8720_STATUS_READ_ERROR;
  }

  return status;
}

/**
  * @brief  Enable the LAN8720 power down mode.
  * @param  pObj: device object LAN8720_Object_t.
  * @retval LAN8720_STATUS_OK  if OK
  *         LAN8720_STATUS_READ_ERROR if cannot read register
  *         LAN8720_STATUS_WRITE_ERROR if cannot write to register
  */
int32_t LAN8720_EnablePowerDownMode(lan8720_Object_t *pObj)
{
  uint32_t readval = 0;
  int32_t status = LAN8720_STATUS_OK;

  if(pObj->IO.ReadReg(pObj->DevAddr, LAN8720_BCR, &readval) >= 0)
  {
    readval |= LAN8720_BCR_POWER_DOWN;

    /* Apply configuration */
    if(pObj->IO.WriteReg(pObj->DevAddr, LAN8720_BCR, readval) < 0)
    {
      status =  LAN8720_STATUS_WRITE_ERROR;
    }
  }
  else
  {
    status = LAN8720_STATUS_READ_ERROR;
  }

  return status;
}

/**
  * @brief  Start the auto negotiation process.
  * @param  pObj: device object LAN8720_Object_t.
  * @retval LAN8720_STATUS_OK  if OK
  *         LAN8720_STATUS_READ_ERROR if cannot read register
  *         LAN8720_STATUS_WRITE_ERROR if cannot write to register
  */
int32_t LAN8720_StartAutoNego(lan8720_Object_t *pObj)
{
  uint32_t readval = 0;
  int32_t status = LAN8720_STATUS_OK;

  if(pObj->IO.ReadReg(pObj->DevAddr, LAN8720_BCR, &readval) >= 0)
  {
    readval |= LAN8720_BCR_AUTONEGO_EN;

    /* Apply configuration */
    if(pObj->IO.WriteReg(pObj->DevAddr, LAN8720_BCR, readval) < 0)
    {
      status =  LAN8720_STATUS_WRITE_ERROR;
    }
  }
  else
  {
    status = LAN8720_STATUS_READ_ERROR;
  }

  return status;
}

/**
  * @brief  Get the link state of LAN8720 device.
  * @param  pObj: Pointer to device object.
  * @param  pLinkState: Pointer to link state
  * @retval LAN8720_STATUS_LINK_DOWN  if link is down
  *         LAN8720_STATUS_AUTONEGO_NOTDONE if Auto nego not completed
  *         LAN8720_STATUS_100MBITS_FULLDUPLEX if 100Mb/s FD
  *         LAN8720_STATUS_100MBITS_HALFDUPLEX if 100Mb/s HD
  *         LAN8720_STATUS_10MBITS_FULLDUPLEX  if 10Mb/s FD
  *         LAN8720_STATUS_10MBITS_HALFDUPLEX  if 10Mb/s HD
  *         LAN8720_STATUS_READ_ERROR if cannot read register
  *         LAN8720_STATUS_WRITE_ERROR if cannot write to register
  */
int32_t LAN8720_GetLinkState(lan8720_Object_t *pObj)
{
  uint32_t readval = 0;

  /* Read Status register  */
  if(pObj->IO.ReadReg(pObj->DevAddr, LAN8720_BSR, &readval) < 0)
  {
    return LAN8720_STATUS_READ_ERROR;
  }

  /* Read Status register again */
  if(pObj->IO.ReadReg(pObj->DevAddr, LAN8720_BSR, &readval) < 0)
  {
    return LAN8720_STATUS_READ_ERROR;
  }

  if((readval & LAN8720_BSR_LINK_STATUS) == 0)
  {
    /* Return Link Down status */
    return LAN8720_STATUS_LINK_DOWN;
  }

  /* Check Auto negotiation */
  if(pObj->IO.ReadReg(pObj->DevAddr, LAN8720_BCR, &readval) < 0)
  {
    return LAN8720_STATUS_READ_ERROR;
  }

  if((readval & LAN8720_BCR_AUTONEGO_EN) != LAN8720_BCR_AUTONEGO_EN)
  {
    if(((readval & LAN8720_BCR_SPEED_SELECT) == LAN8720_BCR_SPEED_SELECT) && ((readval & LAN8720_BCR_DUPLEX_MODE) == LAN8720_BCR_DUPLEX_MODE))  // 判断指定位是否为一
    {
      return LAN8720_STATUS_100MBITS_FULLDUPLEX;
    }
    else if ((readval & LAN8720_BCR_SPEED_SELECT) == LAN8720_BCR_SPEED_SELECT)
    {
      return LAN8720_STATUS_100MBITS_HALFDUPLEX;
    }
    else if ((readval & LAN8720_BCR_DUPLEX_MODE) == LAN8720_BCR_DUPLEX_MODE)
    {
      return LAN8720_STATUS_10MBITS_FULLDUPLEX;
    }
    else
    {
      return LAN8720_STATUS_10MBITS_HALFDUPLEX;
    }
  }
  else /* Auto Nego enabled */
  {
    if(pObj->IO.ReadReg(pObj->DevAddr, LAN8720_PHYSCSR, &readval) < 0)
    {
      return LAN8720_STATUS_READ_ERROR;
    }

    /* Check if auto nego not done */
    if((readval & LAN8720_PHYSCSR_AUTONEGO_DONE) == 0)
    {
      return LAN8720_STATUS_AUTONEGO_NOTDONE;
    }

    if((readval & LAN8720_PHYSCSR_HCDSPEEDMASK) == LAN8720_PHYSCSR_100BTX_FD)
    {
      return LAN8720_STATUS_100MBITS_FULLDUPLEX;
    }
    else if ((readval & LAN8720_PHYSCSR_HCDSPEEDMASK) == LAN8720_PHYSCSR_100BTX_HD)
    {
      return LAN8720_STATUS_100MBITS_HALFDUPLEX;
    }
    else if ((readval & LAN8720_PHYSCSR_HCDSPEEDMASK) == LAN8720_PHYSCSR_10BT_FD)
    {
      return LAN8720_STATUS_10MBITS_FULLDUPLEX;
    }
    else
    {
      return LAN8720_STATUS_10MBITS_HALFDUPLEX;
    }
  }
}

/**
  * @brief  Set the link state of LAN8720 device.
  * @param  pObj: Pointer to device object.
  * @param  pLinkState: link state can be one of the following
  *         LAN8720_STATUS_100MBITS_FULLDUPLEX if 100Mb/s FD
  *         LAN8720_STATUS_100MBITS_HALFDUPLEX if 100Mb/s HD
  *         LAN8720_STATUS_10MBITS_FULLDUPLEX  if 10Mb/s FD
  *         LAN8720_STATUS_10MBITS_HALFDUPLEX  if 10Mb/s HD
  * @retval LAN8720_STATUS_OK  if OK
  *         LAN8720_STATUS_ERROR  if parameter error
  *         LAN8720_STATUS_READ_ERROR if cannot read register
  *         LAN8720_STATUS_WRITE_ERROR if cannot write to register
  */
int32_t LAN8720_SetLinkState(lan8720_Object_t *pObj, uint32_t LinkState)
{
  uint32_t bcrvalue = 0;
  int32_t status = LAN8720_STATUS_OK;

  if(pObj->IO.ReadReg(pObj->DevAddr, LAN8720_BCR, &bcrvalue) >= 0)
  {
    /* Disable link config (Auto nego, speed and duplex) */
    bcrvalue &= ~(LAN8720_BCR_AUTONEGO_EN | LAN8720_BCR_SPEED_SELECT | LAN8720_BCR_DUPLEX_MODE);

    if(LinkState == LAN8720_STATUS_100MBITS_FULLDUPLEX)
    {
      bcrvalue |= (LAN8720_BCR_SPEED_SELECT | LAN8720_BCR_DUPLEX_MODE);
    }
    else if (LinkState == LAN8720_STATUS_100MBITS_HALFDUPLEX)
    {
      bcrvalue |= LAN8720_BCR_SPEED_SELECT;
    }
    else if (LinkState == LAN8720_STATUS_10MBITS_FULLDUPLEX)
    {
      bcrvalue |= LAN8720_BCR_DUPLEX_MODE;
    }
    else
    {
      /* Wrong link status parameter */
      status = LAN8720_STATUS_ERROR;
    }
  }
  else
  {
    status = LAN8720_STATUS_READ_ERROR;
  }

  if(status == LAN8720_STATUS_OK)
  {
    /* Apply configuration */
    if(pObj->IO.WriteReg(pObj->DevAddr, LAN8720_BCR, bcrvalue) < 0)
    {
      status = LAN8720_STATUS_WRITE_ERROR;
    }
  }

  return status;
}

/**
  * @brief  Enable loopback mode.
  * @param  pObj: Pointer to device object.
  * @retval LAN8720_STATUS_OK  if OK
  *         LAN8720_STATUS_READ_ERROR if cannot read register
  *         LAN8720_STATUS_WRITE_ERROR if cannot write to register
  */
int32_t LAN8720_EnableLoopbackMode(lan8720_Object_t *pObj)
{
  uint32_t readval = 0;
  int32_t status = LAN8720_STATUS_OK;

  if(pObj->IO.ReadReg(pObj->DevAddr, LAN8720_BCR, &readval) >= 0)
  {
    readval |= LAN8720_BCR_LOOPBACK;

    /* Apply configuration */
    if(pObj->IO.WriteReg(pObj->DevAddr, LAN8720_BCR, readval) < 0)
    {
      status = LAN8720_STATUS_WRITE_ERROR;
    }
  }
  else
  {
    status = LAN8720_STATUS_READ_ERROR;
  }

  return status;
}

/**
  * @brief  Disable loopback mode.
  * @param  pObj: Pointer to device object.
  * @retval LAN8720_STATUS_OK  if OK
  *         LAN8720_STATUS_READ_ERROR if cannot read register
  *         LAN8720_STATUS_WRITE_ERROR if cannot write to register
  */
int32_t LAN8720_DisableLoopbackMode(lan8720_Object_t *pObj)
{
  uint32_t readval = 0;
  int32_t status = LAN8720_STATUS_OK;

  if(pObj->IO.ReadReg(pObj->DevAddr, LAN8720_BCR, &readval) >= 0)
  {
    readval &= ~LAN8720_BCR_LOOPBACK;

    /* Apply configuration */
    if(pObj->IO.WriteReg(pObj->DevAddr, LAN8720_BCR, readval) < 0)
    {
      status =  LAN8720_STATUS_WRITE_ERROR;
    }
  }
  else
  {
    status = LAN8720_STATUS_READ_ERROR;
  }

  return status;
}

/**
  * @brief  Enable IT source.
  * @param  pObj: Pointer to device object.
  * @param  Interrupt: IT source to be enabled
  *         should be a value or a combination of the following:
  *         LAN8720_WOL_IT
  *         LAN8720_ENERGYON_IT
  *         LAN8720_AUTONEGO_COMPLETE_IT
  *         LAN8720_REMOTE_FAULT_IT
  *         LAN8720_LINK_DOWN_IT
  *         LAN8720_AUTONEGO_LP_ACK_IT
  *         LAN8720_PARALLEL_DETECTION_FAULT_IT
  *         LAN8720_AUTONEGO_PAGE_RECEIVED_IT
  * @retval LAN8720_STATUS_OK  if OK
  *         LAN8720_STATUS_READ_ERROR if cannot read register
  *         LAN8720_STATUS_WRITE_ERROR if cannot write to register
  */
int32_t LAN8720_EnableIT(lan8720_Object_t *pObj, uint32_t Interrupt)
{
  uint32_t readval = 0;
  int32_t status = LAN8720_STATUS_OK;

  if(pObj->IO.ReadReg(pObj->DevAddr, LAN8720_IMR, &readval) >= 0)
  {
    readval |= Interrupt;

    /* Apply configuration */
    if(pObj->IO.WriteReg(pObj->DevAddr, LAN8720_IMR, readval) < 0)
    {
      status =  LAN8720_STATUS_WRITE_ERROR;
    }
  }
  else
  {
    status = LAN8720_STATUS_READ_ERROR;
  }

  return status;
}

/**
  * @brief  Disable IT source.
  * @param  pObj: Pointer to device object.
  * @param  Interrupt: IT source to be disabled
  *         should be a value or a combination of the following:
  *         LAN8720_WOL_IT
  *         LAN8720_ENERGYON_IT
  *         LAN8720_AUTONEGO_COMPLETE_IT
  *         LAN8720_REMOTE_FAULT_IT
  *         LAN8720_LINK_DOWN_IT
  *         LAN8720_AUTONEGO_LP_ACK_IT
  *         LAN8720_PARALLEL_DETECTION_FAULT_IT
  *         LAN8720_AUTONEGO_PAGE_RECEIVED_IT
  * @retval LAN8720_STATUS_OK  if OK
  *         LAN8720_STATUS_READ_ERROR if cannot read register
  *         LAN8720_STATUS_WRITE_ERROR if cannot write to register
  */
int32_t LAN8720_DisableIT(lan8720_Object_t *pObj, uint32_t Interrupt)
{
  uint32_t readval = 0;
  int32_t status = LAN8720_STATUS_OK;

  if(pObj->IO.ReadReg(pObj->DevAddr, LAN8720_IMR, &readval) >= 0)
  {
    readval &= ~Interrupt;

    /* Apply configuration */
    if(pObj->IO.WriteReg(pObj->DevAddr, LAN8720_IMR, readval) < 0)
    {
      status = LAN8720_STATUS_WRITE_ERROR;
    }
  }
  else
  {
    status = LAN8720_STATUS_READ_ERROR;
  }

  return status;
}

/**
  * @brief  Clear IT flag.
  * @param  pObj: Pointer to device object.
  * @param  Interrupt: IT flag to be cleared
  *         should be a value or a combination of the following:
  *         LAN8720_WOL_IT
  *         LAN8720_ENERGYON_IT
  *         LAN8720_AUTONEGO_COMPLETE_IT
  *         LAN8720_REMOTE_FAULT_IT
  *         LAN8720_LINK_DOWN_IT
  *         LAN8720_AUTONEGO_LP_ACK_IT
  *         LAN8720_PARALLEL_DETECTION_FAULT_IT
  *         LAN8720_AUTONEGO_PAGE_RECEIVED_IT
  * @retval LAN8720_STATUS_OK  if OK
  *         LAN8720_STATUS_READ_ERROR if cannot read register
  */
int32_t  LAN8720_ClearIT(lan8720_Object_t *pObj, uint32_t Interrupt)
{
  uint32_t readval = 0;
  int32_t status = LAN8720_STATUS_OK;

  if(pObj->IO.ReadReg(pObj->DevAddr, LAN8720_ISFR, &readval) < 0)
  {
    status =  LAN8720_STATUS_READ_ERROR;
  }

  return status;
}

/**
  * @brief  Get IT Flag status.
  * @param  pObj: Pointer to device object.
  * @param  Interrupt: IT Flag to be checked,
  *         should be a value or a combination of the following:
  *         LAN8720_WOL_IT
  *         LAN8720_ENERGYON_IT
  *         LAN8720_AUTONEGO_COMPLETE_IT
  *         LAN8720_REMOTE_FAULT_IT
  *         LAN8720_LINK_DOWN_IT
  *         LAN8720_AUTONEGO_LP_ACK_IT
  *         LAN8720_PARALLEL_DETECTION_FAULT_IT
  *         LAN8720_AUTONEGO_PAGE_RECEIVED_IT
  * @retval 1 IT flag is SET
  *         0 IT flag is RESET
  *         LAN8720_STATUS_READ_ERROR if cannot read register
  */
int32_t LAN8720_GetITStatus(lan8720_Object_t *pObj, uint32_t Interrupt)
{
  uint32_t readval = 0;
  int32_t status = 0;

  if(pObj->IO.ReadReg(pObj->DevAddr, LAN8720_ISFR, &readval) >= 0)
  {
    status = ((readval & Interrupt) == Interrupt);
  }
  else
  {
    status = LAN8720_STATUS_READ_ERROR;
  }

  return status;
}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */
