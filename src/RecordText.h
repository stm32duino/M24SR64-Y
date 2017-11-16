/**
 ******************************************************************************
 * @file    RecordText.h
 * @author  ST / Central Labs
 * @version V2.0.0
 * @date    28 Apr 2017
 * @brief   {@link Record} containing a simple text
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


#ifndef NDEFLIB_RECORDTYPE_RECORDTEXT_H_
#define NDEFLIB_RECORDTYPE_RECORDTEXT_H_
#include <string>
#include <Record.h>

namespace NDefLib {

/**
 * {@link Record} containing a simple text.
 * @par The text is copied inside the class.
 */
class RecordText: public Record {
public:

	/**
	 * Read a recordText from a buffer.
	 * @param header Record header.
	 * @param buffer Buffer to read the record content from.
	 * @return a record of type Text or NULL if it was not possible build this type of record
	 * @par User is in charge of freeing the pointer returned by this function.
	 */
	static RecordText* parse(const RecordHeader &header,
			const uint8_t * const buffer);

	/**
	 * Set the type of encoding used to store the text data
	 */
	typedef enum {
		UTF8, //!< UTF8
		UTF16,//!< UTF16
	} TextEncoding;

	/**
	 * Build a utf8, English text record.
	 * @param text Text to store in the record.
	 */
	explicit RecordText(const std::string &text="");

	/**
	 * Build a text record.
	 * @param encoding Rype used to store the message.
	 * @param language Language used in the text.
	 * @param Text record text.
	 */
	RecordText(const TextEncoding encoding, const std::string &language,
			const std::string &text);

    /**
     * Get the record type.
     * @return TYPE_TEXT
	 */
	virtual RecordType_t get_type() const {
		return TYPE_TEXT;
	} //getType

	/**
	 * Get the text inside this record.
	 * @return the text content
	 */
	const std::string& get_text() const {
		return mText;
	}

	/**
	 * Change the text content.
	 * @param text New text to store.
	 */
	void set_text (const std::string &text){
		mText = text;
		update_playload_length();
	}

	/**
	 * Get the language used in the text.
	 * @return Language used in the text.
	 */
	const std::string& get_language()const{
		return mLanguage;
	}

	/**
	 * Get the encoding used to store the text.
	 * @return get the encoding used to store the text
	 */
	TextEncoding get_encoding()const{
		return mEncode;
	}

	/**
	 * Compare 2 objects.
	 * @return true if the objects have the same encoding,language and text
	 */
	bool operator==(const RecordText &other)const{
		return 	mTextStatus == other.mTextStatus &&
				mLanguage == other.mLanguage &&
				mText == other.mText;

	}

	virtual uint16_t write(uint8_t *buffer);
	virtual ~RecordText() {	};

private:

	void set_record_header();

	void update_playload_length(){
		mRecordHeader.set_payload_length(1 + mLanguage.size() + mText.size());
	}

	/**
	 * encode the text encoding type and language size in a single byte
	 * @param enc encoding used by this record
	 * @param language language used by this record
	 * @return text encoding and language size in a single byte
	 */
	static uint8_t get_text_status(TextEncoding enc,
			const std::string &language) {
		uint8_t status = language.size();
		if (enc == UTF16) {
			status &= 0x80; // set to 1 the bit 7
		} //if
		return status;
	} //getTextStatus

	/**
	 * extract the encoding information from a status byte
	 * @param textStatus status byte
	 * @return encoding used by this record
	 */
	static TextEncoding get_encoding(const uint8_t textStatus) {
		if ((textStatus & 0x80) == 0)
			return UTF8;
		else
			return UTF16;
	}

	/**
	 * extract language string length from a status byte
	 * @param textStatus status byte
	 * @return number of bytes needed to store the language name used by this record
	 */
	static uint8_t get_language_length(const uint8_t textStatus) {
		//take the fist 5 bits
		return textStatus & 0x1F;
	}

	const TextEncoding mEncode;
	const std::string mLanguage;
	std::string mText;

	const uint8_t mTextStatus;

	static const uint8_t NDEFTextIdCode;

};

} /* namespace NDefLib */

#endif /* NDEFLIB_RECORDTYPE_RECORDTEXT_H_ */
