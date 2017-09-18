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
#include <algorithm>

#include <RecordWifiConf.h>
namespace NDefLib {
const std::string RecordWifiConf::sWifiConfMimeType("application/vnd.wfa.wsc");

const RecordWifiConf::fieldType_t RecordWifiConf::sCredentialField_id = 0x100E;
const RecordWifiConf::fieldType_t RecordWifiConf::sNetworkIdField_id = 0x1026;
const uint8_t  RecordWifiConf::sDefaultNetworkId=0x01;
const RecordWifiConf::fieldType_t RecordWifiConf::sSsidField_id = 0x1045;
const RecordWifiConf::fieldType_t RecordWifiConf::sNetworkKeyField_id = 0x1027;
const RecordWifiConf::fieldType_t RecordWifiConf::sAuthTypeField_id = 0x1003;
const RecordWifiConf::fieldType_t RecordWifiConf::sEncTypeField_id = 0x100F;

RecordWifiConf::RecordWifiConf(const std::string &ssid,const std::string &pass,
		authType_t authType,encryptionType_t encType):
				RecordMimeType(sWifiConfMimeType),
				mSsid(ssid),
				mPasskey(pass),
				mAuthType(authType),
				mEncType(encType),
				mContentIsChange(true),
				mMimeData(NULL),
				mMimeDataLenght(0){}

/**
 * move the data into the buffer swapping the byte order
 * @param data data to write into the buffer
 * @param outBuffer buffer where write the data
*/
template<typename T>
void writeBEFromLE(const T& data,uint8_t *outBuffer){
	uint8_t *buffer = (uint8_t*)&data;
	std::reverse_copy(buffer,buffer+sizeof(T),outBuffer);
}

template<typename T>
uint32_t RecordWifiConf::write_data_field(const fieldType_t& dataType,const T& data,uint8_t  *buffer){
	uint32_t offset=0;
	writeBEFromLE(dataType,buffer+offset);
	offset+=sizeof(dataType);
	const fieldLenght_t dataSize = sizeof(T);
	writeBEFromLE(dataSize,buffer+offset);
	offset+=sizeof(dataSize);
	writeBEFromLE(data,buffer+offset);
	return offset+dataSize;
}

uint32_t RecordWifiConf::write_data_field(const fieldType_t& dataType,
		const char *data,const fieldLenght_t& size, uint8_t *buffer){
	uint32_t offset=0;
	writeBEFromLE(dataType,buffer+offset);
	offset+=sizeof(dataType);;
	writeBEFromLE(size,buffer+offset);
	offset+=sizeof(size);
	if(data!=NULL || size!=0)
		std::memcpy(buffer+offset,data,size);
	return offset+size;
}

void RecordWifiConf::update_mime_data(){
	if(!mContentIsChange)
		return;

	mMimeDataLenght=sizeof(sCredentialField_id)+sizeof(fieldLenght_t)+
			sizeof(sNetworkIdField_id)+sizeof(fieldLenght_t)+sizeof(sDefaultNetworkId)+
			sizeof(sSsidField_id)+sizeof(fieldLenght_t)+mSsid.length()+
			sizeof(sNetworkKeyField_id)+sizeof(fieldLenght_t)+mPasskey.length()+
			sizeof(sAuthTypeField_id)+sizeof(fieldLenght_t)+2+
			sizeof(sEncTypeField_id)+sizeof(fieldLenght_t)+2;

	if(mMimeData!=NULL)
		delete [] mMimeData;

	mMimeData = new uint8_t[mMimeDataLenght];

	uint32_t writeOffset=0;
	writeBEFromLE(sCredentialField_id,mMimeData+writeOffset);
	writeOffset+=sizeof(sCredentialField_id);
	writeBEFromLE((fieldLenght_t)(mMimeDataLenght-4),mMimeData+writeOffset);
	writeOffset+=sizeof(fieldLenght_t);

	writeOffset += write_data_field(sNetworkIdField_id,sDefaultNetworkId,mMimeData+writeOffset);
	fieldLenght_t tempLength = (fieldLenght_t)sizeof(std::string::value_type)*mSsid.length();
	writeOffset += write_data_field(sSsidField_id,mSsid.data(),tempLength,mMimeData+writeOffset);
	writeOffset += write_data_field(sAuthTypeField_id,(int16_t)mAuthType,mMimeData+writeOffset);
	writeOffset += write_data_field(sEncTypeField_id,(int16_t)mEncType,mMimeData+writeOffset);
	tempLength = (fieldLenght_t)sizeof(std::string::value_type)*mPasskey.length();
	writeOffset += write_data_field(sNetworkKeyField_id,mPasskey.data(),tempLength,mMimeData+writeOffset);

	set_mime_data_pointer(mMimeData,mMimeDataLenght);
	mContentIsChange=false;
}

/**
 * Read a data from a buffer, where the buffer is in little endian and we are in
 * a big endian system
 * @param buffer buffer where read the data
 * @param out pointer to the variable where store the read data
 */
template<typename T>
void readLEFromBE(const uint8_t *const buffer,T* out){
	const uint32_t typeSize = sizeof(T);
	uint8_t *outBuffer = (uint8_t*)out;
	std::reverse_copy(buffer,buffer+typeSize,outBuffer);
}

/**
 * Create a RecordWifiConf reading the data from the buffer.
	 * @param header Record header.
 * @param buffer Buffer to read the data from.
 * @return an object of type RecordVCard or NULL
 * @par User is in charge of freeing the pointer returned by this function.
 */
RecordWifiConf* RecordWifiConf::parse(const RecordHeader &header,
		const uint8_t* buffer){
	if (header.get_FNT() != RecordHeader::Mime_media_type
				|| header.get_type_length() != sWifiConfMimeType.size()) {
			return NULL;
	}

	if (sWifiConfMimeType.compare(0, sWifiConfMimeType.size(), (const char*) buffer,
			sWifiConfMimeType.size()) != 0) {
		return NULL;
	}

	buffer += header.get_type_length();
	buffer += header.get_id_length();

	fieldType_t type;
	fieldLenght_t dataLength;

	readLEFromBE(buffer,&type);
	buffer+=sizeof(fieldType_t);
	readLEFromBE(buffer,&dataLength);
	buffer+=sizeof(fieldLenght_t);

	if(type!=sCredentialField_id)
		return NULL;

	std::string ssid;
	std::string pass;
	authType_t authType=AUTH_OPEN;
	encryptionType_t encType=ENC_TYPE_NONE;
	uint16_t enumValue;
	fieldLenght_t readData=0;
	while(readData!=dataLength){
		readLEFromBE(buffer+readData,&type);
		readData+=sizeof(fieldType_t);
		fieldLenght_t length;
		readLEFromBE(buffer+readData,&length);
		readData+=sizeof(fieldLenght_t);

		switch(type){
			case sSsidField_id:
				ssid.insert(0,(const char*)buffer+readData,length);
				break;
			case sNetworkKeyField_id:
				pass.insert(0,(const char*)buffer+readData,length);
				break;
			case sAuthTypeField_id:
				readLEFromBE(buffer+readData,&enumValue);
				authType = (authType_t)enumValue;
				break;
			case sEncTypeField_id:
				readLEFromBE(buffer+readData,&enumValue);
				encType = (encryptionType_t)enumValue;
				break;
		}

		readData+=length;

	}

	return new RecordWifiConf(ssid,pass,authType,encType);
}//parse

}

