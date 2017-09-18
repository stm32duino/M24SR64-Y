/**
 ******************************************************************************
 * @file    RecordURI.cpp
 * @author  ST / Central Labs
 * @version V2.0.0
 * @date    28 Apr 2017
 * @brief   RecordURI implementation
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

#include "RecordURI.h"
#include "RecordMail.h"
#include "RecordSMS.h"
#include "RecordGeo.h"

namespace NDefLib {

const uint8_t RecordURI::sNDEFUriIdCode = 'U';

const std::string RecordURI::sKnowUriPrefix[] = { "", "http://www.",
		"https://www.", "http://", "https://", "tel:", "mailto:",
		"ftp://anonymous:anonymous@", "ftp://ftp.", "ftps://", "sftp://",
		"smb://", "nfs://", "ftp://", "dav://", "news:", "telnet://", "imap:",
		"rtsp://", "urn:", "pop:", "sip:", "sips:", "tftp:", "btspp://",
		"btl2cap://", "btgoep://", "tcpobex://", "irdaobex://", "file://",
		"urn:epc:id:", "urn:epc:tag", "urn:epc:pat:", "urn:epc:raw:",
		"urn:epc:", "urn:nfc:" };

void RecordURI::set_record_header() {
	mRecordHeader.set_FNT(RecordHeader::NFC_well_known);
	mRecordHeader.set_type_length(sizeof(sNDEFUriIdCode));

}

RecordURI::RecordURI(knowUriId_t uriId, const std::string &uriContent) :
		mUriTypeId(uriId), mTypeString("") {
	set_content(uriContent);
	set_record_header();
}

RecordURI::RecordURI(const std::string &uriType, const std::string &uriContent) :
		mContent(uriContent),mUriTypeId(UNKNOWN), mTypeString(uriType)  {
	update_record_header();
	set_record_header();
}

uint16_t RecordURI::write(uint8_t *buffer) {
	uint16_t offset = 0;
	update_content();

	offset += mRecordHeader.write_header(buffer);

	buffer[offset++] = sNDEFUriIdCode;
	buffer[offset++] = (uint8_t) mUriTypeId;

	if (mUriTypeId == UNKNOWN) {
		std::memcpy(buffer + offset, mTypeString.c_str(), mTypeString.size());
		offset += mTypeString.size();
	} //if

	std::memcpy(buffer + offset, mContent.c_str(), mContent.size());
	offset += mContent.size();
	return offset;
}

RecordURI* RecordURI::parse(const RecordHeader &header,
		const uint8_t *buffer) {
	uint16_t offset = 0;

	if (buffer[offset++] != sNDEFUriIdCode)
		return NULL;
	knowUriId_t uriType = (knowUriId_t) buffer[offset++];
	//it is a standard type handle by a specific class
	if (uriType == MAIL) {
		return RecordMail::parse(header, buffer);
	} //if

	//is an standard type without a specific class
	if (uriType != UNKNOWN) {
		return new RecordURI(uriType,
				std::string((const char*) buffer + offset,
						header.get_payload_length() - 1));
	} //else

	//is an unknown type with a specific class
	RecordURI *r = RecordSMS::parse(header, buffer);
	if (r != NULL)
		return r;
	r = RecordGeo::parse(header, buffer);
	if (r != NULL)
		return r;
	//else is an unknown type without a specific class
	return new RecordURI(uriType,
			std::string((const char*) buffer + offset,
					header.get_payload_length() - 1));

}

} /* namespace NDefLib */
