/**
 ******************************************************************************
 * @file    RecordSMS.cpp
 * @author  ST / Central Labs
 * @version V2.0.0
 * @date    28 Apr 2017
 * @brief   RecordSMS implementation
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

#include "RecordSMS.h"

namespace NDefLib {

const std::string RecordSMS::sSmsTag("sms:");
const std::string RecordSMS::sBodyTag("?body=");

RecordSMS::RecordSMS(const std::string &number, const std::string &message) :
		RecordURI(sSmsTag), mNumber(number), mMsg(message),mContentIsChange(true){
}

void RecordSMS::update_content(){

	if(!mContentIsChange)
		return;

	mContent.reserve(mNumber.size()+sBodyTag.size()+mMsg.size());

	mContent  = mNumber;
	mContent +=sBodyTag;
	mContent +=mMsg;

	mContentIsChange=false;
}


RecordSMS* RecordSMS::parse(const RecordHeader &header,
		const uint8_t *buffer) {
	uint16_t offset = 0;
	if (buffer[offset++] != RecordURI::sNDEFUriIdCode)
		return NULL;
	if (buffer[offset++] != RecordURI::UNKNOWN)
		return NULL;
	if (sSmsTag.compare(0, sSmsTag.size(), (const char*) buffer + offset,
			sSmsTag.size()) != 0)
		return NULL;
	offset += sSmsTag.size();

	const std::string uriContent((const char*) (buffer + offset),
			header.get_payload_length() - offset+1);

	std::size_t numberEnd = uriContent.find(sBodyTag);
	if (numberEnd == std::string::npos)
		return NULL;

	return new RecordSMS(uriContent.substr(0, numberEnd),
			uriContent.substr(numberEnd + sBodyTag.size()));

}

} /* namespace NDefLib */
