/*
 * cameraException.h
 *
 *  Created on: Feb 27, 2013
 *      Author: RJ Linton
 */

#ifndef CAMERAEXCEPTION_H_
#define CAMERAEXCEPTION_H_

#include <stdexcept>
#include <string>
#include <cstring>
#include <sstream>

namespace libV4l2Capture {

    class cameraException: public std::runtime_error
    {
        private:
            int cameraNumber;
            char* deviceName;
            std::stringstream helper;

            inline const char* messageBuild( )
            {
                this->helper << "Camera error: "
                             << "Camera number: " << this->cameraNumber
                             << "Device name: " << this->deviceName;
                return this->helper.str().c_str();
            };

        public:
            cameraException() : cameraNumber(0), deviceName("0")
        	{
            	messageBuild( );
                std::runtime_error( this->helper.str() );
        	};

            cameraException( char* deviceName, int cameraNumber ) :
            	cameraNumber( cameraNumber ),deviceName( deviceName )
            {
            	messageBuild( );
            	std::runtime_error( this->helper.str() );
           	};

            cameraException( const cameraException& other ) :
            	cameraNumber( other.cameraNumber ), deviceName( other.deviceName ),
                	std::runtime_error( other.helper.str() ){};

            virtual ~cameraException() throw(){};


    }; /* cameraException */

}; /* libV4l2Capture */

#endif /* CAMERAEXCEPTION_H_ */
