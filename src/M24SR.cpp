/**
 ******************************************************************************
 * @file    m24sr_class.cpp
 * @author  ST Central Labs
 * @version V2.0.0
 * @date    28 Apr 2017
 * @brief   This file provides a set of functions to interface with the M24SR
 *          device.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */



/* Includes ------------------------------------------------------------------*/
#include "m24sr_def.h"
#include "M24SR.h"
#include "NDefNfcTagM24SR.h"

#include "RecordText.h"
#include "RecordURI.h"
#include "RecordAAR.h"
#include "RecordMail.h"
#include "RecordSMS.h"
#include "RecordMimeType.h"

#ifdef GPIO_PIN_RESET
  #undef GPIO_PIN_RESET
#endif
#define GPIO_PIN_RESET (0)

#ifdef GPIO_PIN_SET
  #undef GPIO_PIN_SET
#endif
#define GPIO_PIN_SET (1)

#define M24SR_MAX_BYTE_OPERATION_LENGHT (246)

#define M24SR_MAX_I2C_ACCESS_TRY 1000

/**
 * default password, also used to enable super user mode through the I2C channel
 */
const uint8_t M24SR::DEFAULT_PASSWORD[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

/** @addtogroup M24SR_Driver
 * @{
 *  @brief  <b>This folder contains the driver layer of M24SR family (M24SR64, M24SR16, M24SR04, M24SR02)</b>
 */

/** @defgroup drv_M24SR
 * @{
 *  @brief  This file contains the driver which implements all the M24SR commands.
 */

#ifndef errchk

/** value returned by the NFC chip when a command is successfully completed */
#define NFC_COMMAND_SUCCESS 0x9000

/** call the fCall function and check that the return status is M24SR_SUCCESS,
 *  otherwise return the error status*/
#define errchk(fCall) {\
  const int status = (int) (fCall); \
  if((status!=M24SR_SUCCESS)){ \
    return (M24SR::StatusTypeDef)status; \
  }\
}
#endif

/**
 * @brief  This function updates the CRC
 * @param  None
 * @retval None
 */
 static uint16_t M24SR_UpdateCrc(uint8_t ch, uint16_t *lpwCrc) {
  ch = (ch ^ (uint8_t) ((*lpwCrc) & 0x00FF));
  ch = (ch ^ (ch << 4));
  *lpwCrc = (*lpwCrc >> 8) ^ ((uint16_t) ch << 8) ^ ((uint16_t) ch << 3)
      ^ ((uint16_t) ch >> 4);

  return (*lpwCrc);
}

/**
 * @brief  This function returns the CRC 16
 * @param  Data : pointer on the data used to compute the CRC16
 * @param  Length : number of bytes of the data
 * @retval CRC16
 */
 static uint16_t M24SR_ComputeCrc(uint8_t *Data, uint8_t Length) {
  uint8_t chBlock;
  uint16_t wCrc = 0x6363; // ITU-V.41

  do {
    chBlock = *Data++;
    M24SR_UpdateCrc(chBlock, &wCrc);
  } while (--Length);

  return wCrc;
}

/**
 * @brief    This function computes the CRC16 residue as defined by CRC ISO/IEC 13239
 * @param    DataIn    :  input data
 * @param  Length     :   Number of bits of DataIn
 * @retval   Status (SW1&SW2)    :   CRC16 residue is correct
 * @retval   M24SR_ERROR_CRC    :  CRC16 residue is false
 */
 static M24SR::StatusTypeDef M24SR_IsCorrectCRC16Residue(uint8_t *DataIn, uint8_t Length) {
  uint16_t ResCRC = 0x0000;
  M24SR::StatusTypeDef status;
  /* check the CRC16 Residue */
  if (Length != 0)
    ResCRC = M24SR_ComputeCrc(DataIn, Length);

  if (ResCRC == 0x0000) {
    /* Good CRC, but error status from M24SR */
    status = (M24SR::StatusTypeDef) (((DataIn[Length - UB_STATUS_OFFSET] << 8)
        & 0xFF00) | (DataIn[Length - LB_STATUS_OFFSET] & 0x00FF));
  } else {
    ResCRC = 0x0000;
    ResCRC = M24SR_ComputeCrc(DataIn, 5);
    if (ResCRC != 0x0000) {
      /* Bad CRC */
      return M24SR::M24SR_IO_ERROR_CRC;
    } else {
      /* Good CRC, but error status from M24SR */
      status= (M24SR::StatusTypeDef) (((DataIn[1] << 8) & 0xFF00)
          | (DataIn[2] & 0x00FF));
    }
  }
  if(status==NFC_COMMAND_SUCCESS) status = M24SR::M24SR_SUCCESS;
  return status;
}

/**
 * @brief     This functions creates an I block command according to the structures CommandStructure and Command.
 * @param     Command : structure which contains the field of the different parameters
 * @param     CommandStructure : structure of the command
 * @param     NbByte : number of bytes of the command
 * @param     pCommand : pointer to the command created
 */
 static void M24SR_BuildIBlockCommand(uint16_t CommandStructure, C_APDU *Command,
    uint8_t uDIDbyte, uint16_t *NbByte, uint8_t *pCommand) {
  uint16_t uCRC16;
  static uint8_t BlockNumber = 0x01;

  (*NbByte) = 0;

  /* add the PCD byte */
  if ((CommandStructure & M24SR_PCB_NEEDED) != 0) {
    /* toggle the block number */
    BlockNumber = TOGGLE(BlockNumber);
    /* Add the I block byte */
    pCommand[(*NbByte)++] = 0x02 | BlockNumber;
  }

  /* add the DID byte */
  if ((BlockNumber & M24SR_DID_NEEDED) != 0) {
    /* Add the I block byte */
    pCommand[(*NbByte)++] = uDIDbyte;
  }

  /* add the Class byte */
  if ((CommandStructure & M24SR_CLA_NEEDED) != 0) {
    pCommand[(*NbByte)++] = Command->Header.CLA;
  }
  /* add the instruction byte byte */
  if ((CommandStructure & M24SR_INS_NEEDED) != 0) {
    pCommand[(*NbByte)++] = Command->Header.INS;
  }
  /* add the Selection Mode byte */
  if ((CommandStructure & M24SR_P1_NEEDED) != 0) {
    pCommand[(*NbByte)++] = Command->Header.P1;
  }
  /* add the Selection Mode byte */
  if ((CommandStructure & M24SR_P2_NEEDED) != 0) {
    pCommand[(*NbByte)++] = Command->Header.P2;
  }
  /* add Data field lengthbyte */
  if ((CommandStructure & M24SR_LC_NEEDED) != 0) {
    pCommand[(*NbByte)++] = Command->Body.LC;
  }
  /* add Data field  */
  if ((CommandStructure & M24SR_DATA_NEEDED) != 0) {
    memcpy(&(pCommand[(*NbByte)]), Command->Body.pData, Command->Body.LC);
    (*NbByte) += Command->Body.LC;
  }
  /* add Le field  */
  if ((CommandStructure & M24SR_LE_NEEDED) != 0) {
    pCommand[(*NbByte)++] = Command->Body.LE;
  }
  /* add CRC field  */
  if ((CommandStructure & M24SR_CRC_NEEDED) != 0) {
    uCRC16 = M24SR_ComputeCrc(pCommand, (uint8_t) (*NbByte));
    /* append the CRC16 */
    pCommand[(*NbByte)++] = GETLSB(uCRC16);
    pCommand[(*NbByte)++] = GETMSB(uCRC16);
  }

}




/**
 * @brief    This function returns M24SR_STATUS_SUCCESS if the pBuffer is an s-block
 * @param    pBuffer    :  pointer to the data
 * @retval   M24SR_SUCCESS  :  the data is a S-Block
 * @retval   NFC_ERROR    :  the data is not a S-Block
 */
 static M24SR::StatusTypeDef IsSBlock(uint8_t *pBuffer) {

  if ((pBuffer[M24SR_OFFSET_PCB] & M24SR_MASK_BLOCK) == M24SR_MASK_SBLOCK) {
    return M24SR::M24SR_SUCCESS;
  } else {
    return M24SR::M24SR_ERROR;
  }

}


M24SR::M24SR(const uint8_t address, TwoWire *I2C,gpoEventCallback eventCallback, const int GPOPinName,
    const int RFDISPinName) :
    who_am_i(0),
    type(0),
    address(address),
    dev_I2C(I2C),
    GPOPin(GPOPinName),
    RFDisablePin(RFDISPinName),
    mCommunicationType(SYNC),
    mLastCommandSend(NONE),
    mCallback(&defaultCallback),
    mComponentCallback(NULL),
    mNDefTagUtil(new NDefNfcTagM24SR(*this)),
    mManageGPOCallback(*this),
    mReadIDCallback(*this){
    //mNDefTagUtil(NULL){
  memset(uM24SRbuffer, 0, 0xFF * sizeof(int8_t));
  uDIDbyte = 0;
    pinMode(RFDisablePin, OUTPUT);
    digitalWrite(RFDisablePin, 0);
    pinMode(GPOPin, INPUT_PULLUP);
    if(eventCallback!=NULL) {
      attachInterrupt(GPOPin, eventCallback, FALLING);
    }
}

