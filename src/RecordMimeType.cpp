/**
 ******************************************************************************
 * @file    RecordMimeType.cpp
 * @author  ST / Central Labs
 * @version V2.0.0
 * @date    28 Apr 2017
 * @brief   RecordMimeType implementation
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
#include "RecordMimeType.h"
#include "RecordVCard.h"
#include "RecordWifiConf.h"

namespace NDefLib {

void RecordMimeType::initialize_header_data() {
	mRecordHeader.set_FNT(RecordHeader::Mime_media_type);
	mRecordHeader.set_type_length(mMimeType.size());
	mRecordHeader.set_payload_length(mDataLength);
}

RecordMimeType::RecordMimeType(const std::string &mimeType, const uint8_t *data,
		uint32_t nDataLenght) :
		mMimeType(mimeType), mDataLength(nDataLenght),mData(NULL),mDataToFree(false) {

	if(data!=NULL && nDataLenght!=0){
		mDataToFree=true;
		mData = new uint8_t[nDataLenght];
		std::memcpy(mData,data,nDataLenght);
	}

	initialize_header_data();
}

RecordMimeType::RecordMimeType(const std::string &mimeType, const std::string &data) :
		mMimeType(mimeType), mDataLength(data.size()),mData(new uint8_t[mDataLength]),mDataToFree(true) {

	std::memcpy(mData,data.c_str(),mDataLength);

	initialize_header_data();
}

uint16_t RecordMimeType::write(uint8_t *buffer) {
	uint16_t offset = 0;
	offset += mRecordHeader.write_header(buffer);
	std::memcpy(buffer + offset, mMimeType.c_str(), mMimeType.size());
	offset += mMimeType.size();
	std::memcpy(buffer + offset, mData, mDataLength);
	offset += mDataLength;
	return offset;
}

RecordMimeType* RecordMimeType::parse(const RecordHeader &header,
		const uint8_t* buffer) {
	uint32_t offset = 0;
	if (header.get_FNT() != RecordHeader::Mime_media_type) {
		return NULL;
	} //else

	//check if it is a know subtype
	RecordMimeType *r = RecordVCard::parse(header, buffer);
	if (r != NULL)
		return r;

	r = RecordWifiConf::parse(header, buffer);
	if (r != NULL)
		return r;


	//build a generic mameType
	uint32_t dataOffset = offset + header.get_type_length()+header.get_id_length();
	return new RecordMimeType(
			std::string((const char*) buffer + offset, header.get_type_length()),
			buffer + dataOffset, header.get_payload_length());
}

void RecordMimeType::copy_mime_data(const uint8_t* data, uint32_t dataLength){
		delete_mime_data();
		mData = new uint8_t[dataLength];
		mDataLength=dataLength;
		std::memcpy(mData,data,dataLength);
		mRecordHeader.set_payload_length(dataLength);
		mDataToFree=true;
	}

} /* namespace NDefLib */
