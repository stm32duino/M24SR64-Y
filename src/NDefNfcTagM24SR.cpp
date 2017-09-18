/**
  ******************************************************************************
  * @file       NdefNfcTagSTM24SR.cpp
  * @author  	ST Central Labs
  * @version 	V2.0.0
  * @date    	28 Apr 2017
  * @brief      Wrapper class of the NDefLib library to write/read NDEF messages
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
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

#include <cmath>

#include "NDefNfcTagM24SR.h"


/* wait 1sec, driver is configured to let 200ms for command to complete */
/* which is enough for all commands except GetSession if RF session is already opened */
/* Smartphone generally releases the session within the second, anyway the user can modify this value */
#define OPENSESSION_NTRIALS 5

#define CC_FILE_LENGTH_BYTE 15
////////////////////////////START  OpenSessionCallBack/////////////////////////
NDefNfcTagM24SR::OpenSessionCallBack::
	OpenSessionCallBack(NDefNfcTagM24SR& sender):
		mSender(sender),mNTrials(OPENSESSION_NTRIALS){}

void NDefNfcTagM24SR::OpenSessionCallBack::on_session_open(M24SR *nfc,
		M24SR::StatusTypeDef status){
	if(status==M24SR::M24SR_SUCCESS){
		nfc->select_application();
	}else{
		mSender.mCallBack->on_session_open(&mSender,false);
	}
}

void NDefNfcTagM24SR::OpenSessionCallBack::on_selected_application(M24SR *nfc,
		M24SR::StatusTypeDef status){
	if(status==M24SR::M24SR_SUCCESS){
		nfc->select_CC_file();
	}else{
		if(mNTrials==0)
			mSender.mCallBack->on_session_open(&mSender,false);
		else{
			mNTrials--;
			nfc->select_application();
		}//if-else
	}//if-else
}

void NDefNfcTagM24SR::OpenSessionCallBack::on_selected_CC_file(M24SR *nfc,
		M24SR::StatusTypeDef status){
	if(status==M24SR::M24SR_SUCCESS){
		nfc->read_binary(0x0000, CC_FILE_LENGTH_BYTE, CCFile);
	}else{
		mSender.mCallBack->on_session_open(&mSender,false);
	}
}

void NDefNfcTagM24SR::OpenSessionCallBack::on_read_byte(M24SR *nfc,
		M24SR::StatusTypeDef status,uint16_t offset,uint8_t *readByte, uint16_t nReadByte){
	(void)offset;
	if(status!=M24SR::M24SR_SUCCESS || nReadByte!=CC_FILE_LENGTH_BYTE){
		return mSender.mCallBack->on_session_open(&mSender,false);
	}//else
	uint16_t NDefFileId = (uint16_t) ((readByte[0x09] << 8) | readByte[0x0A]);
	mSender.mMaxReadBytes = (uint16_t) ((readByte[0x03] << 8) | readByte[0x04]);
	mSender.mMaxWriteBytes = (uint16_t) ((readByte[0x05] << 8) | readByte[0x06]);
	nfc->select_NDEF_file(NDefFileId);
}

void NDefNfcTagM24SR::OpenSessionCallBack::on_selected_NDEF_file(M24SR *nfc,
		M24SR::StatusTypeDef status){
	(void)nfc;

	mSender.mIsSessionOpen = status==M24SR::M24SR_SUCCESS;
	mSender.mCallBack->on_session_open(&mSender,mSender.mIsSessionOpen);
}
////////////////////////////END  OpenSessionCallBack/////////////////////////

bool NDefNfcTagM24SR::open_session(bool force) {

	if (is_session_open()){
		mCallBack->on_session_open(this,true);
		return true;
	}

	mDevice.set_callback(&mOpenSessionCallback);
	if(force)
		return mDevice.force_get_session() == M24SR::M24SR_SUCCESS;
	else
		return mDevice.get_session() == M24SR::M24SR_SUCCESS;


}

