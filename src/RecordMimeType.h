/**
 ******************************************************************************
 * @file    RecordMimeType.h
 * @author  ST / Central Labs
 * @version V2.0.0
 * @date    28 Apr 2017
 * @brief   {@link Record} that contains data encoded as MimeType.
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

#ifndef NDEFLIB_RECORDTYPE_RECORDMIMETYPE_H_
#define NDEFLIB_RECORDTYPE_RECORDMIMETYPE_H_

#include <cstring>
#include <string>

#include "Record.h"

namespace NDefLib {

/**
 * {@link Record} that contains data encoded as MimeType.
 */
class RecordMimeType: public Record {
public:

	/**
	 * Create an RecordMimeType reading the data from the buffer.
 	 * @param header Record header.
	 * @param buffer Buffer to read the data from.
	 * @return an object of type RecordMimeType or NULL
	 * @par User is in charge of freeing the pointer returned by this function.
	 */
	static RecordMimeType* parse(const RecordHeader &header,
			const uint8_t* buffer);

	/**
	 * Create a new record with a specific MIME type.
	 * @param mimeType MIME type.
	 * @param data Content.
	 * @param nDataLenght Content length in bytes.
	 * @par Data content is copied inside the class, unless NULL pointer is passed and
	 * @{link setMimeDataPointer} is used to set the data pointer.
	 */
	RecordMimeType(const std::string &mimeType, const uint8_t *data=NULL,
			uint32_t nDataLenght=0);

	/**
	 * Create a new RecordMimeType.
	 * @param mimeType MIME type.
	 * @param data String that contains the data.
	 * @par The string is copied inside the class.
	*/
	RecordMimeType(const std::string &mimeType, const std::string &data);

    /**
     * Return the mime type of the content.
     * @return the mime type of the content.
     */
	const std::string& get_mime_type() const {
		return mMimeType;
	}

	/**
	 * Change the data pointer used by this record.
	 * @param data Pointer used by this record.
	 * @param dataLength Number of byte to write in this record.
	 * @par The buffer is not copied or freed by this class.
	 * With this function only the data pointer is copied, so it must not be freed before the object is deallocated.
	 */
	void set_mime_data_pointer(uint8_t* data, uint32_t dataLength){
		delete_mime_data();
		mData = data;
		mDataLength=dataLength;
		mRecordHeader.set_payload_length(dataLength);
	}

	/**
	 * Change the data linked with this record, with this function the
	 * data are copied inside the object.
	 * @param data Pointer to the data buffer.
	 * @param dataLength Number of bytes to write.
	 */
	void copy_mime_data(const uint8_t* data, uint32_t dataLength);

	/**
	 * @return number of bytes used to store the content data
	 */
	uint32_t get_mime_data_lenght() const {
		return mDataLength;
	}

	/**
	 * @return pointer to the content data
	 * @par DO NOT free this pointer, it is managed by the class.
	 */
	uint8_t const* get_mime_data() const {
		return mData;
	}

	/**
     * Get the record type.
     * @return TYPE_MIME
	 */
	virtual RecordType_t get_type() const {
		return TYPE_MIME;
	} //getType


	virtual uint16_t write(uint8_t *buffer);

	/**
	 * If needed free the memory used to store the content data.
	 */
	virtual ~RecordMimeType() {
		delete_mime_data();
	};

    /**
     * Compare two objects
     * @return true if the 2 records have the same mime type, length and data.
     */
	bool operator==(const RecordMimeType &other) const {
		return 	mMimeType==other.mMimeType &&
				mDataLength == other.mDataLength &&
				(std::memcmp(mData,other.mData,mDataLength)==0);
	}

private:

	void initialize_header_data();

	/**
	 * Delete the mData buffer if it is allocated by this class.
	 */
	void delete_mime_data(){
		if(mDataToFree && mData!=NULL){
			mDataToFree=false;
			delete [] mData;
			mData=NULL;
		}//if
	}//deleteMimeData

	const std::string mMimeType;
	uint32_t mDataLength;
	uint8_t *mData;
	bool mDataToFree;

};

} /* namespace NDefLib */

#endif /* NDEFLIB_RECORDTYPE_RECORDMIMETYPE_H_ */