M24SR::~M24SR(){
  delete mNDefTagUtil;
}

/**
 * @brief  This function sends the FWT extension command (S-Block format)
 * @param  FWTbyte : FWT value
 * @return M24SR_SUCCESS if no errors
 */
 M24SR::StatusTypeDef M24SR::M24SR_SendFWTExtension(uint8_t FWTbyte) {
  uint8_t pBuffer[M24SR_STATUSRESPONSE_NBBYTE];
  M24SR::StatusTypeDef status;
  uint8_t NthByte = 0;
  uint16_t uCRC16;

  /* create the response */
  pBuffer[NthByte++] = 0xF2;
  pBuffer[NthByte++] = FWTbyte;
  /* compute the CRC */
  uCRC16 = M24SR_ComputeCrc(pBuffer, 0x02);
  /* append the CRC16 */
  pBuffer[NthByte++] = GETLSB(uCRC16);
  pBuffer[NthByte++] = GETMSB(uCRC16);

  /* send the request */
  status = M24SR_IO_SendI2Ccommand(NthByte, pBuffer);
  if(status != M24SR_SUCCESS)
    return status;

  mLastCommandSend=UPDATE;

  if(mCommunicationType==M24SR::SYNC){
    return M24SR_ReceiveUpdateBinary();
  }//if

  return M24SR_SUCCESS;
}

/**
 * @brief  This function initialize the M24SR device
 * @retval None
 */
 M24SR::StatusTypeDef M24SR::M24SR_Init(M24SR_InitTypeDef *notUsed) {
  (void) notUsed;
  //force to open a i2c session
  StatusTypeDef status = M24SR_ForceSession();
  if(status!= M24SR_SUCCESS)
    return status;
  //leave the gpo always up

    status = M24SR_ManageI2CGPO(DEFAULT_GPO_STATUS);
  if(status!= M24SR_SUCCESS)
    return status;

  status = M24SR_ManageRFGPO(DEFAULT_GPO_STATUS);
  if(status!= M24SR_SUCCESS)
    return status;

  //close the session
  status = M24SR_Deselect();
  if(status!= M24SR_SUCCESS)
    return status;

  return M24SR_SUCCESS;
}


/**
 * @brief  This function sends the KillSession command to the M24SR device
 * @param  None
 * @return M24SR_SUCCESS if no errors
 */
 M24SR::StatusTypeDef M24SR::M24SR_ForceSession(void) {
  uint8_t commandBuffer[] = M24SR_KILLSESSION_COMMAND;
  M24SR::StatusTypeDef status;
  status = M24SR_IO_SendI2Ccommand(sizeof(commandBuffer), commandBuffer);
  if(status!=M24SR_SUCCESS){
    mCallback->on_session_open(this,status);
    return status;
  }

  /* Insure no access will be done just after open session */
  /* The only way here is to poll I2C to know when M24SR is ready */
  /* GPO can not be use with KillSession command */
  //status = M24SR_IO_PollI2C();

  getCallback()->on_session_open(this,status);
  return status;

}

/**
 * @brief  This function sends the Deselect command (S-Block format)
 * @return M24SR_SUCCESS if no errors
 */
 M24SR::StatusTypeDef M24SR::M24SR_Deselect(void) {
  uint8_t pBuffer[] = M24SR_DESELECTREQUEST_COMMAND;
  M24SR::StatusTypeDef status;
  /* send the request */
  status = M24SR_IO_SendI2Ccommand(sizeof(pBuffer), pBuffer);
  if(status!=M24SR_SUCCESS){
    getCallback()->on_deselect(this,status);
  }

  mLastCommandSend=DESELECT;

  if(mCommunicationType==M24SR::SYNC){
    return M24SR_ReceiveDeselect();
  }//if

  return M24SR_SUCCESS;
}

M24SR::StatusTypeDef M24SR::M24SR_ReceiveDeselect(void){
  uint8_t pBuffer[4];
  M24SR::StatusTypeDef status;

  mLastCommandSend=NONE;

  status = M24SR_IO_ReceiveI2Cresponse(sizeof(pBuffer), pBuffer);
  getCallback()->on_deselect(this,status);
  return status;
}



/**
 * @brief  This function sends the GetSession command to the M24SR device
 * @retval M24SR_SUCCESS the function is successful.
 * @retval Status (SW1&SW2) if operation does not complete.
 */
 M24SR::StatusTypeDef M24SR::M24SR_GetSession(void) {
  uint8_t commandBuffer[] = M24SR_OPENSESSION_COMMAND;

  M24SR::StatusTypeDef status;
  status = M24SR_IO_SendI2Ccommand(sizeof(commandBuffer), commandBuffer);
  if(status!=M24SR_SUCCESS){
    getCallback()->on_session_open(this,status);
    return status;
  }

  /* Insure no access will be done just after open session */
  /* The only way here is to poll I2C to know when M24SR is ready */
  /* GPO can not be use with KillSession command */
  //status = M24SR_IO_PollI2C();

  getCallback()->on_session_open(this,status);
  return status;
}

/**
 * @brief  This function sends the SelectApplication command
 * @return M24SR_SUCCESS if no errors
 */
 M24SR::StatusTypeDef M24SR::M24SR_SendSelectApplication(void) {

  C_APDU command;
  M24SR::StatusTypeDef status;
  uint8_t *pBuffer = uM24SRbuffer;
  uint8_t pDataOut[] = M24SR_SELECTAPPLICATION_COMMAND;
  uint8_t uLe = 0x00;
  uint16_t uP1P2 =0x0400,  NbByte;

  /* build the command */
  command.Header.CLA = C_APDU_CLA_DEFAULT;
  command.Header.INS = C_APDU_SELECT_FILE;
  /* copy the offset */
  command.Header.P1 = GETMSB(uP1P2);
  command.Header.P2 = GETLSB(uP1P2);
  /* copy the number of byte of the data field */
  command.Body.LC = sizeof(pDataOut);
  /* copy the data */
  command.Body.pData = pDataOut;
  /* copy the number of byte to read */
  command.Body.LE = uLe;
  /* build the I2C command */
  M24SR_BuildIBlockCommand( M24SR_CMDSTRUCT_SELECTAPPLICATION, &command,
      uDIDbyte, &NbByte, pBuffer);

  /* send the request */
  status = M24SR_IO_SendI2Ccommand(NbByte, pBuffer);
  if(status != M24SR_SUCCESS){
    getCallback()->on_selected_application(this,status);
    return status;
  }

  mLastCommandSend=SELECT_APPLICATION;

  if(mCommunicationType==M24SR::SYNC){
    return M24SR_ReceiveSelectApplication();
  }//if

  return M24SR_SUCCESS;
}

M24SR::StatusTypeDef M24SR::M24SR_ReceiveSelectApplication(void) {

  uint8_t pDataIn[M24SR_STATUSRESPONSE_NBBYTE];
  M24SR::StatusTypeDef status;

  mLastCommandSend = NONE;

  status = M24SR_IO_ReceiveI2Cresponse(sizeof(pDataIn), pDataIn);
  if(status!=M24SR_SUCCESS){
    getCallback()->on_selected_application(this,status);
    return status;
  }//else
  status= M24SR_IsCorrectCRC16Residue(pDataIn, sizeof(pDataIn));
  getCallback()->on_selected_application(this,status);
  return status;

}



M24SR::StatusTypeDef M24SR::M24SR_ReadID(uint8_t *nfc_id) {
  if (!nfc_id) {
    return M24SR_ERROR;
  }

  //enable the callback for change the gpo
  mComponentCallback = &mReadIDCallback;
  mReadIDCallback.read_id_on(nfc_id);

  //start the readID procedure
  return M24SR_SendSelectApplication();
}

