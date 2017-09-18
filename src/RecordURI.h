/**
 ******************************************************************************
 * @file    RecordURI.h
 * @author  ST / Central Labs
 * @version V2.0.0
 * @date    28 Apr 2017
 * @brief   {@link Record} that contains an URI address
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

#ifndef NDEFLIB_RECORDTYPE_RECORDURI_H_
#define NDEFLIB_RECORDTYPE_RECORDURI_H_
#include <map>
#include <string>
#include <Record.h>

namespace NDefLib {

/**
 * {@link Record} that contains an URI address.
 */
class RecordURI: public Record {

public:

	/**
	 * Load a record URI from a buffer.
	 * @param header Record header.
	 * @param buffer Buffer to read the tag playload from.
	 * @return record or NULL if it was not possible build it
	 * @par User is in charge of freeing the pointer returned by this function.
	 */
	static RecordURI* parse(const RecordHeader &header,
			const uint8_t *buffer);

	/**
	 * If you want encode an know URI you can use this define to
	 * avoid to encode the URI type as string
	 */
	typedef enum {
		UNKNOWN = 0X00,     //!< UNKNOWN
		HTTP_WWW = 0X01,    //!< HTTP_WWW
		HTTPS_WWW = 0X02,   //!< HTTPS_WWW
		HTTP = 0X03,        //!< HTTP
		HTTPS = 0X04,       //!< HTTPS
		TEL = 0x05,         //!< TEL
		MAIL = 0X06,        //!< MAIL
		FTP_ANONIMUS = 0X07,//!< FTP_ANONIMUS
		FTP_FTP = 0X08,     //!< FTP_FTP
		FTPS = 0X09,        //!< FTPS
		SFTP = 0X0A,        //!< SFTP
		SMB = 0X0B,         //!< SMB
		NFS = 0X0C,         //!< NFS
		FTP = 0X0d,         //!< FTP
		DAV = 0X0E,         //!< DAV
		NEWS = 0X0F,        //!< NEWS
		TELNET = 0X10,      //!< TELNET
		IMAP = 0X11,        //!< IMAP
		RTSP = 0X12,        //!< RTSP
		URN = 0X13,         //!< URN
		POP = 0X14,         //!< POP
		SIP = 0X15,         //!< SIP
		SIPS = 0X016,       //!< SIPS
		TFTP = 0X017,       //!< TFTP
		BTSPP = 0x018,      //!< BTSPP
		BTL2CAP = 0x019,    //!< BTL2CAP
		BTGOEP = 0X01A,     //!< BTGOEP
		TCPOBEX = 0X1B,     //!< TCPOBEX
		IRDAOBEX = 0X1C,    //!< IRDAOBEX
		FILE = 0X1D,        //!< FILE
		URN_EPC_ID = 0X1E,  //!< URN_EPC_ID
		URN_EPC_TAG = 0X1F, //!< URN_EPC_TAG
		URN_EPC_PAT = 0X20, //!< URN_EPC_PAT
		URN_EPC_RAW = 0X21, //!< URN_EPC_RAW
		URN_EPC = 0X22,     //!< URN_EPC
		URN_NFC = 0X23      //!< URN_NFC
	} knowUriId_t;

	/**
	 * Build RecordUri with a defined URI type.
	 * @param URIId URI type.
	 * @param URIContent URI content.
	 * @par The URI prefix is removed.
	 * @par The string is copied inside the class.
	 */
	explicit RecordURI(knowUriId_t URIId, const std::string &URIContent="");

	/**
	 * Build a custom URI type.
	 * @param URIType string with the URI type.
	 * @param URIContent URI content.
	 */
	explicit RecordURI(const std::string &URIType, const std::string &URIContent="");

    /**
     * Get the record type.
     * @return TYPE_URI
	 */
	virtual RecordType_t get_type() const {
		return TYPE_URI;
	} //getType

	/**
	 * Returns the URI type.
	 * @return URI type inside this record, UNKNOWN if unknown
	 */
	knowUriId_t get_uri_id() const {
		return mUriTypeId;
	}

	virtual uint16_t get_byte_length() {
		update_content_and_header();
		return mRecordHeader.get_record_length();
	}

	/**
	 * Returns the URI content.
	 * @return URI content
	 */
	std::string& get_content() {
		update_content_and_header();
		return mContent;
	}

	/**
	 * Change the URI content.
	 * @param URI new URI content.
	 */
	void set_content(const std::string &URI){
		if(mUriTypeId!=UNKNOWN)
			store_removeing_prefix(sKnowUriPrefix[mUriTypeId],URI);
		else
			mContent=URI;
		update_content_and_header();
	}

	/**
	 * If the URI type Id is {@code UNKNOWN} this return the user
	 * URI type.
	 * @return URI type set by the user or an empyt string
	 */
	const std::string& get_uri_type() const {
		return mTypeString;
	}

	bool operator==(const RecordURI &other) const {
		return 	(mUriTypeId==other.mUriTypeId) &&
				(mTypeString==other.mTypeString) &&
				(mContent==other.mContent);
	}

	virtual uint16_t write(uint8_t *buffer);
	virtual ~RecordURI() {
	};

protected:

	/**
	 * Record id to write to be recognizable as an URI record.
	 */
	static const uint8_t sNDEFUriIdCode;

	/**
	 * A subclass must implement this function to store the tag content and update the mContent
	 * variable accordingly.
	 * @par You should not call this function directly but use updateContentAndHeader that keeps the
	 * header information in sync with the content.
	 */
	virtual void update_content(){};

	/**
	 * Update the tag content and update the header with the new content size.
	 */
	void update_content_and_header(){
		update_content();
		update_record_header();
	}

	/**
	 * The subclass must store in this variable the content to write to the tag.
	 * This class will ask to update the content throught the updateContent callback.
	 */
	std::string mContent;

private:

	/**
	 * Set the record header flags.
	 */
	void set_record_header();

	/**
	 * Set the correct size of the payload.
	 */
	void update_record_header(){
		//+1 = size of the URITypeId
		mRecordHeader.set_payload_length(1 + mTypeString.size() + mContent.size());
	}

	void store_removeing_prefix(const std::string &prefix,const std::string &content){
		//check that the content doens't contain the prefix
		if (content.compare(0, prefix.size(), prefix) == 0) {
			mContent = std::string(content, prefix.size());
		} else
			mContent = content;
	}

	/**
	 * URI type used by this record
	 */
	const knowUriId_t mUriTypeId;

	/**
	 * In case of unknown URI type, it stores the used defined URI type.
	 */
	const std::string mTypeString;


	/**
	 * Array of known prefix of known URI type
	 */
	static const std::string sKnowUriPrefix[];
};

} /* namespace NDefLib */

#endif /* NDEFLIB_RECORDTYPE_RECORDURI_H_ */
