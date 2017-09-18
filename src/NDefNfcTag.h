/**
 ******************************************************************************
 * @file    NDefNfcTag.h
 * @author  ST / Central Labs
 * @version V2.0.0
 * @date    28 Apr 2017
 * @brief   Generic interface that a device must implement to use the NDefLib
 * with async communication
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

#ifndef NDEFLIB_NDEFNFCTAG_H_
#define NDEFLIB_NDEFNFCTAG_H_

#include "Message.h"

namespace NDefLib {

/**
 * Abstract class used to write/read NDef messages to/from a nfc tag.
 * This class is made to handle also asynchronous communication with the nfc component.
 * All the functions in this class will return immediately, when a command ends a proper callback function will
 * be called.
 */
class NDefNfcTag {

public:

	/**
	 * Class that contains all the function called when a command finished.
	 * The default implementation is an empty function.
	 */
	class Callbacks {
		public:

			/**
			 * Called when a session is open.
			 * @param tag Tag where the session is open.
			 * @param success True if the operation has success.
			 */
			virtual void on_session_open(NDefNfcTag *tag,bool success){
				(void)tag;(void)success;
			};

			/**
			 * Called when a message is written.
			 * @param tag Tag where the message is written.
			 * @param success True if the operation has success.
			 */
			virtual void on_message_write(NDefNfcTag *tag,bool success,
					const Message &msg){
				(void)tag;(void)success; (void)msg;

			};

			/**
			 * Called when a message is read.
			 * @param tag Tag where the message is read.
			 * @param success True if the operation has success.
			 */
			virtual void on_message_read(NDefNfcTag *tag,bool success,
					const Message *msg){
				(void)tag;(void)success; (void)msg;
			};


			/**
			 * Called when a session is closed.
			 * @param tag Tag where the session is closed.
			 * @param success True if the operation has success.
			 */
			virtual void on_session_close(NDefNfcTag *tag,bool success){
				(void)tag;(void)success;
			};

			virtual ~Callbacks(){};
	};

private:

	/**
	 * Data used to store the callback status during a read/write operation.
	 */
	struct CallbackStatus{

		/** Object that triggers the callback */
		NDefNfcTag *callOwner;

		/** Message that the callback is writing/reading */
		Message *msg;
	};

public:

	NDefNfcTag():mCallBack(&mDefaultCallBack){}

	/**
	 * Set the callback object.
	 * @param c Object containing the callback.
	 */
	void set_callback(Callbacks *c){
		if(c!=NULL)
			mCallBack=c;
		else
			mCallBack=&mDefaultCallBack;
	}//setCallBack


	/**
	 * Open the communication session with the nfc tag.
	 * @param force Force to open a communication.
	 * @return true if success
	 */
	virtual bool open_session(bool force = false)=0;

	/**
	 * Close the communication with the nfc tag.
	 * @return true if success
	 */
	virtual bool close_session()=0;

	/**
	 * Returns true if a communication with the nfc tag is open.
	 * @return true if a communication with the nfc tag is open
	 */
	virtual bool is_session_open()=0;

	/**
	 * Write a message in the nfc tag.
	 * @par This call will delete the previous message.
	 * @param msg Message to write.
	 * @return true if success
	 */
	virtual bool write(Message &msg) {
		if(!is_session_open()){
			mCallBack->on_message_write(this,false,msg);
			return false;
		}

		const uint16_t length = msg.get_byte_length();
		uint8_t *buffer = new uint8_t[length];
		if(buffer==NULL){ //impossible to allocate the buffer
			mCallBack->on_message_write(this,false,msg);
			return false;
		}

		msg.write(buffer);

		mCallBackStatus.callOwner=this;
		mCallBackStatus.msg=&msg;

		return writeByte(buffer, length,0,NDefNfcTag::onWriteMessageCallback,&mCallBackStatus);
	}

	/**
	 * Read a message from the tag.
	 * @param[in,out] msg Message object the read records are added to.
	 * @return true if success
	 */
	virtual bool read(Message *msg) {
		if(!is_session_open()){
			mCallBack->on_message_read(this,false,msg);
			return false;
		}

		uint8_t *buffer = new uint8_t[2];
		if(buffer==NULL){
			mCallBack->on_message_read(this,false,msg);
			return false;
		}

		mCallBackStatus.callOwner=this;
		mCallBackStatus.msg=msg;
		return readByte(0,2,buffer,NDefNfcTag::onReadMessageLength,&mCallBackStatus);
	}

