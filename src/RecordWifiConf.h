/**
 ******************************************************************************
 * @file    RecordWifiConf.h
 * @author  ST / Central Labs
 * @version V2.0.0
 * @date    28 Apr 2017
 * @brief   {@link RecordMimeType} that contains a Wifi configuration data
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

#ifndef NDEFLIB_RECORDTYPE_RECORDWIFICONF_H_
#define NDEFLIB_RECORDTYPE_RECORDWIFICONF_H_

#include <RecordMimeType.h>
namespace NDefLib {

/**
 * Specialize the {@link RecordMimeType} to store Wifi configuration information.
 */
class RecordWifiConf: public NDefLib::RecordMimeType {
public:

	/**
	 * Authentication required by the wifi network
	 */
	typedef enum {
		AUTH_OPEN = 0x0000, //!< no authentication
		AUTH_WPA_PSK = 0x0002,
		AUTH_WPA_EAP = 0x0008,
		AUTH_WPA2_EAP = 0x0010,
		AUTH_WPA2_PSK = 0x0020,
	}authType_t;

	/**
	 * Encryption used by the network
	 */
	typedef enum {
		ENC_TYPE_NONE = 0x0001,     //!< no authentication
	    ENC_TYPE_WEP = 0x0002,      //!< wep is deprecated
	    ENC_TYPE_TKIP = 0x0004,     //!< deprecated -> only with mixed mode (0x000c)
	    ENC_TYPE_AES = 0x0008,      //!< includes CCMP and GCMP
	    ENC_TYPE_AES_TKIP = 0x000c, //!< mixed mode
	}encryptionType_t;

	/**
	 * Create a RecordWifiConf reading the data from the buffer.
 	 * @param header Record header.
	 * @param buffer Buffer to read the data from.
	 * @return an object of type RecordVCard or NULL
	 * @par User is in charge of freeing the pointer returned by this function.
	 */
	static RecordWifiConf* parse(const RecordHeader &header,
			const uint8_t* buffer);


	/**
	 * Create a record with the wifi configuration, the default parameters create an open network
	 * @param ssid network name
	 * @param pass network password
	 * @param authType authentication used in the network
	 * @param encType encryption used in the network
	 */
	RecordWifiConf(const std::string &ssid,const std::string &pass="",
			authType_t authType=AUTH_OPEN,encryptionType_t encType=ENC_TYPE_NONE);

    /**
     * Get the record type.
     * @return TYPE_WIFI_CONF
	 */
	virtual RecordType_t get_type() const {
		return TYPE_WIFI_CONF;
	} //getType

	/**
	 * @return update the record content and return the number of
	 * bytes needed to store this record
	 */
	virtual uint16_t get_byte_length() {
		update_mime_data();
		return RecordMimeType::get_byte_length();
	}

	/**
	 * Update the content and write it on the buffer.
	 * @param[out] buffer buffer to write the record content into.
	 * @return number of bytes written
	 * @see Record#write
	 */
	virtual uint16_t write(uint8_t *buffer){
		update_mime_data();
		return RecordMimeType::write(buffer);
	}

	/**
	 * Compare two objects.
	 * @return true if the records have the same Vcard information
	 */
	bool operator==(const RecordWifiConf &other){
		return (mSsid==other.mSsid) &&
			   (mPasskey==other.mPasskey) &&
			   (mAuthType==other.mAuthType) &&
			   (mEncType==other.mEncType);
	}


	const std::string& get_network_key() const{
		return mPasskey;
	}

	void set_network_key(const std::string &newKey){
		mPasskey=newKey;
		mContentIsChange=true;
	}

	const std::string& get_network_ssid() const{
		return mSsid;
	}

	void set_network_ssid(const std::string &newSsid){
		mSsid=newSsid;
		mContentIsChange=true;
	}

	authType_t get_auth_type() const{
		return mAuthType;
	}

	void set_auth_type(const authType_t &newAuth){
		mAuthType = newAuth;
		mContentIsChange=true;
	}

	encryptionType_t get_encryption() const{
		return mEncType;
	}

	void set_encryption_type(const encryptionType_t &newEncript){
		mEncType =  newEncript;
		mContentIsChange=true;
	}

	virtual ~RecordWifiConf(){
		if(mMimeData!=NULL)
			delete [] mMimeData;
	}

private:

	/**
	 * function that update the ndef data format
	 */
	void update_mime_data();

	typedef uint16_t fieldType_t; //< type to use for store a field type
	typedef uint16_t fieldLenght_t; //<type to use for store a filed length

	/**
	 * write a data field into the output buffer
	 * @param dataType field type
	 * @param data field content
	 * @param buffer buffer where write the data field
	 * @return number of write bytes
	 */
	template<typename T>
	uint32_t write_data_field(const fieldType_t& dataType,
			const T& data, uint8_t *buffer);

	/**
	 * write a data buffer inside the output buffer
	 * @param dataType field type
	 * @param data buffer where read the data
	 * @param size number of byte to move
	 * @param out output buffer
	 * @return number of write bytes
	 */
	uint32_t write_data_field(const fieldType_t& dataType,
			const char *data,const fieldLenght_t& size, uint8_t *buffer);

	std::string mSsid;
	std::string mPasskey;
	authType_t mAuthType;
	encryptionType_t mEncType;

	bool mContentIsChange; ///< true if we have to update the string representation of the data
	uint8_t *mMimeData; //< buffer where store the record playload
	fieldLenght_t mMimeDataLenght;


	static const std::string sWifiConfMimeType;
	static const fieldType_t sCredentialField_id;
	static const fieldType_t sNetworkIdField_id;
	static const uint8_t sDefaultNetworkId;
	static const fieldType_t sSsidField_id;
	static const fieldType_t sNetworkKeyField_id;
	static const fieldType_t sAuthTypeField_id;
	static const fieldType_t sEncTypeField_id;
};
}

#endif /* NDEFLIB_RECORDTYPE_RECORDWIFICONF_H_ */

