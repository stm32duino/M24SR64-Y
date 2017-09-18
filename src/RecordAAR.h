/**
 ******************************************************************************
 * @file    RecordAAR.h
 * @author  ST / Central Labs
 * @version V2.0.0
 * @date    28 Apr 2017
 * @brief   Create a Record that can start an application in an Android mobile.
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

#ifndef NDEFLIB_RECORDTYPE_RECORDAAR_H_
#define NDEFLIB_RECORDTYPE_RECORDAAR_H_

#include <string>

#include "Record.h"

namespace NDefLib {

/**
 * Create a Record that can start an application in an Android mobile.
 * @par The package data are copied inside the class.
 */
class RecordAAR: public Record {
public:

	/**
	 * Create an RecordAAR reading the data from the buffer.
 	 * @param header Record header.
	 * @param buffer Buffer to read the data from.
	 * @return an object of type recordAAR or NULL
	 * @par User is in charge of freeing the pointer returned by this function.
	 */
	static RecordAAR* parse(const RecordHeader &header,
			const uint8_t * const buffer);

	/**
	 * Build a new record.
	 * @param packageName Package of the application to start
	 */
	explicit RecordAAR(const std::string &packageName);

    /**
     * Get the record type.
     * @return TYPE_AAR
	 */
	virtual RecordType_t get_type() const {
		return TYPE_AAR;
	} //getType

	/**
	 * Get the package inside this record
	 * @return get the package inside this record
	 */
	const std::string& get_package() const {
		return mPackageName;
	}

	/**
	 * Change the package name of this record.
	 * @param package new package
	 */
	void set_package(const std::string& package){
		mPackageName=package;
		mRecordHeader.set_payload_length(mPackageName.size());
	}

	virtual uint16_t write(uint8_t *buffer);
	virtual ~RecordAAR() { };

	/**
	 * compare two objects
	 * @return true if the records have the same package name
	 */
	bool operator==(const RecordAAR &other) const{
		return 	(mPackageName==other.mPackageName);
	}

private:
	/**
	 * Application to start
	 */
	std::string mPackageName;

	/**
	 * String to use as record type for this record
	 */
	static const char sRecordType[];
};

} /* namespace NDefLib */

#endif /* NDEFLIB_RECORDTYPE_RECORDAAR_H_ */
