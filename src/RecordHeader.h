/**
 ******************************************************************************
 * @file    RecordHeader.h
 * @author  ST / Central Labs
 * @version V2.0.0
 * @date    28 Apr 2017
 * @brief   Record header class
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

#ifndef NDEFLIB_RECORDHEADER_H_
#define NDEFLIB_RECORDHEADER_H_

#include <stdint.h>

namespace NDefLib {

/**
 * Record header class.
 * @see NFC Data Exchange Format (NDEF) Technical Specification NDEF 1.0
 */
class RecordHeader {

public:

	/**
	 * Record type format
	 */
	typedef enum TypeNameFormat {
		Empty = 0x00,          //!< Empty
		NFC_well_known = 0x01, //!< NFC_well_known
		Mime_media_type = 0x02,//!< Mime_media_type
		Absolute_URI = 0x03,   //!< Absolute_URI
		NFC_external = 0x04,   //!< NFC_external
		Unknown = 0x05,        //!< Unknown
		Unchanged = 0x06,      //!< Unchanged
		Reserved = 0x07        //!< Reserved
	} TypeNameFormat_t;


	RecordHeader() :
			headerFlags(0), typeLength(0), payloadLength(0) {
		set_SR(true);
	}

	/**
	 * Set the 'message begin' flag.
	 * @param value True if the record is the first within the message.
	 */
	void set_MB(bool value) {
		if (value)
			headerFlags |= 0x80;
		else
			headerFlags &= ~0x80;
	}//set_MB

	/**
	 * Check the 'message begin' flag.
	 * @return true if it is the first record in the message
	 */
	bool get_MB() const {
		return (headerFlags & 0x80) != 0;
	}//get_MB

	/**
	 * Set the 'message end' flag.
	 * @param value True if it is the last record in the message.
	 */
	void set_ME(bool value) {
		if (value)
			headerFlags |= 0x40;
		else
			headerFlags &= ~0x40;
	}//set_ME

	/**
	 * Check the 'message end' flag.
	 * @return true if it is the last record in the message
	 */
	bool get_ME() const {
		return (headerFlags & 0x40) != 0;
	}//get_ME

	/**
	 * Set the 'Chunk' flag.
	 * @param value True if the record is in the first record chunk or in a middle record
	 * chunk of a chunked payload.
	 */
	void set_CF(bool value) {
		if (value)
			headerFlags |= 0x20;
		else
			headerFlags &= ~0x20;
	}//set_CF

	/**
	 * Check the 'Chunk' flag value.
	 * @return true if the record is in the first record chunk or in a middle record
	 * chunk of a chunked payload
	 */
	bool get_CF() const {
		return (headerFlags & 0x20) != 0;
	}//get_CF

	/**
	 * Set the 'Short record' flag value.
	 * @param value True if the record size can be encoded with 8 bits.
	 */
	void set_SR(bool value) {
		if (value)
			headerFlags |= 0x10;
		else
			headerFlags &= ~0x10;
	}//setSR

	/**
	 * Check the 'Short record' flag.
	 * @return true if the short range header format is set
	 */
	bool get_SR() const {
		return (headerFlags & 0x10) != 0;
	}//getSR

	/**
	 * Set the 'ID length' flag.
	 * @param value True if the 'ID length' value is used.
	 */
	void set_IL(bool value) {
		if (value)
			headerFlags |= 0x08;
		else
			headerFlags &= ~0x08;
	}//setIL

	/**
	 * Check the 'ID length' flag.
	 * @param value True if 'ID length' is set.
	 */
	bool get_IL() const {
		return (headerFlags & 0x08) != 0;
	}//getIL

	/**
	 * Set the type name format field.
	 * @param value Record type name format.
	 */
	void set_FNT(const TypeNameFormat_t value) {
		uint8_t temp = (uint8_t) value;
		temp &= 0x07; //keep the first 3 bits
		headerFlags &= 0xF8; //clean the fist 3 bits
		headerFlags |= temp; //set the fist 3 bits
	}//setFNT

