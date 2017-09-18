/**
 ******************************************************************************
 * @file    Message.cpp
 * @author  ST / Central Labs
 * @version V2.0.0
 * @date    28 Apr 2017
 * @brief   NDef Message class implementation
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

#include <cstdlib>
#include "Message.h"
#include "EmptyRecord.h"
#include "RecordText.h"
#include "RecordAAR.h"
#include "RecordMimeType.h"
#include "RecordURI.h"

namespace NDefLib {

uint16_t Message::get_byte_length() const {
	uint16_t lenght = 2; //length size

	if (mRecords.size() == 0)
		return lenght + EmptyRecord().get_byte_length();

	std::vector<Record*>::const_iterator it = mRecords.begin();
	const std::vector<Record*>::const_iterator end = mRecords.end();

	for (; it != end; ++it) {
		lenght += (*it)->get_byte_length();
	} //for

	return lenght;
} //getByteLenght

uint16_t Message::write(uint8_t *buffer) const {

	const uint16_t length = get_byte_length() - 2;
	uint16_t offset = 0;
	buffer[offset++] = (uint8_t) ((length & 0xFF00) >> 8);
	buffer[offset++] = (uint8_t) ((length & 0x00FF));

	const uint32_t nRecord = mRecords.size();

	if (mRecords.size() == 0) {
		offset += EmptyRecord().write(buffer + offset);
		return offset;
	} //else

	for (uint32_t i = 0; i < nRecord; i++) {
		Record *r = mRecords[i];

		r->set_as_middle_record();
		if (i == 0)
			r->set_as_first_record();
		if (i == nRecord - 1)
			r->set_as_last_record();

		offset += r->write(buffer + offset);
	} //for

	return offset;
} //write

void Message::parse_message(const uint8_t * const rawNdefFile,
		const uint16_t length, Message *msg) {
	uint16_t offset = 0;
	Record *r;

	RecordHeader header;
	do {
		const uint8_t headerLenght = header.load_header(rawNdefFile + offset);
		r = RecordText::parse(header, rawNdefFile + offset + headerLenght);
		if (r == NULL)
			r = RecordAAR::parse(header, rawNdefFile + offset + headerLenght);
		if (r == NULL)
			r = RecordMimeType::parse(header,
					rawNdefFile + offset + headerLenght);
		if (r == NULL)
			r = RecordURI::parse(header, rawNdefFile + offset + headerLenght);

		offset += header.get_record_length();
		msg->add_record(r);
	} while (offset < length);

}

void Message::remove_and_delete_all_record(Message &msg){
    const uint32_t nRecords =msg.get_N_records();
	for(uint32_t i =0 ;i<nRecords ;i++){
        NDefLib::Record *r = msg[i];
        delete r;
    }//for
    msg.mRecords.clear();
}//removeAndDeleteAllRecord


} /* namespace NDefLib */
