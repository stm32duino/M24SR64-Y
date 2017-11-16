/**
 ******************************************************************************
 * @file    EmptyRecord.h
 * @author  ST / Central Labs
 * @version V2.0.0
 * @date    28 Apr 2017
 * @brief   Implement an Empyt Record.
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

#ifndef NDEFLIB_RECORDTYPE_EMPTYRECORD_H_
#define NDEFLIB_RECORDTYPE_EMPTYRECORD_H_

#include "Record.h"

namespace NDefLib {

/**
 * Define an empty Record.
 */
class EmptyRecord: public Record {
public:

	EmptyRecord() {
		mRecordHeader.set_FNT(RecordHeader::Empty);
		mRecordHeader.set_MB(true);
		mRecordHeader.set_ME(true);
		mRecordHeader.set_type_length(0);
		mRecordHeader.set_payload_length(0);
	}

	/**
	 * Write the 3 bytes used to define an empty record.
	 * @param[out] buffer Buffer to write the record into.
	 * @return number of write bytes
	 */
	virtual uint16_t write(uint8_t *buffer) {
		return mRecordHeader.write_header(buffer);
	} //write

	virtual ~EmptyRecord() { }
};

} /* namespace NDefLib */

#endif /* NDEFLIB_RECORDTYPE_EMPTYRECORD_H_ */