/**
 * @brief  This function sends the SelectCCFile command
 * @retval M24SR_SUCCESS the function is successful.
 * @retval M24SR_ERROR_I2CTIMEOUT I2C timeout occurred.
 * @retval Status (SW1&SW2)   if operation does not complete for another reason.
 */
 M24SR::StatusTypeDef M24SR::M24SR_SendSelectCCfile(void) {
  C_APDU command;
  M24SR::StatusTypeDef status;
  uint8_t *pBuffer = uM24SRbuffer;
  uint8_t pDataOut[] = CC_FILE_ID_BYTES;
  uint16_t uP1P2 =0x000C,
  NbByte;

  /* build the command */
  command.Header.CLA = C_APDU_CLA_DEFAULT;
  command.Header.INS = C_APDU_SELECT_FILE;
  /* copy the offset */
  command.Header.P1 = GETMSB(uP1P2);
  command.Header.P2 = GETLSB(uP1P2);
  /* copy the number of byte of the data field */
  command.Body.LC = sizeof(pDataOut);
  command.Body.pData = pDataOut;
  /* build the I2C command */
  M24SR_BuildIBlockCommand(M24SR_CMDSTRUCT_SELECTCCFILE, &command, uDIDbyte,
      &NbByte, pBuffer);

  /* send the request */
  status = M24SR_IO_SendI2Ccommand(NbByte, pBuffer);
  if(status!=M24SR_SUCCESS){
    getCallback()->on_selected_CC_file(this,status);
    return status;
  }//else


  mLastCommandSend=SELECT_CC_FILE;

  if(mCommunicationType==M24SR::SYNC){
    return M24SR_ReceiveSelectCCfile();
  }//if

  return M24SR_SUCCESS;
}

M24SR::StatusTypeDef M24SR::M24SR_ReceiveSelectCCfile(void){

  uint8_t pDataIn[M24SR_STATUSRESPONSE_NBBYTE];
  M24SR::StatusTypeDef status;

  mLastCommandSend = NONE;

  status = M24SR_IO_ReceiveI2Cresponse(sizeof(pDataIn), pDataIn);
  if(status!=M24SR_SUCCESS){
    getCallback()->on_selected_CC_file(this,status);
    return status;
  }//else
  status= M24SR_IsCorrectCRC16Residue(pDataIn, sizeof(pDataIn));
  getCallback()->on_selected_CC_file(this,status);
  return status;

}

/**
 * @brief  This function sends the SelectSystemFile command
 * @retval Status (SW1&SW2) Status of the operation to complete.
 * @retval M24SR_ERROR_I2CTIMEOUT I2C timeout occurred.
 */
 M24SR::StatusTypeDef M24SR::M24SR_SendSelectSystemfile(void) {
  C_APDU command;

  uint8_t *pBuffer = uM24SRbuffer;
  uint8_t pDataOut[] = SYSTEM_FILE_ID_BYTES;
  M24SR::StatusTypeDef status;
  uint16_t uP1P2 =0x000C,  NbByte;

  /* build the command */
  command.Header.CLA = C_APDU_CLA_DEFAULT;
  command.Header.INS = C_APDU_SELECT_FILE;
  /* copy the offset */
  command.Header.P1 = GETMSB(uP1P2);
  command.Header.P2 = GETLSB(uP1P2);
  /* copy the number of byte of the data field */
  command.Body.LC = sizeof(pDataOut);
  command.Body.pData = pDataOut;
  /* build the I2C command */
  M24SR_BuildIBlockCommand(M24SR_CMDSTRUCT_SELECTCCFILE, &command, uDIDbyte,
      &NbByte, pBuffer);

  /* send the request */
  status = M24SR_IO_SendI2Ccommand(NbByte, pBuffer);
  if(status!=M24SR_SUCCESS){
    getCallback()->on_selected_system_file(this,status);
    return status;
  }//else

  mLastCommandSend=SELECT_SYSTEM_FILE;

  if(mCommunicationType==M24SR::SYNC){
    return M24SR_ReceiveSelectSystemfile();
  }//if

  return M24SR_SUCCESS;
}

M24SR::StatusTypeDef M24SR::M24SR_ReceiveSelectSystemfile(){

  uint8_t pDataIn[M24SR_STATUSRESPONSE_NBBYTE];
  M24SR::StatusTypeDef status;

  mLastCommandSend = NONE;

  status = M24SR_IO_ReceiveI2Cresponse(sizeof(pDataIn), pDataIn);
  if(status!=M24SR_SUCCESS){
    getCallback()->on_selected_system_file(this,status);
    return status;
  }//else
  status= M24SR_IsCorrectCRC16Residue(pDataIn, sizeof(pDataIn));
  getCallback()->on_selected_system_file(this,status);
  return status;

}

/**
 * @brief  This function sends the SelectNDEFfile command
 * @retval Status (SW1&SW2) Status of the operation to complete.
 * @retval M24SR_ERROR_I2CTIMEOUT I2C timeout occurred.
 */
 M24SR::StatusTypeDef M24SR::M24SR_SendSelectNDEFfile(uint16_t NDEFfileId) {
  C_APDU command;
  M24SR::StatusTypeDef status;
  uint8_t *pBuffer = uM24SRbuffer;
  uint8_t pDataOut[] = { GETMSB(NDEFfileId), GETLSB(NDEFfileId) };
  uint16_t uP1P2 = 0x000C, NbByte;

  /* build the command */
  command.Header.CLA = C_APDU_CLA_DEFAULT;
  command.Header.INS = C_APDU_SELECT_FILE;
  /* copy the offset */
  command.Header.P1 = GETMSB(uP1P2);
  command.Header.P2 = GETLSB(uP1P2);
  /* copy the number of byte of the data field */
  command.Body.LC = sizeof(pDataOut);
  command.Body.pData = pDataOut;
  /* copy the offset */
  /* build the I2C command */
  M24SR_BuildIBlockCommand(M24SR_CMDSTRUCT_SELECTNDEFFILE, &command, uDIDbyte,
      &NbByte, pBuffer);

  /* send the request */
  status = M24SR_IO_SendI2Ccommand(NbByte, pBuffer);
  if(status!=M24SR_SUCCESS){

  }

  mLastCommandSend=SELECT_NDEF_FILE;

  if(mCommunicationType==M24SR::SYNC){
    return M24SR_ReceiveSelectNDEFfile();
  }

  return M24SR_SUCCESS;

}

M24SR::StatusTypeDef M24SR::M24SR_ReceiveSelectNDEFfile(){

  uint8_t pDataIn[M24SR_STATUSRESPONSE_NBBYTE];
  M24SR::StatusTypeDef status;

  mLastCommandSend = NONE;

  status = M24SR_IO_ReceiveI2Cresponse(sizeof(pDataIn), pDataIn);
  if(status!=M24SR_SUCCESS){
    getCallback()->on_selected_NDEF_file(this,status);
    return status;
  }//else
  status= M24SR_IsCorrectCRC16Residue(pDataIn, sizeof(pDataIn));
  getCallback()->on_selected_NDEF_file(this,status);
  return status;

}

/**
 * @brief  This function sends a read binary command
 * @param  Offset   first byte to read
 * @param  NbByteToRead   number of bytes to read
 * @param  pBufferRead   pointer to the buffer read from the M24SR device
 * @retval Status (SW1&SW2) Status of the operation to complete.
 * @retval M24SR_ERROR_I2CTIMEOUT I2C timeout occurred.
 */
 M24SR::StatusTypeDef M24SR::M24SR_SendReadBinary(uint16_t Offset, uint8_t NbByteToRead,
    uint8_t *pBufferRead) {
  //clamp the buffer to the max size
  if(NbByteToRead>M24SR_MAX_BYTE_OPERATION_LENGHT)
    NbByteToRead=M24SR_MAX_BYTE_OPERATION_LENGHT;

  C_APDU command;
  uint8_t *pBuffer = uM24SRbuffer;
  uint16_t NbByte;
  M24SR::StatusTypeDef status;

  /* build the command */
  command.Header.CLA = C_APDU_CLA_DEFAULT;
  command.Header.INS = C_APDU_READ_BINARY;
  /* copy the offset */
  command.Header.P1 = GETMSB(Offset);
  command.Header.P2 = GETLSB(Offset);
  /* copy the number of byte to read */
  command.Body.LE = NbByteToRead;

  M24SR_BuildIBlockCommand(M24SR_CMDSTRUCT_READBINARY, &command, uDIDbyte,
      &NbByte, pBuffer);

  status = M24SR_IO_SendI2Ccommand(NbByte, pBuffer);
  if(status!=M24SR_SUCCESS){
    getCallback()->on_read_byte(this,status,Offset,pBufferRead,NbByteToRead);
    return status;
  }

  mLastCommandSend=READ;
  mLastCommandData.data=pBufferRead;
  mLastCommandData.length=NbByteToRead;
  mLastCommandData.offset=Offset;

  if(mCommunicationType==M24SR::SYNC){
    return M24SR_ReceiveReadBinary();
  }//if

  return M24SR_SUCCESS;
}

