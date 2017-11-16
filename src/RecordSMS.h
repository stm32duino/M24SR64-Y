/**
 ******************************************************************************
 * @file    RecordSMS.h
 * @author  ST / Central Labs
 * @version V2.0.0
 * @date    28 Apr 2017
 * @brief   Extend the {@link RecordUri} to handle the SMS content
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

#ifndef NDEFLIB_RECORDTYPE_RECORDSMS_H_
#define NDEFLIB_RECORDTYPE_RECORDSMS_H_

#include <string>

#include "RecordURI.h"

namespace NDefLib {

/**
 * Extend the {@link RecordUri} to handle the SMS content.
 */
class RecordSMS: public RecordURI {
public:

	/**
	 * Read a recordSMS.
	 * @param header Record header.
	 * @param buffer Buffer to read the data from.
	 * @return a RecordSMS type or NULL if it was not possible to build this record
	 * @par User is in charge of freeing the pointer returned by this function.
	 */
	static RecordSMS* parse(const RecordHeader &header,
			const uint8_t *buffer);

	/**
	 * Build a sms record.
	 * @param number Sms recipient phone number.
	 * @param message Message to send.
	 * @par Strings are copied inside the class.
	 */
	RecordSMS(const std::string &number, const std::string &message);

    /**
     * Get the record type.
     * @return TYPE_URI_SMS
	 */
	virtual RecordType_t get_type() const {
		return TYPE_URI_SMS;
	} //getType

	/**
	 * Get recipient number.
	 * @return recipient number
	 */
	const std::string& get_number() const {
		return mNumber;
	}

    /**
     * Get SMS text message.
     * @return SMS text message
     */
	const std::string& get_messagge() const {
		return mMsg;
	}

    /**
     * Change the recipent number.
     * @param number New recipent number.
     */
	void set_number(const std::string &number){
		mNumber=number;
		mContentIsChange=true;
	}

	/**
	 * Change the message content.
	 * @param message New message.
	 */
	void set_message(const std::string &message){
		mMsg=message;
		mContentIsChange=true;
	}

	/**
	 * Compare 2 objects.
	 * @return true if the records have the same message and number.
	 */
	bool operator==(const RecordSMS &other) const {
		return (mMsg == other.mMsg) &&
				(mNumber == other.mNumber);
	}

	virtual ~RecordSMS() {
	};

protected:

	/**
     * Generate the URI content that encodes the sms.
     * @see RecordUri#updateContent
     */
	virtual void update_content();

private:
	std::string mNumber; ///< Recipient number.
	std::string mMsg; ///< Sms text.

	/**
	 * This variable is true when we need to update the URI content,
	 * it is an optimization to avoid to rebuild multiple times the URI content when
	 * data has not changed
	 */
	bool mContentIsChange;

	/**
	 * String used as URI type.
	 */
	static const std::string sSmsTag;

	/**
	 * String used to separate the number and body data.
	 */
	static const std::string sBodyTag;
};

} /* namespace NDefLib */

#endif /* NDEFLIB_RECORDTYPE_RECORDSMS_H_ */
