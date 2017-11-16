/**
 ******************************************************************************
 * @file    RecordVCard.h
 * @author  ST / Central Labs
 * @version V2.0.0
 * @date    28 Apr 2017
 * @brief   {@link RecordMimeType} that contains a VCard data
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

#ifndef NDEFLIB_RECORDTYPE_RECORDVCARD_H_
#define NDEFLIB_RECORDTYPE_RECORDVCARD_H_

#include <map>
#include <string>
#include "RecordMimeType.h"

namespace NDefLib {

/**
 * Specialize the {@link RecordMimeType} to store VCard information.
 * This record handles the VCard version 3 format.
 * @see https://en.wikipedia.org/wiki/VCard
 */
class RecordVCard: public RecordMimeType {
public:

	/**
	 * Type of information that you can store inside the tag
	 */
	typedef enum {
		ADDRESS,       //!< ADDRESS
		ADDRESS_HOME,  //!< ADDRESS_HOME
		ADDRESS_WORK,  //!< ADDRESS_WORK
		AGENT,         //!< AGENT
		BIRDAY,        //!< BIRDAY
		CATEGORIES,    //!< CATEGORIES
		EMAIL,         //!< EMAIL
		EMAIL_HOME,    //!< EMAIL_HOME
		EMAIL_WORK,    //!< EMAIL_WORK
		FORMATTED_NAME,//!< FORMATTED_NAME
		GEO,           //!< GEO latitude and longitude ; separated
		IMPP,          //!< IMPP
		PGPKEY_URL,    //!< PGPKEY_URL
		PGPGKEY_BASE64,//!< PGPGKEY_BASE64
		LOGO,          //!< LOGO
		LOGO_URI,      //!< LOGO_URI
		LOGO_BASE64,   //!< generic hardcoded image add TYPE=XXXX:imagebyte
		NAME,          //!< NAME
		NICKNAME,      //!< NICKNAME
		NOTE,          //!< NOTE
		ORGANIZATION,  //!< ORGANIZATION
		PHOTO_URI,     //!< PHOTO_URI
		PHOTO_BASE64,  //!< generic hardcoded image add TYPE=XXXX:imagebyte
		REVISION,      //!< REVISION
		SOURCE_URL,    //!< SOURCE_URL
		TEL,           //!< TEL
		TEL_HOME,      //!< TEL_HOME
		TEL_WORK,      //!< TEL_WORK
		TEL_MOBILE,    //!< TEL_MOBILE
		TITLE,         //!< TITLE
		URL,           //!< URL
	} VCardField_t;

	/**
	 * Type used to store the vcard information.
	 */
	typedef std::map<VCardField_t, std::string> VCardInfo_t;

	/**
	 * Create a RecordVCard reading the data from the buffer.
 	 * @param header Record header.
	 * @param buffer Buffer to read the data from.
	 * @return an object of type RecordVCard or NULL
	 * @par User is in charge of freeing the pointer returned by this function.
	 */
	static RecordVCard* parse(const RecordHeader &header,
			const uint8_t* buffer);

	/**
	 * Create a record with the specific information.
	 * @param Info optional information to store into the record.
	 */
	explicit RecordVCard(const VCardInfo_t &info=VCardInfo_t());

	/**
	 * Get the specific information stored in this record.
	 * @param Type type of information to get.
	 * @return if present, the information or an empty string
	 */
	const std::string& operator[](const VCardField_t &type)const {
		VCardInfo_t::const_iterator elem = mCardInfo.find(type);
		if (elem == mCardInfo.end())
			return sEmptyTagContent;
		//else
		return elem->second;
	}

	/**
	 * Get or set/change an information associated with this record.
	 * @param type Type of information to change.
	 * @return reference to the string information
	 */
	std::string& operator[](const VCardField_t &type) {
		mContentIsChange=true;
		return mCardInfo[type];
	}

    /**
     * Get the record type.
     * @return TYPE_MIME_VCARD
	 */
	virtual RecordType_t get_type() const {
		return TYPE_MIME_VCARD;
	} //getType

	/**
	 * @return update the record content and return the number of
	 * bytes needed to store this record
	 */
	virtual uint16_t get_byte_length() {
		update_content_info_string();
		return RecordMimeType::get_byte_length();
	}

	/**
	 * Update the content and write it on the buffer.
	 * @param[out] buffer buffer to write the record content into.
	 * @return number of bytes written
	 * @see Record#write
	 */
	virtual uint16_t write(uint8_t *buffer){
		update_content_info_string();
		return RecordMimeType::write(buffer);
	}

	/**
	 * Compare two objects.
	 * @return true if the records have the same Vcard information
	 */
	bool operator==(const RecordVCard &other){
		return (mCardInfo==other.mCardInfo);
	}

	virtual ~RecordVCard() {
	}

private:
	VCardInfo_t mCardInfo;

	std::string mCardInfoString; ///< buffer containing the Vcard representation for this record

	bool mContentIsChange; ///< true if we have to upadte the string rappresentation of the data

	/**
	 * Generate a string representing the vcard info
	 */
	void update_content_info_string();

	/**
	 * Check if the string that starts from the offset position is a valid
	 * vcard field.
	 * @param content String to search into.
	 * @param offset Search offset.
	 * @return type index of the found field or a negative number if not a valid vcard
	 */
	static int8_t find_VCard_field_type(const std::string &content,
			uint16_t offset);

	/**
	 * Return the lenght of a vcard field.
	 * @param Content string to search into.
	 * @param Offset search offset.
	 * @return field length
	 * @par This function searches the first sEndVCardTag appearance.
	 */
	static uint16_t find_VCard_field_data_lenght(const std::string &content,
			uint16_t offset);

	static const std::string sVcardMimeType;
	static const std::string sStartFieldTag[];
	static const std::string sEndFieldTag;
	static const std::string sStartVCardTag;
	static const std::string sEndVCardTag;
	static const std::string sEmptyTagContent;
};

} /* namespace NDefLib */

#endif /* NDEFLIB_RECORDTYPE_RECORDVCARD_H_ */