M24SR::StatusTypeDef M24SR::M24SR_ReceiveReadBinary(){

  M24SR::StatusTypeDef status;
  const uint16_t length = mLastCommandData.length;
  const uint16_t offset = mLastCommandData.offset;
  uint8_t *data = mLastCommandData.data;

  mLastCommandSend=NONE;

  status = M24SR_IO_ReceiveI2Cresponse (length + M24SR_STATUSRESPONSE_NBBYTE , uM24SRbuffer );
  if(status!=M24SR_SUCCESS){
    getCallback()->on_read_byte(this,status,offset,data,length);
    return status;
  }
  status = M24SR_IsCorrectCRC16Residue(uM24SRbuffer, length + M24SR_STATUSRESPONSE_NBBYTE);
  if(status != M24SR_SUCCESS)
    getCallback()->on_read_byte(this,status,offset,data,length);
  else{
    /* retrieve the data without SW1 & SW2 as provided as return value of the function */
    memcpy(mLastCommandData.data, &uM24SRbuffer[1], length);
    getCallback()->on_read_byte(this,status,offset,data,length);
  }
  return status;

}

/**
 * @brief  This function sends a ST read binary command (no error if access is not inside NDEF file)
 * @param  Offset   first byte to read
 * @param  NbByteToRead number of bytes to read
 * @param  pBufferRead  pointer to the buffer read from the M24SR device
 * @retval Status (SW1&SW2) Status of the operation to complete.
 * @retval M24SR_ERROR_I2CTIMEOUT I2C timeout occurred.
 */
 M24SR::StatusTypeDef M24SR::M24SR_SendSTReadBinary(uint16_t Offset,
    uint8_t NbByteToRead, uint8_t *pBufferRead) {
  //clamp the buffer to the max size
  if(NbByteToRead>M24SR_MAX_BYTE_OPERATION_LENGHT)
    NbByteToRead=M24SR_MAX_BYTE_OPERATION_LENGHT;

  C_APDU command;
  uint8_t *pBuffer = uM24SRbuffer;
  uint16_t NbByte;
  M24SR::StatusTypeDef status;

  /* build the command */
  command.Header.CLA = C_APDU_CLA_ST;
  command.Header.INS = C_APDU_READ_BINARY;
  /* copy the offset */
  command.Header.P1 = GETMSB(Offset);
  command.Header.P2 = GETLSB(Offset);
  /* copy the number of byte to read */
  command.Body.LE = NbByteToRead;

  M24SR_BuildIBlockCommand(M24SR_CMDSTRUCT_READBINARY, &command, uDIDbyte,
      &NbByte, pBuffer);

  status = M24SR_IO_SendI2Ccommand(NbByte, pBuffer);
  if(status!=M24SR_SUCCESS){
    getCallback()->on_read_byte(this,status,Offset,pBufferRead,NbByteToRead);
    return status;
  }

  mLastCommandSend=READ;
  mLastCommandData.data=pBufferRead;
  mLastCommandData.length=NbByteToRead;

  if(mCommunicationType==M24SR::SYNC){
    return M24SR_ReceiveReadBinary();
  }//if

  return M24SR_SUCCESS;
}

/**
 * @brief  This function sends a Update binary command
 * @param  Offset   first byte to read
 * @param  NbByteToWrite   number of bytes to write
 * @param  pBufferRead   pointer to the buffer read from the M24SR device
 * @retval Status (SW1&SW2) Status of the operation to complete.
 * @retval M24SR_ERROR_I2CTIMEOUT I2C timeout occurred.
 */
 M24SR::StatusTypeDef M24SR::M24SR_SendUpdateBinary(uint16_t Offset,
    uint8_t NbByteToWrite, uint8_t *pDataToWrite) {
  //clamp the buffer to the max size
  if(NbByteToWrite>M24SR_MAX_BYTE_OPERATION_LENGHT)
    NbByteToWrite=M24SR_MAX_BYTE_OPERATION_LENGHT;

  C_APDU command;
  M24SR::StatusTypeDef status;
  uint8_t *pBuffer = uM24SRbuffer;
  uint16_t NbByte;

  /* build the command */
  command.Header.CLA = C_APDU_CLA_DEFAULT;
  command.Header.INS = C_APDU_UPDATE_BINARY;
  /* copy the offset */
  command.Header.P1 = GETMSB(Offset);
  command.Header.P2 = GETLSB(Offset);
  /* copy the number of byte of the data field */
  command.Body.LC = NbByteToWrite;
  command.Body.pData = pDataToWrite;
  /* copy the File Id */
  //memcpy(command.Body.pData ,pDataToWrite, NbByteToWrite );
  M24SR_BuildIBlockCommand(M24SR_CMDSTRUCT_UPDATEBINARY, &command, uDIDbyte,
      &NbByte, pBuffer);

  status = M24SR_IO_SendI2Ccommand(NbByte, pBuffer);
  if(status!=M24SR_SUCCESS){
    getCallback()->on_updated_binary(this,status,Offset,pDataToWrite,NbByteToWrite);
    return status;
  }

  mLastCommandSend=UPDATE;
  mLastCommandData.data=pDataToWrite;
  mLastCommandData.length=NbByteToWrite;
  mLastCommandData.offset=Offset;

  if(mCommunicationType==M24SR::SYNC){
    return M24SR_ReceiveUpdateBinary();
  }

  return M24SR_SUCCESS;
}

M24SR::StatusTypeDef M24SR::M24SR_ReceiveUpdateBinary() {

  uint8_t respBuffer[M24SR_STATUSRESPONSE_NBBYTE];
  M24SR::StatusTypeDef status;
  const uint16_t length = mLastCommandData.length;
  uint8_t *data = mLastCommandData.data;
  const uint16_t offset = mLastCommandData.offset;

  mLastCommandSend=NONE;

  status= M24SR_IO_ReceiveI2Cresponse (sizeof(respBuffer) , respBuffer);
  if(status != M24SR_SUCCESS){
    getCallback()->on_updated_binary(this,status,offset,data,length);
    return status;
  }

  if (IsSBlock(respBuffer) == M24SR_SUCCESS) {
    /*check the CRC */
    status =M24SR_IsCorrectCRC16Residue(respBuffer,
          M24SR_WATINGTIMEEXTRESPONSE_NBBYTE);
    // TODO: why if we check ==NFC_Commandsuccess it fail?
    if (status != M24SR_IO_ERROR_CRC) {
      /* send the FrameExension response*/
      status = M24SR_SendFWTExtension(respBuffer[M24SR_OFFSET_PCB + 1]);
      if(status!=M24SR_SUCCESS){ //something get wrong -> abort the update
        getCallback()->on_updated_binary(this,status,offset,data,length);
      }//status
    }//if
  } else { //isSBlock
    status = M24SR_IsCorrectCRC16Residue(respBuffer,
        M24SR_STATUSRESPONSE_NBBYTE);
    getCallback()->on_updated_binary(this,status,offset,data,length);
  }//if else
  return status;
}//M24SR_ReceiveUpdateBinary


/**
 * @brief  This function sends the Verify command
 * @param  uPwdId   PasswordId ( 0x0001 : Read NDEF pwd or 0x0002 : Write NDEF pwd or 0x0003 : I2C pwd)
 * @param  NbPwdByte   Number of bytes ( 0x00 or 0x10)
 * @param  pPwd   pointer to the password
 * @retval Status (SW1&SW2) Status of the operation to complete.
 * @retval M24SR_ERROR_I2CTIMEOUT I2C timeout occurred.
 */
 M24SR::StatusTypeDef M24SR::M24SR_SendVerify(uint16_t uPwdId, uint8_t NbPwdByte,
    const uint8_t *pPwd) {
  C_APDU command;
  M24SR::StatusTypeDef status;
  uint8_t *pBuffer = uM24SRbuffer;
  uint16_t NbByte;

  /*check the parameters */
  if ((uPwdId > 0x0003)|| ((NbPwdByte != 0x00) && (NbPwdByte != 0x10))) {
    getCallback()->on_verified(this,M24SR_IO_ERROR_PARAMETER,constToPasswordType(uPwdId),pPwd);
    return M24SR_IO_ERROR_PARAMETER;
  }

  /* build the command */
  command.Header.CLA = C_APDU_CLA_DEFAULT;
  command.Header.INS = C_APDU_VERIFY;
  /* copy the Password Id */
  command.Header.P1 = GETMSB(uPwdId);
  command.Header.P2 = GETLSB(uPwdId);
  /* copy the number of bytes of the data field */
  command.Body.LC = NbPwdByte;

  if (NbPwdByte == 0x10) {
    /* copy the password */
    command.Body.pData = pPwd;
    /* build the I2C command */
    M24SR_BuildIBlockCommand(M24SR_CMDSTRUCT_VERIFYBINARYWITHPWD, &command,
        uDIDbyte, &NbByte, pBuffer);
  } else {
    command.Body.pData = NULL;
    /* build the I2C command */
    M24SR_BuildIBlockCommand(M24SR_CMDSTRUCT_VERIFYBINARYWOPWD, &command,
        uDIDbyte, &NbByte, pBuffer);
  }

  /* send the request */
  status = M24SR_IO_SendI2Ccommand(NbByte, pBuffer);
  if(status!=M24SR_SUCCESS){
    getCallback()->on_verified(this,status,constToPasswordType(uPwdId),pPwd);
    return status;
  }
  mLastCommandSend=VERIFY;

  mLastCommandData.data=(uint8_t*)pPwd;
  mLastCommandData.offset=uPwdId;

  if(mCommunicationType==M24SR::SYNC){
    return M24SR_ReceiveVerify();
  }

  return M24SR_SUCCESS;
}

