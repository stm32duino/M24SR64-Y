/**
 ******************************************************************************
 * @file    Record.h
 * @author  ST / Central Labs
 * @version V2.0.0
 * @date    28 Apr 2017
 * @brief   Generic Record class
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

#ifndef NDEFLIB_RECORD_H_
#define NDEFLIB_RECORD_H_
#include <stdint.h>

#include "RecordHeader.h"

namespace NDefLib {

/**
 * Base class for a NDefRecord
 * @see NFC Data Exchange Format (NDEF) Technical Specification NDEF 1.0
 */
class Record {
public:

	/**
	 * Enum used to identify the record type.
	 */
	typedef enum {
		TYPE_UNKNOWN,        //!< UNKNOWN record
		TYPE_TEXT,           //!< TEXT
		TYPE_AAR,            //!< Android Archive record
		TYPE_MIME,           //!< generic MIME type
		TYPE_URI,            //!< generic URI
		TYPE_URI_MAIL,       //!< Email URI record
		TYPE_URI_SMS,        //!< SMS URI record
		TYPE_URI_GEOLOCATION,//!< position URI record
		TYPE_MIME_VCARD,     //!< VCard record
		TYPE_WIFI_CONF 		 //!< Wifi configuration
	} RecordType_t;

	Record() {
	}

	/**
	 * Set the record as the first record in the message.
	 */
	void set_as_first_record() {
		mRecordHeader.set_MB(true);
	}

	/**
	 * Set the record as the last record in the message.
	 */
	void set_as_last_record() {
		mRecordHeader.set_ME(true);
	}

	/**
	 * Check if it is the last record in the message.
	 * @return true if it is the last record in the message
	 */
	bool is_last_record() const {
		return mRecordHeader.get_ME();
	}

	/**
	 * Check if it is the first record in the message.
	 * @return true if it is the fist record in the message
	 */
	bool is_first_record() const {
		return mRecordHeader.get_MB();
	}

	/**
	 * Set the record as generic (not the first one and not the last one)
	 */
	void set_as_middle_record() {
		mRecordHeader.set_MB(false);
		mRecordHeader.set_ME(false);
	}

	/**
	 * Check if the record is in the middle of a chain.
	 * @return true if is not the fist or the last one
	 */
	bool is_middle_record() const{
		return ! (mRecordHeader.get_MB() || mRecordHeader.get_ME());
	}

	/**
	 * Get tag type.
	 * @par This method should be overridden to return a valid type.
	 * @return tag type
	 */
	virtual RecordType_t get_type() const {
		return TYPE_UNKNOWN;
	} //getType


	/**
	 * Get the record header.
	 * @return record header
	 */
	const RecordHeader& get_header() const{
		return mRecordHeader;
	}

	/**
	 * Number of bytes needed to store this record.
	 * @return size of the header + size of the record content
	 */
	virtual uint16_t get_byte_length() {
		return mRecordHeader.get_record_length();
	}

	/**
	 * Write the record content into a buffer.
	 * @param[out] buffer buffer to write the record content into, the buffer size
	 *  must be almost {@link Record#getByteLength} bytes.
	 * @return number of written bytes
	 */
	virtual uint16_t write(uint8_t *buffer)=0;

	virtual ~Record() {
	};

protected:
	RecordHeader mRecordHeader;
};

} /* namespace NDefLib */

#endif /* NDEFLIB_RECORD_H_ */