	/**
	 * Get the record type name.
	 * @return type name format of the record
	 */
	TypeNameFormat_t get_FNT() const {
		return (TypeNameFormat_t) (headerFlags & 0x07);
	}

	/**
	 * Set the record payload length.
	 * @par This function will update the SR field as needed.
	 * @param length payload length
	 */
	void set_payload_length(uint32_t length) {
		payloadLength = length;
		set_SR(payloadLength <= 255);
	}

	/**
	 * Get the payload length.
	 * @return payload length
	 */
	uint32_t get_payload_length() const {
		return payloadLength;
	}

	/**
	 * Set the type length.
	 * @param size.
	 */
	void set_type_length(uint8_t size) {
		typeLength = size;
	}

	/**
	 * Get the type length.
	 * @return type length
	 */
	uint8_t get_type_length() const {
		return typeLength;
	}

	/**
	 * Set the id length.
	 * @param size.
	 */
	void set_id_length(uint8_t size) {
		if(size!=0){
			idLength = size;
			set_IL(true);
		}else
			set_IL(false);

	}

	/**
	 * Get the id length.
	 * @return id length
	 */
	uint8_t get_id_length() const {
		return idLength;
	}

	/**
	 * Get the number of bytes needed to store this record.
	 * @return 3 or 6
	 */
	uint16_t get_record_length() const {
		return (get_SR() ? 3 : 6) + (get_IL() ? idLength : 0)+typeLength + payloadLength;
	}

	/**
	 * Store the header information in the buffer.
	 * @param[out] outBuffer Buffer to write the header into.
	 * @return number of write bytes
	 */
	uint8_t write_header(uint8_t *outBuffer) const {

		uint32_t index = 0;

		outBuffer[index++] = headerFlags;
		outBuffer[index++] = typeLength;
		if (get_SR()) {
			outBuffer[index++] = (uint8_t) payloadLength;
		} else {
			outBuffer[index++] = (uint8_t) ((payloadLength & 0xFF000000)
					>> 24);
			outBuffer[index++] = (uint8_t) ((payloadLength & 0x00FF0000)
					>> 16);
			outBuffer[index++] = (uint8_t) ((payloadLength & 0x0000FF00)
					>> 8);
			outBuffer[index++] = (uint8_t) (payloadLength & 0x000000FF);
		} //if-else
		if(get_IL())
			outBuffer[index++] =idLength;
		return index;
	} //writeHeader

	/**
	 * Load an header from a buffer.
	 * @param buffer Buffer to load the header from.
	 * @return number of read bytes
	 */
	uint16_t load_header(const uint8_t * const buffer) {
		uint32_t index = 0;
		headerFlags = buffer[index++];
		typeLength = buffer[index++];
		if (get_SR()) {
			payloadLength = buffer[index++];
		} else {
			payloadLength = (((uint32_t) buffer[index + 0]) << 24)
					| (((uint32_t) buffer[index + 1]) << 16)
					| (((uint32_t) buffer[index + 2]) << 8)
					| ((uint32_t) buffer[index + 3]);
			index += 4;
		} //if-else
		if(get_IL())
			idLength=buffer[index++];
		else
			idLength=0;
		return index;
	} //loadHeader

	/**
	 * Equal operator.
	 * @param other Other object to compare with.
	 * @return true if the two record headers are equal
	 */
	bool operator==(const RecordHeader &other) const{
		return (headerFlags==other.headerFlags) &&
				(typeLength==other.typeLength) &&
				(payloadLength==other.payloadLength);
	}

	bool operator!=(const RecordHeader &other) const{
		return !(*this==other);
	}

private:
	uint8_t idLength;
	uint8_t headerFlags;
	uint8_t typeLength;
	uint32_t payloadLength;
};

} /* namespace NDefLib */

#endif /* NDEFLIB_RECORDHEADER_H_ */