bool NDefNfcTagM24SR::close_session() {
	mDevice.set_callback(&mCloseSessionCallback);
	M24SR::StatusTypeDef status = mDevice.deselect();
	return status == M24SR::M24SR_SUCCESS;
}

void NDefNfcTagM24SR::WriteByteCallback::on_updated_binary(M24SR *nfc,
		M24SR::StatusTypeDef status,uint16_t startOffset, uint8_t *writeByte,uint16_t nWriteByte){

	if(status!=M24SR::M24SR_SUCCESS){ // error -> finish to write
		mCallback(mCallbackParam,false,mByteToWrite,mNByteToWrite);
		return;
	}//else

	mByteWrote+=nWriteByte;
	if(mByteWrote==mNByteToWrite){ //write all -> finish
		mCallback(mCallbackParam,true,mByteToWrite,mNByteToWrite);
	}else{ //else write another slice
		uint16_t tempLenght = 0;
        if(mSender.mMaxWriteBytes < (uint16_t)(mNByteToWrite-mByteWrote))
			tempLenght = mSender.mMaxWriteBytes;
		else
			tempLenght = (uint16_t)(mNByteToWrite-mByteWrote);
		nfc->update_binary(startOffset+nWriteByte,tempLenght,writeByte+nWriteByte);
	}//if-else
}

bool NDefNfcTagM24SR::writeByte(const uint8_t *buffer, uint16_t length,uint16_t offset,
		byteOperationCallback_t callback,CallbackStatus_t *callbackStatus){
	if(!is_session_open())
		callback(callbackStatus,false,buffer,length);
	//else
	mWriteByteCallback.set_task(buffer,length,callback,callbackStatus);
	mDevice.set_callback(&mWriteByteCallback);

	if (length > mMaxWriteBytes) {
		return mDevice.update_binary(offset, mMaxWriteBytes,(uint8_t*) buffer) == M24SR::M24SR_SUCCESS;
	}else{
		return mDevice.update_binary(offset,length,(uint8_t*)buffer) == M24SR::M24SR_SUCCESS;
	}//if-else

}

void NDefNfcTagM24SR::ReadByteCallback::on_read_byte(M24SR *nfc,
		M24SR::StatusTypeDef status,uint16_t startOffset, uint8_t *readBffer,uint16_t nReadByte){

	if(status!=M24SR::M24SR_SUCCESS){ // error -> finish to write
		mCallback(mCallbackParam,false,mBuffer,mNByteToRead);
		return;
	}//else

	mByteRead += nReadByte;
	if(mByteRead==mNByteToRead){ //read all -> finish
		mCallback(mCallbackParam,true,mBuffer,mNByteToRead);
	}else{ //else write another slice
		uint16_t tempLenght = 0;
		if(mSender.mMaxReadBytes < (uint16_t)(mNByteToRead-mByteRead))
			tempLenght = mSender.mMaxReadBytes;
		else
			tempLenght = (uint16_t)(mNByteToRead-mByteRead);
		nfc->read_binary(startOffset+nReadByte,tempLenght,readBffer+nReadByte);
	}//if-else

}

bool NDefNfcTagM24SR::readByte(const uint16_t byteOffset, const uint16_t length,
		uint8_t *buffer, byteOperationCallback_t callback,CallbackStatus_t *callbackStatus){
	if(!is_session_open()){
		return callback(callbackStatus,false,buffer,length);
	}
	//else
	mReadByteCallback.set_task(buffer,length,callback,callbackStatus);
	mDevice.set_callback(&mReadByteCallback);

	if (length > mMaxReadBytes) {
		return mDevice.read_binary(byteOffset, mMaxReadBytes,buffer)== M24SR::M24SR_SUCCESS;;
	}else{
		return mDevice.read_binary(byteOffset,length,buffer)== M24SR::M24SR_SUCCESS;;
	}//if-else

}