M24SR::StatusTypeDef M24SR::M24SR_ReceiveVerify(){
  M24SR::StatusTypeDef status;
  uint8_t respBuffer[M24SR_STATUSRESPONSE_NBBYTE];
  mLastCommandSend=NONE;

  const uint8_t *data = mLastCommandData.data;
  const PasswordType_t type = constToPasswordType(mLastCommandData.offset);

  status=M24SR_IO_ReceiveI2Cresponse (sizeof(respBuffer),respBuffer);

  if(status !=M24SR_SUCCESS){
    getCallback()->on_verified(this,status,type,data);
    return status;
  }
  status = M24SR_IsCorrectCRC16Residue(respBuffer, M24SR_STATUSRESPONSE_NBBYTE);
  getCallback()->on_verified(this,status,type,data);
  return status;
}

/**
 * @brief  This function sends the ChangeReferenceData command
 * @param  uPwdId   PasswordId ( 0x0001 : Read NDEF pwd or 0x0002 : Write NDEF pwd or 0x0003 : I2C pwd)
 * @param  pPwd   pointer to the passwaord
 * @retval Status (SW1&SW2) Satus of the operation to complete.
 * @retval M24SR_ERROR_I2CTIMEOUT I2C timeout occurred.
 */
 M24SR::StatusTypeDef M24SR::M24SR_SendChangeReferenceData(uint16_t uPwdId,
    uint8_t *pPwd) {
  C_APDU command;
  M24SR::StatusTypeDef status;
  uint8_t *pBuffer = uM24SRbuffer;
  uint16_t NbByte;

  /*check the parameters */
  if (uPwdId > 0x0003) {
    getCallback()->on_change_reference_data(this,M24SR_IO_ERROR_PARAMETER,
        constToPasswordType(uPwdId), pPwd);
    return M24SR_IO_ERROR_PARAMETER;
  }

  /* build the command */
  command.Header.CLA = C_APDU_CLA_DEFAULT;
  command.Header.INS = C_APDU_CHANGE;
  /* copy the Password Id */
  command.Header.P1 = GETMSB(uPwdId);
  command.Header.P2 = GETLSB(uPwdId);
  /* copy the number of byte of the data field */
  command.Body.LC = M24SR_PASSWORD_NBBYTE;
  /* copy the password */
  command.Body.pData = pPwd;
  /* build the IÃƒâ€šÃ‚Â²C command */
  M24SR_BuildIBlockCommand(M24SR_CMDSTRUCT_CHANGEREFDATA, &command, uDIDbyte,
      &NbByte, pBuffer);

  /* send the request */
  status = M24SR_IO_SendI2Ccommand(NbByte, pBuffer);
  if(status != M24SR_SUCCESS){
    getCallback()->on_change_reference_data(this,status,
              constToPasswordType(uPwdId), pPwd);
    return status;
  }

  mLastCommandSend = CHANGE_REFERENCE_DATA;
  mLastCommandData.data = pPwd;
  //use the offset filed for store the pwd id;
  mLastCommandData.offset = (uint8_t)uPwdId;

  if(mCommunicationType==M24SR::SYNC){
    return M24SR_ReceiveChangeReferenceData();
  }//if

  return M24SR_SUCCESS;
}

M24SR::StatusTypeDef M24SR::M24SR_ReceiveChangeReferenceData(){
  M24SR::StatusTypeDef status;
  //TODO the size is 3?
  uint8_t rensponseBuffer[M24SR_STATUSRESPONSE_NBBYTE];

  PasswordType_t type = constToPasswordType(mLastCommandData.offset);
  uint8_t *data = mLastCommandData.data;

  status =M24SR_IO_ReceiveI2Cresponse (M24SR_STATUSRESPONSE_NBBYTE , rensponseBuffer );
  if(status!=M24SR_SUCCESS){
    getCallback()->on_change_reference_data(this,status,type,data);
    return status;
  }//else

  status = M24SR_IsCorrectCRC16Residue(rensponseBuffer, M24SR_STATUSRESPONSE_NBBYTE);
  getCallback()->on_change_reference_data(this,status,type,data);
  return status;
}

/**
 * @brief  This function sends the EnableVerificationRequirement command
 * @param  uReadOrWrite enable the read or write protection ( 0x0001 : Read or 0x0002 : Write  )
 * @retval Status (SW1&SW2) Status of the operation to complete.
 * @retval M24SR_ERROR_I2CTIMEOUT I2C timeout occurred.
 */
 M24SR::StatusTypeDef M24SR::M24SR_SendEnableVerificationRequirement(
    uint16_t uReadOrWrite) {
  C_APDU command;
  M24SR::StatusTypeDef status;
  uint8_t *pBuffer = uM24SRbuffer;
  uint16_t NbByte;

  /*check the parameters */
  if ((uReadOrWrite != 0x0001) && (uReadOrWrite != 0x0002)) {
    getCallback()->on_enable_verification_requirement(this,M24SR_IO_ERROR_PARAMETER,
        constToPasswordType(uReadOrWrite));
    return M24SR_IO_ERROR_PARAMETER;
  }

  /* build the command */
  command.Header.CLA = C_APDU_CLA_DEFAULT;
  command.Header.INS = C_APDU_ENABLE;
  /* copy the Password Id */
  command.Header.P1 = GETMSB(uReadOrWrite);
  command.Header.P2 = GETLSB(uReadOrWrite);
  /* build the I2C command */
  M24SR_BuildIBlockCommand( M24SR_CMDSTRUCT_ENABLEVERIFREQ, &command,
      uDIDbyte, &NbByte, pBuffer);

  /* send the request */
  status = M24SR_IO_SendI2Ccommand(NbByte, pBuffer);
  if(status != M24SR_SUCCESS){
    getCallback()->on_enable_verification_requirement(this,status,
        constToPasswordType(uReadOrWrite));
    return status;
  }//if

  mLastCommandSend = ENABLE_VERIFICATION_REQUIREMENT;
  //use the offset filed for store the pwd id;
  mLastCommandData.offset = (uint8_t)uReadOrWrite;

  if(mCommunicationType==M24SR::SYNC){
    return M24SR_ReceiveEnableVerificationRequirement();
  }//if

  return M24SR_SUCCESS;
}

M24SR::StatusTypeDef M24SR::M24SR_ReceiveEnableVerificationRequirement(){
  M24SR::StatusTypeDef status;
  //TODO the size is 3?
  uint8_t rensponseBuffer[M24SR_STATUSRESPONSE_NBBYTE];

  const PasswordType_t type = constToPasswordType(mLastCommandData.offset);

  status =M24SR_IO_ReceiveI2Cresponse (M24SR_STATUSRESPONSE_NBBYTE , rensponseBuffer );
  if(status!=M24SR_SUCCESS){
    getCallback()->on_enable_verification_requirement(this,status,type);
    return status;
  }//else

  status = M24SR_IsCorrectCRC16Residue(rensponseBuffer, M24SR_STATUSRESPONSE_NBBYTE);
  getCallback()->on_enable_verification_requirement(this,status,type);
  return status;
}


