/**
 ******************************************************************************
 * @file    RecordText.cpp
 * @author  ST / Central Labs
 * @version V2.0.0
 * @date    28 Apr 2017
 * @brief   RecordText implementation
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

#include <cstring>
#include <RecordText.h>

namespace NDefLib {

const uint8_t RecordText::NDEFTextIdCode = 'T';

void RecordText::set_record_header() {
	mRecordHeader.set_FNT(RecordHeader::NFC_well_known);
	mRecordHeader.set_type_length(sizeof(NDEFTextIdCode));
	update_playload_length();
}

RecordText::RecordText(const std::string &text) :
		mEncode(UTF8), mLanguage("en"), mText(text), mTextStatus(
				get_text_status(mEncode, mLanguage)) {
	set_record_header();
}

RecordText::RecordText(const TextEncoding encoding, const std::string &language,
		const std::string &text) :
		mEncode(encoding), mLanguage(language), mText(text), mTextStatus(
				get_text_status(mEncode, mLanguage)) {
	set_record_header();
}

uint16_t RecordText::write(uint8_t *buffer) {
	int16_t offset = 0;
	offset += mRecordHeader.write_header(buffer);

	buffer[offset++] = NDEFTextIdCode;
	buffer[offset++] = mTextStatus;

	std::memcpy(buffer + offset, mLanguage.c_str(), mLanguage.size());
	offset += mLanguage.size();
	std::memcpy(buffer + offset, mText.c_str(), mText.size());
	offset += mText.size();
	return offset;
}

RecordText* RecordText::parse(const RecordHeader &header,
		const uint8_t * const buffer) {
	uint32_t index = 0;
	if (header.get_FNT() == RecordHeader::NFC_well_known
			&& buffer[index++] == NDEFTextIdCode) {

		const uint8_t textStatus = buffer[index++];
		const TextEncoding enc = get_encoding(textStatus);
		const uint8_t langSize = get_language_length(textStatus);
		//-1 is the textStatus
		const uint8_t textSize = header.get_payload_length() - langSize - 1;

		return new RecordText(enc,
				std::string((const char*) (buffer + index), langSize),
				std::string((const char*) (buffer + index + langSize),
						textSize));
	} else
		return NULL;
}

} /* namespace NDefLib */