	virtual ~NDefNfcTag() {}

protected:

	typedef struct CallbackStatus CallbackStatus_t;

	/**
	 * Function that the component will call when a read/write operation is completed
	 * @param internalState Callback internal state data.
	 * @param status True if the operation succeed.
	 * @param buffer Buffer to read/write.
	 * @param length Number of byte read/write.
	 * @return true if the operation had success
	 */
	typedef bool(*byteOperationCallback_t)(CallbackStatus_t *internalState,
			bool status,const uint8_t *buffer, uint16_t length);

	/**
	 * Write a sequence of bytes to the NDEF file.
	 * @param buffer Buffer to write.
	 * @param length Number of bytes to write.
	 * @param offset Offset where start to write.
	 * @param callback Function to call when the operation ended.
	 * @param callbackStatus Parameter to pass to the callback function.
	 * @return true if the operation has success
	 */
	virtual bool writeByte(const uint8_t *buffer, uint16_t length,uint16_t offset,
			byteOperationCallback_t callback,CallbackStatus_t *callbackStatus)=0;

	/**
	 * Read a sequence of bytes from the NDEF file.
	 * @param byteOffset Read offset in bytes.
	 * @param byteLength Number of bytes to read.
	 * @param[out] buffer Buffer to store the read data into.
	 * @param callback Function to call when the operation ended.
	 * @param callbackStatus Parameter to pass to the callback function.
	 * @return true if the operation has success
	 */
	virtual bool readByte(const uint16_t byteOffset, const uint16_t byteLength,
			uint8_t *buffer, byteOperationCallback_t callback,CallbackStatus_t *callbackStatus)=0;


	/** object with the user callback */
	Callbacks *mCallBack;
private:

	/** object with the current callback status */
	CallbackStatus_t mCallBackStatus;
	/** default callback object, all the functions are empty */
	Callbacks mDefaultCallBack;

	/**
	 * Function called when a write operation completes, it will invoke on_message_write
	 * @param internalState Object that invokes the write operation.
	 * @param status True if the operation had success.
	 * @param buffer Buffer written.
	 * @param length Number of bytes written.
	 * @return true if the write had success
	 */
	static bool onWriteMessageCallback(CallbackStatus_t *internalState,
			bool status,const uint8_t *buffer, uint16_t ){
		delete [] buffer;

		internalState->callOwner->mCallBack->
			on_message_write(internalState->callOwner,status,*internalState->msg);
		return status;
	}

	/**
	 * Function called when a read operation completes.
	 * In this case we read the message length, this function will read all the message
	 * @param internalState Object that invokes the write operation.
	 * @param status true If the operation had success.
	 * @param buffer Buffer read.
	 * @param length Number of bytes read.
	 * @return true if the read had success
	 */
	static bool onReadMessageLength(CallbackStatus_t *internalState,
			bool status,const uint8_t *buffer, uint16_t length){

		if(!status || length!=2){
			internalState->callOwner->mCallBack->
					on_message_read(internalState->callOwner,false,internalState->msg);
			return false;
		}//if

		length = (((uint16_t) buffer[0]) << 8 | buffer[1]);
		delete [] buffer;

		uint8_t *readBuffer = new uint8_t[length];
		if(readBuffer==NULL){
			internalState->callOwner->mCallBack->
					on_message_read(internalState->callOwner,false,internalState->msg);
			return false;
		}//readBuffer

		internalState->callOwner->readByte(2,length,readBuffer,
				&NDefNfcTag::onReadMessageCallback,internalState);
		return status;
	}

	/**
	 * Function called when all messages have been read
	 * @param internalState Object that invokes the write operation.
	 * @param status True if the operation had success.
	 * @param buffer Buffer read.
	 * @param length Number of bytes read.
	 * @return true if the read had success
	 */
	static bool onReadMessageCallback(CallbackStatus_t *internalState,
			bool status,const uint8_t *buffer, uint16_t length){
		if(!status){
			internalState->callOwner->mCallBack->
					on_message_read(internalState->callOwner,false,internalState->msg);
			return false;
		}
		Message::parse_message(buffer, length, internalState->msg);
		delete [] buffer;
		internalState->callOwner->mCallBack->
			on_message_read(internalState->callOwner,true,internalState->msg);
		return status ;
	}

}; //class NDefNfcTagASync

}// namespace NDefLib

#endif /* NDEFLIB_NDEFNFCTAG_H_ */
