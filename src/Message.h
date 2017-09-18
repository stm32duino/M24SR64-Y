/**
 ******************************************************************************
 * @file    Message.h
 * @author  ST / Central Labs
 * @version V2.0.0
 * @date    28 Apr 2017
 * @brief   NDef Message class
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

#ifndef NDEFLIB_MESSAGE_H_
#define NDEFLIB_MESSAGE_H_

#include <algorithm>
#include <vector>

#include "Record.h"

namespace NDefLib {

/**
 * Class containing a list of {@link Record}
 */
class Message {
public:

	/**
	 * Add a ndef record to this message.
	 * @param r Record to add
	 */
	void add_record(Record *r) {
		mRecords.push_back(r);
	}

	/**
	 * Remove a ndef record to this message
	 * @param r record to remove
	 */
	void remove_record(Record *r){
		mRecords.erase( std::remove( mRecords.begin(), mRecords.end(), r ),
			 mRecords.end() );
	}


	/**
	 * Add all the records in the list to this message.
	 * @param addList List of records to add.
	 */
	void add_records(const std::vector<Record*> &addList) {
		mRecords.insert(mRecords.end(), addList.begin(), addList.end());
	}

	/**
	 * Get the specific record contained by this message, NULL if not a valid index.
	 * @param index Record index.
	 * @return a Record object if present, otherwise NULL
	 */
	Record* operator[](const uint32_t index)const{
		if (index >= mRecords.size())
			return NULL;
		return mRecords[index];
	}

	/**
	 * Get the number of records in this message.
	 * @return number of records in this message
	 */
	uint32_t get_N_records() const {
		return mRecords.size();
	}

	/**
	 * Length in bytes needed to write this message.
	 * @return number of bytes needed to write this message
	 */
	uint16_t get_byte_length() const;

	/**
	 * Write message in the provided buffer
	 * @par The first 2 bytes contain the NDEF message length.
	 * @param[out] buffer Buffer the message must be written into.
	 * @return number of bytes written
	 */
	uint16_t write(uint8_t *buffer) const;

	/**
	 * Create a set of records from a raw buffer adding them to a message object.
	 * @par Message buffer must NOT contain the buffer length in the first two bytes.
	 * @param buffer Buffer containing the message record.
	 * @param bufferLength Buffer length.
	 * @param[in,out] Message message that will contain the new records.
	 */
	static void parse_message(const uint8_t * const buffer,
			const uint16_t bufferLength, Message *message);

	/**
	 * Remove all the recrods from the mesasge and delete it
	 * @param msg Message with the records to delete
	 */
	static void remove_and_delete_all_record(Message &msg);

	virtual ~Message() {
	}

private:
	/**
	 * List of records contained by this message.
	 */
	std::vector<Record*> mRecords;
};

} /* namespace NDefLib */

#endif /* NDEFLIB_MESSAGE_H_ */
