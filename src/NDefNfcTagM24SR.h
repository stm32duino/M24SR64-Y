/**
  ******************************************************************************
  * @file       Type4NfcTagM24SR.h
  * @author  	ST Central Labs
  * @version 	V2.0.0
  * @date    	28 Apr 2017
  * @brief      M24SR specific NDefLib derived class
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

#ifndef M24SR_NDEFNFCTAGM24SR_H_
#define M24SR_NDEFNFCTAGM24SR_H_

#include <cstdlib>
#include <stdint.h>

#include "NDefNfcTag.h"

#include "M24SR.h"

/**
 * Helper class to use the NDefLib
 */
class NDefNfcTagM24SR: public NDefLib::NDefNfcTag {

public:

	/**
	 *
	 * @param device device to use
	 */
	NDefNfcTagM24SR(M24SR &device):
		NDefLib::NDefNfcTag(),
		mDevice(device),mIsSessionOpen(false),
		mMaxReadBytes(0xFF), mMaxWriteBytes(0xFF),
		mOpenSessionCallback(*this),
		mCloseSessionCallback(*this),
		mWriteByteCallback(*this),
		mReadByteCallback(*this){}

	virtual bool open_session(bool force = false);

	virtual bool close_session();

	virtual bool is_session_open(){
		return mIsSessionOpen;
	}

	/**
	* Close the open session.
	*/
	virtual ~NDefNfcTagM24SR(){
		if(is_session_open())
			close_session();
	}//~NDefNfcTagM24SR

	protected:

		virtual bool writeByte(const uint8_t *buffer, uint16_t length,uint16_t offset,
				byteOperationCallback_t callback,CallbackStatus_t *callbackStatus);

		virtual bool readByte(const uint16_t byteOffset, const uint16_t byteLength,
				uint8_t *buffer, byteOperationCallback_t callback,CallbackStatus_t *callbackStatus);

	private:

		M24SR &mDevice;

		/**
		 * true if the session is open
		 */
		bool mIsSessionOpen;

		/**
		* Max length for a read operation
		*/
		uint16_t mMaxReadBytes;

		/**
		* Max length for a write operation
		*/
		uint16_t mMaxWriteBytes;

		/**
		 * Class containing the callback needed to open a session and read the max
		 * read/write size
		 */
		class OpenSessionCallBack: public M24SR::Callbacks{
			public:
				OpenSessionCallBack(NDefNfcTagM24SR &sender);

				virtual void on_session_open(M24SR *nfc,M24SR::StatusTypeDef status);
				virtual void on_selected_application(M24SR *nfc,M24SR::StatusTypeDef status);
				virtual void on_selected_CC_file(M24SR *nfc,M24SR::StatusTypeDef status);
				virtual void on_read_byte(M24SR *nfc,M24SR::StatusTypeDef status,uint16_t offset,
						uint8_t *readByte, uint16_t nReadByte);
				virtual void on_selected_NDEF_file(M24SR *nfc,M24SR::StatusTypeDef status);

			private:
				/**
				 * Object that send the open session callback
				 */
				NDefNfcTagM24SR &mSender;

				/**
				 * number of trials done for open the session
				 */
				uint32_t mNTrials;

				/**
				 * buffer where read the CC file
				 */
				uint8_t CCFile[15];
		};

		OpenSessionCallBack mOpenSessionCallback;
		friend class OpenSessionCallBack;

		/**
		 * Class containing the callback needed to close a session
		 */
		class CloseSessionCallBack : public M24SR::Callbacks{
			public:
				CloseSessionCallBack(NDefNfcTagM24SR &sender):
					mSender(sender){}

				virtual void on_deselect(M24SR *nfc,M24SR::StatusTypeDef status){
					(void)nfc;
					if(status==M24SR::M24SR_SUCCESS){
						mSender.mIsSessionOpen=false;
						mSender.mCallBack->on_session_close(&mSender,true);
					}else{
						mSender.mCallBack->on_session_close(&mSender,false);
					}//if-else
				}
			private:

