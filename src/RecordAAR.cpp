/**
 ******************************************************************************
 * @file    RecordAAR.cpp
 * @author  ST / Central Labs
 * @version V2.0.0
 * @date    28 Apr 2017
 * @brief   RecordAAR implementation.
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
#include "RecordAAR.h"

namespace NDefLib {

const char RecordAAR::sRecordType[] = { 'a', 'n', 'd', 'r', 'o', 'i', 'd', '.',
		'c', 'o', 'm', ':', 'p', 'k', 'g' };

RecordAAR::RecordAAR(const std::string &packageName) :
		mPackageName(packageName) {
	mRecordHeader.set_FNT(RecordHeader::NFC_external);
	mRecordHeader.set_type_length(sizeof(sRecordType));
	mRecordHeader.set_payload_length(mPackageName.size());

}

uint16_t RecordAAR::write(uint8_t *buffer) {
	uint16_t offset = 0;
	offset += mRecordHeader.write_header(buffer);
	std::memcpy(buffer + offset, sRecordType, sizeof(sRecordType));
	offset += sizeof(sRecordType);
	std::memcpy(buffer + offset, mPackageName.c_str(), mPackageName.size());
	offset += mPackageName.size();
	return offset;
}

RecordAAR* RecordAAR::parse(const RecordHeader &header,
		const uint8_t *buffer) {
	uint8_t offset = 0;
	if ((header.get_FNT() != RecordHeader::NFC_external)
			&& (header.get_type_length() != sizeof(sRecordType))) {
		return NULL;
	} //else

	if (std::memcmp(buffer + offset, sRecordType, sizeof(sRecordType)) != 0) {
		return NULL;
	} //else tagType == android.com:pkg

	offset += sizeof(sRecordType);

	return new RecordAAR(
			std::string((const char*) buffer + offset,
					header.get_payload_length()));
}

} /* namespace NDefLib */
