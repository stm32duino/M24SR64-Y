/**
 ******************************************************************************
 * @file    RecordMAIL.cpp
 * @author  ST / Central Labs
 * @version V2.0.0
 * @date    28 Apr 2017
 * @brief   RecordMail implementation.
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
#include "RecordMail.h"

namespace NDefLib {

const std::string RecordMail::sSubjectTag("?subject=");
const std::string RecordMail::sBodyTag("&body=");

void RecordMail::update_content(){
    if(!mContentIsChange)
        return;

    mContent.reserve(mToAddress.size() + sSubjectTag.size() + mSubject.size() + sBodyTag.size() + mBody.size());
    mContent = mToAddress;
    mContent += sSubjectTag;
    mContent += mSubject;
    mContent += sBodyTag;
    mContent += mBody;

    mContentIsChange=false;
}


RecordMail* RecordMail::parse(const RecordHeader &header,
        const uint8_t* buffer) {
    //not a uri tag or a mail tag
    if (buffer[0] != RecordURI::sNDEFUriIdCode
            || buffer[1] != RecordURI::MAIL) {
        return NULL;
    }
    const std::string uriContent((const char*) (buffer + 2),
            header.get_payload_length() - 1);

    std::size_t subjectStart = uriContent.find(sSubjectTag);
    if (subjectStart == std::string::npos) //subject not present
        return NULL;
    const std::size_t destSize = subjectStart;
    subjectStart += sSubjectTag.size(); //skip the tag string

    std::size_t bodyStart = uriContent.find(sBodyTag);
    if (bodyStart == std::string::npos) //body not present
        return NULL;
    const std::size_t subjectLenght = bodyStart - subjectStart;
    bodyStart += sBodyTag.size(); // skip the tag string

    const std::size_t bodyLenght = uriContent.size() - bodyStart;

    return new RecordMail(uriContent.substr(0, destSize),
            uriContent.substr(subjectStart, subjectLenght),
            uriContent.substr(bodyStart, bodyLenght));

}

} /* namespace NDefLib */