				/**
				 * Object that send the open session callback
				 */
				NDefNfcTagM24SR &mSender;
		};

		CloseSessionCallBack mCloseSessionCallback;
		friend class OpenSessionCallBack;

		/**
		 * Class containing the callback needed to write a buffer
		 */
		class WriteByteCallback : public M24SR::Callbacks{
			public:

				/**
				 *
				 * @param sender tag where write the buffer
				 */
				WriteByteCallback(NDefNfcTagM24SR &sender):
									mByteToWrite(NULL),
									mNByteToWrite(0),
									mByteWrote(0),
									mCallback(NULL),
									mCallbackParam(NULL),
									mSender(sender){}

				/**
				 * Set the buffer to write and the function to call when finish
				 * @param buffer Buffer to write.
				 * @param nByte Number of bytes to write.
				 * @param callback Function to call when the write ends.
				 * @param param Parameter to pass to the callback function.
				 */
				void set_task(const uint8_t *buffer,uint16_t nByte,
						byteOperationCallback_t callback,CallbackStatus_t *param){
					mByteToWrite=buffer;
					mNByteToWrite=nByte;
					mByteWrote=0;
					mCallback = callback;
					mCallbackParam = param;
				}

				virtual void on_updated_binary(M24SR *nfc,M24SR::StatusTypeDef status,
						uint16_t startOffset,uint8_t *writeByte,uint16_t nWriteByte);


			private:

				/** buffer to write */
				const uint8_t *mByteToWrite;
				/** length of the buffer */
				uint16_t mNByteToWrite;
				/** number of byte already wrote */
				uint16_t mByteWrote;

				/** function to call when all the bytes are write */
				byteOperationCallback_t mCallback;
				/** parameter to pass to the callback function*/
				CallbackStatus_t* mCallbackParam;

				/** tag where write the buffer*/
				NDefNfcTagM24SR &mSender;
		};

		WriteByteCallback mWriteByteCallback;
		friend class WriteByteCallback;

		/**
		 * Class containing the callback needed to read a buffer
		 */
		class ReadByteCallback : public M24SR::Callbacks{
			public:

				/**
				 *
				 * @param Sender tag where read the buffer
				 */
				ReadByteCallback(NDefNfcTagM24SR &sender):
										mBuffer(NULL),
										mNByteToRead(0),
										mByteRead(0),
										mCallback(NULL),
										mCallbackParam(NULL),
										mSender(sender){}

				/**
				 * Set the buffer where read the data and the function to call when finish
				 * @param buffer Buffer read
				 * @param nByte Number of bytes to read
				 * @param callback Function to call when the read ends
				 * @param param Parameter to pass to the callback function
				 */
				void set_task(uint8_t *buffer,uint16_t nByte,
						byteOperationCallback_t callback,CallbackStatus_t *param){
					mBuffer=buffer;
					mNByteToRead=nByte;
					mByteRead=0;
					mCallback = callback;
					mCallbackParam = param;
				}

				virtual void on_read_byte(M24SR *nfc,M24SR::StatusTypeDef status,
						uint16_t offset,uint8_t *readByte, uint16_t nReadByte);


			private:

				/**
				 * Buffer where read the data
				 */
				uint8_t *mBuffer;

				/**
				 * Number of bytes to read
				 */
				uint16_t mNByteToRead;

				/**
				 * Number of bytes already read
				 */
				uint16_t mByteRead;

				/**
				 * Function to call when all the bytes are read
				 */
				byteOperationCallback_t mCallback;

				/**
				 * Parameter to pass to the callback function
				 */
				CallbackStatus_t* mCallbackParam;

				/**
				 * Tag where the data are read
				 */
				NDefNfcTagM24SR &mSender;
		};

		ReadByteCallback mReadByteCallback;
		friend class ReadByteCallback;


};

#endif /* M24SR_NDEFNFCTAGM24SR_H_ */
