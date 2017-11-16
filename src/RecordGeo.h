/**
 ******************************************************************************
 * @file    RecordGEO.h
 * @author  ST / Central Labs
 * @version V2.0.0
 * @date    28 Apr 2017
 * @brief   Specialize the @{link RecordURI} to handle a location position, the location
 *	 is stored with latitude and longitude.
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

#ifndef NDEFLIB_RECORDTYPE_RECORDGEO_H_
#define NDEFLIB_RECORDTYPE_RECORDGEO_H_

#include "RecordURI.h"

namespace NDefLib {

/**
 * Specialize the {@link RecordUri} to handle a location position, the location
 * is stored with latitude and longitude.
 * @par The coordinates are floats with 4 decimal positions.
 */
class RecordGeo: public RecordURI {
public:

	/**
	 * Create an RecordGeo reading the data from the buffer.
 	 * @param header Record header.
	 * @param buffer Buffer to read the data from.
	 * @return an object of type RecordGeo or NULL
	 * @par User is in charge of freeing the pointer returned by this function.
	 */
	static RecordGeo* parse(const RecordHeader &header,
			const uint8_t * const buffer);


	/**
	 * Create a record that contains a gps coordinate.
	 * @param lat Latitude.
	 * @param lon Longitude.
	 */
	RecordGeo(const float lat, const float lon);

	virtual ~RecordGeo() { };

	/**
	 * Return the longitude coordinate
	 * @return the longitude coordinate
	 */
	float get_longitude()const {
		return mLongitude;
	}

	/**
	 * Return the latitude coordinate
	 * @return the latitude coordinate
	 */
	float get_latitude()const {
		return mLatitiude;
	}

	/**
	 * Change the latitude coordinate.
	 * @param lat new latitude.
	 */
	void set_latitude(const float lat){
		mContentIsChange=true;
		mLatitiude=lat;
	}

    /**
	 * Change the longitude coordinate.
	 * @param lon new longitude
	 */
	void set_longitude(const float lon){
		mContentIsChange=true;
		mLongitude=lon;
	}

    /**
     * Get the record type.
     * @return TYPE_URI_GEOLOCATION
	 */
	virtual RecordType_t get_type() const {
		return TYPE_URI_GEOLOCATION;
	} //getType

    /**
     * Compare two RecordGeo tags.
     * @return true if the two tags have the same latitude and longitude
     */
	bool operator==(const RecordGeo &other) const {
		return 	(mLatitiude == other.mLatitiude) &&
				(mLongitude == other.mLongitude) ;
	}

protected:

	/**
	 * This function updates the uri message to write into the tag.
	 */
	virtual void update_content();

private:

    /**
     * Latitude information.
     */
	float mLatitiude;

	/**
	 * Longitude information.
	 */
	float mLongitude;

    /**
     * True if the user has updated the latitude or longitude.
     */
	bool mContentIsChange;

	/**
	 * Uri type to use for this record.
	 */
	static const std::string sGeoTag;
};

} /* namespace NDefLib */

#endif /* NDEFLIB_RECORDTYPE_RECORDGEO_H_ */