/**
 * @brief  This function sends the DisableVerificationRequirement command
 * @param  uReadOrWrite enable the read or write protection ( 0x0001 : Read or 0x0002 : Write  )
 * @retval Status (SW1&SW2)   Status of the operation to complete.
 * @retval M24SR_ERROR_I2CTIMEOUT I2C timeout occurred.
 */
 M24SR::StatusTypeDef M24SR::M24SR_SendDisableVerificationRequirement(
    uint16_t uReadOrWrite) {
  C_APDU command;
  M24SR::StatusTypeDef status;
  uint8_t *pBuffer = uM24SRbuffer;
  uint16_t NbByte;

  /*check the parameters */
  if ((uReadOrWrite != 0x0001) && (uReadOrWrite != 0x0002)) {
    getCallback()->on_disable_verification_requirement(this,M24SR_IO_ERROR_PARAMETER,
         constToPasswordType(uReadOrWrite));
    return M24SR_IO_ERROR_PARAMETER;
  }

  /* build the command */
  command.Header.CLA = C_APDU_CLA_DEFAULT;
  command.Header.INS = C_APDU_DISABLE;
  /* copy the Password Id */
  command.Header.P1 = GETMSB(uReadOrWrite);
  command.Header.P2 = GETLSB(uReadOrWrite);
  /* build the IÃƒâ€šÃ‚Â²C command */
  M24SR_BuildIBlockCommand(M24SR_CMDSTRUCT_DISABLEVERIFREQ, &command,
      uDIDbyte, &NbByte, pBuffer);

  /* send the request */
  status = M24SR_IO_SendI2Ccommand(NbByte, pBuffer);
  if(status != M24SR_SUCCESS){
    getCallback()->on_disable_verification_requirement(this,status,
        constToPasswordType(uReadOrWrite));
    return status;
  }

  mLastCommandSend = DISABLE_VERIFICATION_REQUIREMENT;
  //use the offset filed for store the pwd id;
  mLastCommandData.offset = (uint8_t)uReadOrWrite;

  if(mCommunicationType==M24SR::SYNC){
    return M24SR_ReceiveDisableVerificationRequirement();
  }

  return M24SR_SUCCESS;

}

M24SR::StatusTypeDef M24SR::M24SR_ReceiveDisableVerificationRequirement(){
  M24SR::StatusTypeDef status;
  //TODO the size is 3?
  uint8_t rensponseBuffer[M24SR_STATUSRESPONSE_NBBYTE];

  PasswordType_t type = constToPasswordType(mLastCommandData.offset);

  status =M24SR_IO_ReceiveI2Cresponse (M24SR_STATUSRESPONSE_NBBYTE , rensponseBuffer );
  if(status!=M24SR_SUCCESS){
    getCallback()->on_disable_verification_requirement(this,status,type);
    return status;
  }//else

  status = M24SR_IsCorrectCRC16Residue(rensponseBuffer, M24SR_STATUSRESPONSE_NBBYTE);
  getCallback()->on_disable_verification_requirement(this,status,type);
  return status;
}


/**
 * @brief  This function sends the EnablePermananentState command
 * @param  uReadOrWrite   enable the read or write protection ( 0x0001 : Read or 0x0002 : Write  )
 * @retval Status (SW1&SW2)   Status of the operation to complete.
 * @retval M24SR_ERROR_I2CTIMEOUT I2C timeout occurred.
 */
 M24SR::StatusTypeDef M24SR::M24SR_SendEnablePermanentState(uint16_t uReadOrWrite) {
  C_APDU command;
  M24SR::StatusTypeDef status;

  uint8_t *pBuffer = uM24SRbuffer;
  uint16_t NbByte;

  /*check the parameters */
  if ((uReadOrWrite != 0x0001) && (uReadOrWrite != 0x0002)) {
    getCallback()->on_enable_permanent_state(this,M24SR_IO_ERROR_PARAMETER,
            constToPasswordType(uReadOrWrite));
    return M24SR_IO_ERROR_PARAMETER;
  }

  /* build the command */
  command.Header.CLA = C_APDU_CLA_ST;
  command.Header.INS = C_APDU_ENABLE;
  /* copy the Password Id */
  command.Header.P1 = GETMSB(uReadOrWrite);
  command.Header.P2 = GETLSB(uReadOrWrite);
  /* build the I2C command */
  M24SR_BuildIBlockCommand(M24SR_CMDSTRUCT_ENABLEVERIFREQ, &command, uDIDbyte,
      &NbByte, pBuffer);

  /* send the request */
  status = M24SR_IO_SendI2Ccommand(NbByte, pBuffer);
  if(status != M24SR_SUCCESS){
    getCallback()->on_enable_permanent_state(this,status,
        constToPasswordType(uReadOrWrite));
    return status;
  }

  mLastCommandSend = ENABLE_PERMANET_STATE;
  //use the offset filed for store the pwd id;
  mLastCommandData.offset = (uint8_t)uReadOrWrite;

  if(mCommunicationType==M24SR::SYNC){
    return M24SR_ReceiveEnablePermanentState();
  }

  return M24SR_SUCCESS;
}

M24SR::StatusTypeDef M24SR::M24SR_ReceiveEnablePermanentState(){
  M24SR::StatusTypeDef status;
  //TODO the size is 3?
  uint8_t rensponseBuffer[M24SR_STATUSRESPONSE_NBBYTE];

  PasswordType_t type = constToPasswordType(mLastCommandData.offset);

  status =M24SR_IO_ReceiveI2Cresponse (M24SR_STATUSRESPONSE_NBBYTE , rensponseBuffer );
  if(status!=M24SR_SUCCESS){
    getCallback()->on_enable_permanent_state(this,status,type);
    return status;
  }//else

  status = M24SR_IsCorrectCRC16Residue(rensponseBuffer, M24SR_STATUSRESPONSE_NBBYTE);
  getCallback()->on_enable_permanent_state(this,status,type);
  return status;
}


/**
 * @brief  This function sends the DisablePermanentState command
 * @param  uReadOrWrite enable the read or write protection ( 0x0001 : Read or 0x0002 : Write  )
 * @retval Status (SW1&SW2)   Status of the operation to complete.
 * @retval M24SR_ERROR_I2CTIMEOUT I2C timeout occurred.
 */
 M24SR::StatusTypeDef M24SR::M24SR_SendDisablePermanentState(uint16_t uReadOrWrite) {
  C_APDU command;
  M24SR::StatusTypeDef status;
  uint8_t *pBuffer = uM24SRbuffer;
  uint16_t NbByte;

  /*check the parameters */
  if ((uReadOrWrite != 0x0001) && (uReadOrWrite != 0x0002)) {
    getCallback()->on_disable_permanent_state(this,M24SR_IO_ERROR_PARAMETER,
        constToPasswordType(uReadOrWrite));
    return M24SR_IO_ERROR_PARAMETER;
  }

  /* build the command */
  command.Header.CLA = C_APDU_CLA_ST;
  command.Header.INS = C_APDU_DISABLE;
  /* copy the Password Id */
  command.Header.P1 = GETMSB(uReadOrWrite);
  command.Header.P2 = GETLSB(uReadOrWrite);
  /* build the I2C command */
  M24SR_BuildIBlockCommand(M24SR_CMDSTRUCT_DISABLEVERIFREQ, &command,
      uDIDbyte, &NbByte, pBuffer);

  /* send the request */
  status = M24SR_IO_SendI2Ccommand(NbByte, pBuffer);
  if(status != M24SR_SUCCESS){
    getCallback()->on_enable_permanent_state(this,status,
        constToPasswordType(uReadOrWrite));
    return status;
  }

  mLastCommandSend = DISABLE_PERMANET_STATE;
  //use the offset filed for store the pwd id;
  mLastCommandData.offset = (uint8_t)uReadOrWrite;

  if(mCommunicationType==M24SR::SYNC){
    return M24SR_ReceiveDisablePermanentState();
  }

  return M24SR_SUCCESS;
}

M24SR::StatusTypeDef M24SR::M24SR_ReceiveDisablePermanentState(){
  M24SR::StatusTypeDef status;
  //TODO the size is 3?
  uint8_t rensponseBuffer[M24SR_STATUSRESPONSE_NBBYTE];

  PasswordType_t type = constToPasswordType(mLastCommandData.offset);

  status =M24SR_IO_ReceiveI2Cresponse (M24SR_STATUSRESPONSE_NBBYTE , rensponseBuffer );
  if(status!=M24SR_SUCCESS){
    getCallback()->on_disable_permanent_state(this,status,type);
    return status;
  }//else

  status = M24SR_IsCorrectCRC16Residue(rensponseBuffer, M24SR_STATUSRESPONSE_NBBYTE);
  getCallback()->on_disable_permanent_state(this,status,type);
  return status;
}

