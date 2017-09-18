/**
 ******************************************************************************
 * @file    RecordVCard.cpp
 * @author  ST / Central Labs
 * @version V2.0.0
 * @date    28 Apr 2017
 * @brief   RecordVCard implementation
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
#include "RecordVCard.h"

namespace NDefLib {

const std::string RecordVCard::sEmptyTagContent("");
const std::string RecordVCard::sVcardMimeType("text/vcard");
const std::string RecordVCard::sStartVCardTag("BEGIN:VCARD\nVERSION:3.0\n");
const std::string RecordVCard::sStartFieldTag[] =
		{ "ADR:", "ADR;TYPE=home:", "ADR;TYPE=work:", "AGENT:", "BDAY:",
				"CATEGORIES:", "EMAIL:", "EMAIL;TYPE=home:", "EMAIL;TYPE=work:",
				"FN:", "GEO:", "IMPP:", "KEY;TYPE=PGP:",
				"KEY;TYPE=PGP;ENCODING=B:","LOGO:", "LOGO;VALUE=uri:",
				"LOGO;ENCODING=B;", "N:", "NICKNAME:", "NOTE:", "ORG:",
				"PHOTO;VALUE=uri:", "PHOTO;ENCODING=B;", "REV:", "SOURCE:",
				"TEL:", "TEL;TYPE=HOME:", "TEL;TYPE=WORK:", "TEL;TYPE=CELL:",
				"TITLE:", "URL:", };

const std::string RecordVCard::sEndFieldTag("\n");
const std::string RecordVCard::sEndVCardTag("END:VCARD");

RecordVCard::RecordVCard(const VCardInfo_t &info) :
		RecordMimeType(sVcardMimeType), mCardInfo(info),mContentIsChange(true) {
	update_content_info_string();
}

void RecordVCard::update_content_info_string() {
	if(!mContentIsChange)
		return;

	mCardInfoString = sStartVCardTag;

	VCardInfo_t::const_iterator it = mCardInfo.begin();
	VCardInfo_t::const_iterator end = mCardInfo.end();

	for (; it != end; ++it) {
		mCardInfoString += sStartFieldTag[it->first];
		mCardInfoString += it->second;
		mCardInfoString += sEndFieldTag;
	} //for

	mCardInfoString += sEndVCardTag;

	set_mime_data_pointer((uint8_t*)mCardInfoString.data(),mCardInfoString.size());

	mContentIsChange=false;

}

uint16_t RecordVCard::find_VCard_field_data_lenght(const std::string &content,
		uint16_t offset) {
	std::size_t pos = content.find(sEndFieldTag, offset);
	if (pos == std::string::npos) { //if we don't find the end, lets consume all the content
		return content.size() - offset;
	} //else
	return content.find(sEndFieldTag, offset) - offset;
} //findVCardFieldDataLenght

int8_t RecordVCard::find_VCard_field_type(const std::string &content,
		uint16_t offset) {
	const uint32_t nFieldTag = sizeof(sStartFieldTag)
			/ sizeof(sStartFieldTag[0]);
	for (uint32_t i = 0; i < nFieldTag; i++) {
		if (content.find(sStartFieldTag[i], offset) != std::string::npos)
			return i;
	} //for
	return -1;
} //findVCardFieldType

RecordVCard* RecordVCard::parse(const RecordHeader &header,
		const uint8_t* buffer) {
	if (header.get_FNT() != RecordHeader::Mime_media_type
			|| header.get_type_length() != sVcardMimeType.size()) {
		return NULL;
	} //else
	if (sVcardMimeType.compare(0, sVcardMimeType.size(), (const char*) buffer,
			sVcardMimeType.size()) != 0) {
		return NULL;
	}
	buffer += header.get_type_length();
	//we are at the start of the vcard data
	if (sStartVCardTag.compare(0, sStartVCardTag.size(), (const char*) (buffer),
			sStartVCardTag.size()) != 0)
		return NULL;
	//the version is ok
	const std::string vCardContent((const char*) buffer + sStartVCardTag.size(),
			header.get_payload_length() - sStartVCardTag.size());
	uint16_t offset = 0;
	buffer += sStartVCardTag.size(); // for debug
	VCardInfo_t info;
	do {
		const int8_t type = find_VCard_field_type(vCardContent, offset);

		if (type >= 0) { //if is a valid type
			const VCardField_t fieldType = (VCardField_t) type;
			offset += sStartFieldTag[fieldType].size(); // skip the tag name
			//search the tag size
			const uint16_t length = find_VCard_field_data_lenght(vCardContent,
					offset);
			info[fieldType] = vCardContent.substr(offset, length);
			offset += length + sEndFieldTag.size();
		} else {
			//unknow field, skip until next one
			offset += find_VCard_field_data_lenght(vCardContent, offset)
					+ sEndFieldTag.size();
		}

	} while (offset < vCardContent.size());

	return new RecordVCard(info);
}

} /* namespace NDefLib */
