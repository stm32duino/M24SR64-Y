/**
 ******************************************************************************
 * @file    RecordGEO.cpp
 * @author  ST / Central Labs
 * @version V2.0.0
 * @date    28 Apr 2017
 * @brief   RecordGEO implementation.
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
#include <cstdio>

#include "RecordGeo.h"

namespace NDefLib {

const std::string RecordGeo::sGeoTag = ("geo:");

//number of digits used for print the coordintate
#define N_FLOAT_DIGIS 4
#define N_INTEGER_DIGIS 3
//string with the 2 coordinate format
#define COORDINATE_WRITE_FORMAT "%3.4f,%3.4f"
//+1 = sign, +1 = .
#define COORDINATE_STR_SIZE (1+N_INTEGER_DIGIS+1+N_FLOAT_DIGIS)

//string to use for read a couple of coordinate
#define COORDINATE_READ_FORMAT "%f,%f"


RecordGeo::RecordGeo(const float lat, const float lon) :
		RecordURI(sGeoTag), mLatitiude(lat), mLongitude(lon),mContentIsChange(true) {
};

void RecordGeo::update_content(){
	if(!mContentIsChange)
		return;

	//2 coordinate +1 for the separator +1 for the \0
	char buffer[2*COORDINATE_STR_SIZE+1+1];
	std::sprintf(buffer,COORDINATE_WRITE_FORMAT,mLatitiude,mLongitude);
	mContent.clear();
	mContent.append(buffer);

	mContentIsChange=false;
}

RecordGeo* RecordGeo::parse(const RecordHeader &header,
		const uint8_t * const buffer) {
	uint16_t offset = 0;
	if (buffer[offset++] != RecordURI::sNDEFUriIdCode)
		return NULL;
	if (buffer[offset++] != RecordURI::UNKNOWN)
		return NULL;
	if (sGeoTag.compare(0, sGeoTag.size(), (const char*) buffer + offset,
			sGeoTag.size()) != 0)
		return NULL;
	offset += sGeoTag.size();

	const std::string uriContent((const char*) (buffer + offset),
			header.get_payload_length() - offset);

	float lat,lon;
	//build the record only if both the coordinate are available
	if(std::sscanf(uriContent.c_str(),COORDINATE_READ_FORMAT,&lat,&lon)!=2)
		return NULL;
	return new RecordGeo(lat,lon);

}

} /* namespace NDefLib */