/**
 * @brief  This function generates an interrupt on GPO pin
 * @param  None
 * @retval Status (SW1&SW2)   Status of the operation to complete.
 * @retval M24SR_ERROR_I2CTIMEOUT I2C timeout occurred.
 */
 M24SR::StatusTypeDef M24SR::M24SR_SendInterrupt(void) {
  C_APDU command;

  uint8_t *pBuffer = uM24SRbuffer;
  uint16_t uP1P2 = 0x001E;
  uint16_t NbByte;


  StatusTypeDef status = M24SR_ManageI2CGPO(INTERRUPT);
  if(status!=M24SR_SUCCESS)
    return status;

  /* build the command */
  command.Header.CLA = C_APDU_CLA_ST;
  command.Header.INS = C_APDU_INTERRUPT;
  /* copy the Password Id */
  command.Header.P1 = GETMSB(uP1P2);
  command.Header.P2 = GETLSB(uP1P2);
  command.Body.LC = 0x00;
  /* build the I2C command */
  M24SR_BuildIBlockCommand(M24SR_CMDSTRUCT_SENDINTERRUPT, &command, uDIDbyte,
      &NbByte, pBuffer);

  /* send the request */
  errchk(M24SR_IO_SendI2Ccommand(NbByte, pBuffer));
  //errchk(M24SR_IO_PollI2C());
  /* read the response */
  errchk(
      M24SR_IO_ReceiveI2Cresponse (M24SR_STATUSRESPONSE_NBBYTE , pBuffer ));

  return M24SR_IsCorrectCRC16Residue(pBuffer, M24SR_STATUSRESPONSE_NBBYTE);

}

/**
 * @brief  This function forces GPO pin to low state or high Z
 * @param  uSetOrReset select if GPO must be low (reset) or HiZ
 * @retval Status (SW1&SW2) Status of the operation to complete.
 * @retval M24SR_ERROR_I2CTIMEOUT I2C timeout occurred.
 */
 M24SR::StatusTypeDef M24SR::M24SR_StateControl(uint8_t uSetOrReset) {
  C_APDU command;
  uint8_t *pBuffer = uM24SRbuffer;
  uint16_t uP1P2 = 0x001F;
  uint16_t NbByte;

  /*check the parameters */
  if ((uSetOrReset != 0x01) && (uSetOrReset != 0x00)) {
    return M24SR_IO_ERROR_PARAMETER;
  }


  StatusTypeDef status = M24SR_ManageI2CGPO(STATE_CONTROL);
  if(status == M24SR_SUCCESS)
    return status;

  /* build the command */
  command.Header.CLA = C_APDU_CLA_ST;
  command.Header.INS = C_APDU_INTERRUPT;
  /* copy the Password Id */
  command.Header.P1 = GETMSB(uP1P2);
  command.Header.P2 = GETLSB(uP1P2);
  command.Body.LC = 0x01;
  command.Body.pData = &uSetOrReset;
  /* copy the data */
  //memcpy(command.Body.pData , &uSetOrReset, 0x01 );
  //command.Body.LE = 0x00 ;
  /* build the I2C command */
  M24SR_BuildIBlockCommand( M24SR_CMDSTRUCT_GPOSTATE, &command, uDIDbyte,
      &NbByte, pBuffer);

  /* send the request */
  errchk(M24SR_IO_SendI2Ccommand(NbByte, pBuffer));
  //errchk(M24SR_IO_PollI2C());
  /* read the response */
  errchk(
      M24SR_IO_ReceiveI2Cresponse (M24SR_STATUSRESPONSE_NBBYTE , pBuffer ));

  return M24SR_IsCorrectCRC16Residue(pBuffer, M24SR_STATUSRESPONSE_NBBYTE);

}


M24SR::StatusTypeDef M24SR::M24SR_ManageI2CGPO(NFC_GPO_MGMT GPO_I2Cconfig) {

  if (GPO_I2Cconfig > STATE_CONTROL) {
    return M24SR_IO_ERROR_PARAMETER;
  }
  //enable the callback for change the gpo
  mComponentCallback = &mManageGPOCallback;
  mManageGPOCallback.set_new_GPO_config(true,GPO_I2Cconfig);

  //start the manageGPO procedure
  return M24SR_SendSelectApplication();
}

M24SR::StatusTypeDef M24SR::M24SR_ManageRFGPO(NFC_GPO_MGMT GPO_I2Cconfig) {
  if (GPO_I2Cconfig > STATE_CONTROL) {
    return M24SR_IO_ERROR_PARAMETER;
  }

  //enable the callback for change the gpo
  mComponentCallback = &mManageGPOCallback;
  mManageGPOCallback.set_new_GPO_config(false,GPO_I2Cconfig);

  //start the manageGPO procedure
  return M24SR_SendSelectApplication();
}

M24SR::StatusTypeDef M24SR::M24SR_RFConfig(uint8_t OnOffChoice) {
  /* Disable RF */
  if (OnOffChoice != 0) {
    M24SR_IO_RFDIS_WritePin(GPIO_PIN_RESET);
  } else {
    M24SR_IO_RFDIS_WritePin(GPIO_PIN_SET);
  }
  return M24SR_SUCCESS;
}

int M24SR::M24SR_IO_I2C_Write(uint8_t *pBuffer, uint8_t NumByteToWrite) {
  dev_I2C->beginTransmission(((uint8_t)(((address) >> 1) & 0x7F)));

  for (int i = 0 ; i < NumByteToWrite ; i++)
    dev_I2C->write(pBuffer[i]);

  return dev_I2C->endTransmission(true);
}

int M24SR::M24SR_IO_I2C_Read(uint8_t *pBuffer, uint8_t NumByteToRead) {
  if(dev_I2C->requestFrom(((uint8_t)(((address) >> 1) & 0x7F)), (byte) NumByteToRead) == 0)
    return 1;

  int i=0;
  while (dev_I2C->available())
  {
    pBuffer[i] = dev_I2C->read();
    i++;
  }

  return 0;
}

M24SR::StatusTypeDef M24SR::M24SR_IO_SendI2Ccommand(uint8_t NbByte,
    uint8_t *pBuffer) {

  int nTry = 0;
  int status =1;
  while (status != 0 && (nTry++)<M24SR_MAX_I2C_ACCESS_TRY) {
    status = M24SR_IO_I2C_Write(pBuffer, NbByte);
  }
  if (status == 0)
    return M24SR_SUCCESS;

  return M24SR_IO_ERROR_I2CTIMEOUT;

}

M24SR::StatusTypeDef M24SR::M24SR_IO_ReceiveI2Cresponse(uint8_t NbByte,
    uint8_t *pBuffer) {

  int nTry = 0;
  int status =1;

  while (status != 0 && (nTry++)<M24SR_MAX_I2C_ACCESS_TRY) {
    status = M24SR_IO_I2C_Read(pBuffer, NbByte);
    if(status != 0) {
      delay(1); //delay required to avoid an error
    }
  }
  if (status == 0)
    return M24SR_SUCCESS;

  return M24SR_IO_ERROR_I2CTIMEOUT;
}


M24SR::StatusTypeDef M24SR::manage_event(void){

  switch(mLastCommandSend){
    case SELECT_APPLICATION:
      return M24SR_ReceiveSelectApplication();
    case SELECT_CC_FILE:
      return M24SR_ReceiveSelectCCfile();
    case SELECT_NDEF_FILE:
      return M24SR_ReceiveSelectNDEFfile();
    case SELECT_SYSTEM_FILE:
      return M24SR_ReceiveSelectSystemfile();
    case READ:
      return M24SR_ReceiveReadBinary();
    case UPDATE:
      return M24SR_ReceiveUpdateBinary();
    case VERIFY:
      return M24SR_ReceiveVerify();
    case DESELECT:
      return M24SR_ReceiveDeselect();
    case CHANGE_REFERENCE_DATA:
      return M24SR_ReceiveChangeReferenceData();
    case ENABLE_VERIFICATION_REQUIREMENT:
      return M24SR_ReceiveEnableVerificationRequirement();
    case DISABLE_VERIFICATION_REQUIREMENT:
      return M24SR_ReceiveDisableVerificationRequirement();
    case ENABLE_PERMANET_STATE:
      return M24SR_ReceiveEnablePermanentState();
    case DISABLE_PERMANET_STATE:
      return M24SR_ReceiveDisablePermanentState();

    default:
      return M24SR_SUCCESS;
  }//switch
}//manageInterrupt

NDefLib::NDefNfcTag* M24SR::get_NDef_tag(){
    return mNDefTagUtil;
}


bool M24SR::writeTxt(const char *text)
{
  bool success = true;

  //retrieve the NdefLib interface
  NDefLib::NDefNfcTag *tag = this->get_NDef_tag();

  //open the i2c session with the nfc chip
  if(tag->open_session()) {
    //create the NDef message and record
    NDefLib::Message msg;
    NDefLib::RecordText rText(text);
    msg.add_record(&rText);
    //write the tag
    if(tag->write(msg)) {
      success = true;
    } else {
      success = false;
    }

    //close the i2c session
    tag->close_session();
  } else {
    success = false;
  }
  return success;
}


