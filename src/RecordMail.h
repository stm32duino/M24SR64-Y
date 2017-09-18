/**
 ******************************************************************************
 * @file    RecordMail.h
 * @author  ST / Central Labs
 * @version V2.0.0
 * @date    28 Apr 2017
 * @brief   Class that specializes the {@link RecordUri} to store a mail content.
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
#ifndef NDEFLIB_RECORDTYPE_RECORDMAIL_H_s
#define NDEFLIB_RECORDTYPE_RECORDMAIL_H_

#include "RecordURI.h"

namespace NDefLib {

/**
 * Class that specializes the {@link RecordUri} to store a mail content.
 */
class RecordMail: public RecordURI {

public:

	/**
	 * Create a RecordMail reading the data from the buffer.
 	 * @param header Record header.
	 * @param buffer Buffer to read the data from.
	 * @return an object of type RecordMail or NULL
	 * @par User is in charge of freeing the pointer returned by this function.
	 */
	static RecordMail* parse(const RecordHeader &header,
			const uint8_t* buffer);

	/**
	 * Create a mail.
	 * @param toAddress Mail recipient.
	 * @param subject Mail subject.
	 * @param msg Message.
	 */
	RecordMail(const std::string &toAddress, const std::string &subject,
			const std::string &msg) :
			RecordURI(RecordURI::MAIL), mToAddress(toAddress), mSubject(
					subject), mBody(msg),mContentIsChange(true) { }	;

	virtual ~RecordMail() {	};

    /**
     * @return the mail recipient address
     */
	const std::string& get_to_address() const {
		return mToAddress;
	}

    /**
     * @return the mail subject
     */
	const std::string& get_subject() const {
		return mSubject;
	}

	/**
	  * @return the mail body
	  */
	const std::string& get_body() const {
		return mBody;
	}

    /**
     * Change the mail recipient.
     * @param dest Recipient address.
     * @par The string is copied inside the class.
     */
	void set_to_address(const std::string& dest){
		mContentIsChange=true;
		mToAddress=dest;
	}

    /**
     * Change the mail subject.
     * @param subj New mail subject.
     * @par The string is copied inside the class.
     */
	void set_subject(const std::string& subj) {
		mContentIsChange=true;
		mSubject=subj;
	}

    /**
     * Change the mail body.
     * @param body New mail body.
     * @par The string is copied inside the class.
     */
	void set_body(const std::string& body) {
		mContentIsChange=true;
		mBody=body;
	}

    /**
     * Get the record type.
     * @return TYPE_URI_MAIL
	 */
	virtual RecordType_t get_type() const {
		return TYPE_URI_MAIL;
	} //getType

    /**
     * Compare 2 RecordMails.
     *Â @return true if both the tags have the same addresses,subjects and bodies
     */
	bool operator==(const RecordMail& other)const{
		return mToAddress == other.mToAddress &&
				mSubject == other.mSubject &&
				mBody == other.mBody;
	}

protected:

    /**
     * Generate the uri content that encodes the mail content.
     * @see RecordUri#updateContent
     */
	virtual void update_content();

private:

	std::string mToAddress; ///< Mail address.
	std::string mSubject; ///< Mail subject.
	std::string mBody; ///< Mail body.
	bool mContentIsChange; ///< True if the use update some mail field.

	static const std::string sSubjectTag; ///< String to use bofore the subject.
	static const std::string sBodyTag; ///< String to use before the mail body.
};

} /* namespace NDefLib */

#endif /* NDEFLIB_RECORDTYPE_RECORDMAIL_H_ */