void M24SR::readTxt(char text_read[])
{
  //retrieve the NdefLib interface
  NDefLib::NDefNfcTag *tag = this->get_NDef_tag();

  //open the i2c session with the nfc chip
  if(tag->open_session()) {

    //create the NDef message and record
    NDefLib::Message msg;

    //read the tag
    if(tag->read(&msg)) {
      const uint32_t nRecords = msg.get_N_records();
      for(int i =0 ;i<(int)nRecords ;i++) {
        if(msg[i]->get_type()== NDefLib::Record::TYPE_TEXT) {
          NDefLib::RecordText *rTxt = (NDefLib::RecordText *)msg[i];
          strcpy(text_read, rTxt->get_text().c_str());
        }
      }

      //free the read records
      NDefLib::Message::remove_and_delete_all_record(msg);
    }

    //close the i2c session
    tag->close_session();
  }
}

bool M24SR::writeUri(const char *uri)
{
  bool success = false;

  //retrieve the NdefLib interface
  NDefLib::NDefNfcTag *tag = this->get_NDef_tag();

  //open the i2c session with the nfc chip
  if(tag->open_session()) {
    //create the NDef message and record
    NDefLib::Message msg;
    NDefLib::RecordURI rUri(NDefLib::RecordURI::HTTP_WWW, uri);
    msg.add_record(&rUri);
    //write the tag
    if(tag->write(msg)) {
      success = true;
    } else {
      success = false;
    }

    //close the i2c session
    tag->close_session();
  } else {
    success = false;
  }
  return success;
}

void M24SR::readUri(char text_read[])
{
  //retrieve the NdefLib interface
  NDefLib::NDefNfcTag *tag = this->get_NDef_tag();

  //open the i2c session with the nfc chip
  if(tag->open_session()) {

    //create the NDef message and record
    NDefLib::Message msg;

    //read the tag
    if(tag->read(&msg)) {
      const uint32_t nRecords = msg.get_N_records();
      for(int i =0 ;i<(int)nRecords ;i++) {
        if(msg[i]->get_type()== NDefLib::Record::TYPE_URI) {
          NDefLib::RecordURI *rUri = (NDefLib::RecordURI *)msg[i];
          strcpy(text_read, rUri->get_content().c_str());
        }
      }

      //free the read records
      NDefLib::Message::remove_and_delete_all_record(msg);
    }

    //close the i2c session
    tag->close_session();
  }
}


bool M24SR::writeAAR(const char *text)
{
  bool success = true;

  //retrieve the NdefLib interface
  NDefLib::NDefNfcTag *tag = this->get_NDef_tag();

  //open the i2c session with the nfc chip
  if(tag->open_session()) {
    //create the NDef message and record
    NDefLib::Message msg;
    NDefLib::RecordAAR rAAR(text);
    msg.add_record(&rAAR);
    //write the tag
    if(tag->write(msg)) {
      success = true;
    } else {
      success = false;
    }

    //close the i2c session
    tag->close_session();
  } else {
    success = false;
  }
  return success;
}


void M24SR::readAAR(char text_read[])
{
  //retrieve the NdefLib interface
  NDefLib::NDefNfcTag *tag = this->get_NDef_tag();

  //open the i2c session with the nfc chip
  if(tag->open_session()) {

    //create the NDef message and record
    NDefLib::Message msg;

    //read the tag
    if(tag->read(&msg)) {
      const uint32_t nRecords = msg.get_N_records();
      for(int i =0 ;i<(int)nRecords ;i++) {
        if(msg[i]->get_type()== NDefLib::Record::TYPE_AAR) {
          NDefLib::RecordAAR *rAAR = (NDefLib::RecordAAR *)msg[i];
          strcpy(text_read, rAAR->get_package().c_str());
        }
      }

      //free the read records
      NDefLib::Message::remove_and_delete_all_record(msg);
    }

    //close the i2c session
    tag->close_session();
  }
}


bool M24SR::writeURIMail(const char *add, const char *subject, const char *body)
{
  bool success = true;

  //retrieve the NdefLib interface
  NDefLib::NDefNfcTag *tag = this->get_NDef_tag();

  //open the i2c session with the nfc chip
  if(tag->open_session()) {
    //create the NDef message and record
    NDefLib::Message msg;
    NDefLib::RecordMail rMail(add, subject, body);
    msg.add_record(&rMail);
    //write the tag
    if(tag->write(msg)) {
      success = true;
    } else {
      success = false;
    }

    //close the i2c session
    tag->close_session();
  } else {
    success = false;
  }
  return success;
}


void M24SR::readURIMail(char add[], char subject[], char body[])
{
  //retrieve the NdefLib interface
  NDefLib::NDefNfcTag *tag = this->get_NDef_tag();

  //open the i2c session with the nfc chip
  if(tag->open_session()) {

    //create the NDef message and record
    NDefLib::Message msg;

    //read the tag
    if(tag->read(&msg)) {
      const uint32_t nRecords = msg.get_N_records();
      for(int i =0 ;i<(int)nRecords ;i++) {
        if(msg[i]->get_type()== NDefLib::Record::TYPE_URI_MAIL) {
          NDefLib::RecordMail *rMail = (NDefLib::RecordMail *)msg[i];
          strcpy(add, rMail->get_to_address().c_str());
          strcpy(subject, rMail->get_subject().c_str());
          strcpy(body, rMail->get_body().c_str());
        }
      }

      //free the read records
      NDefLib::Message::remove_and_delete_all_record(msg);
    }

    //close the i2c session
    tag->close_session();
  }
}


bool M24SR::writeSMS(const char *recipient, const char *body)
{
  bool success = true;

  //retrieve the NdefLib interface
  NDefLib::NDefNfcTag *tag = this->get_NDef_tag();

  //open the i2c session with the nfc chip
  if(tag->open_session()) {
    //create the NDef message and record
    NDefLib::Message msg;
    NDefLib::RecordSMS rSMS(recipient, body);
    msg.add_record(&rSMS);
    //write the tag
    if(tag->write(msg)) {
      success = true;
    } else {
      success = false;
    }

    //close the i2c session
    tag->close_session();
  } else {
    success = false;
  }
  return success;
}


void M24SR::readSMS(char recipient[], char body[])
{
  //retrieve the NdefLib interface
  NDefLib::NDefNfcTag *tag = this->get_NDef_tag();

  //open the i2c session with the nfc chip
  if(tag->open_session()) {

    //create the NDef message and record
    NDefLib::Message msg;

    //read the tag
    if(tag->read(&msg)) {
      const uint32_t nRecords = msg.get_N_records();
      for(int i =0 ;i<(int)nRecords ;i++) {
        if(msg[i]->get_type()== NDefLib::Record::TYPE_URI_SMS) {
          NDefLib::RecordSMS *rSMS = (NDefLib::RecordSMS *)msg[i];
          strcpy(recipient, rSMS->get_number().c_str());
          strcpy(body, rSMS->get_messagge().c_str());
        }
      }

      //free the read records
      NDefLib::Message::remove_and_delete_all_record(msg);
    }

    //close the i2c session
    tag->close_session();
  }
}

bool M24SR::writeMime(const char *type, const char *data, const uint8_t dataLength)
{
  bool success = true;

  //retrieve the NdefLib interface
  NDefLib::NDefNfcTag *tag = this->get_NDef_tag();

  //open the i2c session with the nfc chip
  if(tag->open_session()) {
    //create the NDef message and record
    NDefLib::Message msg;
    NDefLib::RecordMimeType rMime(type, (uint8_t*)data, dataLength);
    msg.add_record(&rMime);
    //write the tag
    if(tag->write(msg)) {
      success = true;
    } else {
      success = false;
    }

    //close the i2c session
    tag->close_session();
  } else {
    success = false;
  }
  return success;
}


void M24SR::readMime(char *read_type, uint8_t *read_data, uint8_t *read_dataLength)
{
  //retrieve the NdefLib interface
  NDefLib::NDefNfcTag *tag = this->get_NDef_tag();

  //open the i2c session with the nfc chip
  if(tag->open_session()) {

    //create the NDef message and record
    NDefLib::Message msg;

    //read the tag
    if(tag->read(&msg)) {
      const uint32_t nRecords = msg.get_N_records();
      for(int i =0 ;i<(int)nRecords ;i++) {
        if(msg[i]->get_type()== NDefLib::Record::TYPE_MIME) {
          NDefLib::RecordMimeType *rMime = (NDefLib::RecordMimeType *)msg[i];
          strcpy(read_type, rMime->get_mime_type().c_str());
          *read_dataLength = rMime->get_mime_data_lenght();
          for (int i = 0; i<*read_dataLength; i++)
          {
            read_data[i] = rMime->get_mime_data()[i];
          }
        }
      }

      //free the read records
      NDefLib::Message::remove_and_delete_all_record(msg);
    }

    //close the i2c session
    tag->close_session();
  }
}


/******************* (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
