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
#include <sstream>

namespace libV4l2Capture {

    class cameraException: public std::runtime_error
    {
        private:
            int cameraNumber;
            std::string deviceName;
            std::stringstream helper;

            inline void messageBuild( )
            {
                this->helper << "Camera error: " << extendedError.str()
                             << " Camera number: " << this->cameraNumber
                             << " Device name: " << this->deviceName;
             };

        public:

            std::stringstream extendedError;

            cameraException() : cameraNumber(0), deviceName( NULL ),
            	std::runtime_error( "" )
        	{
            	messageBuild( );
        	};

            cameraException( std::string deviceName, int cameraNumber ) :
            	cameraNumber( cameraNumber ),deviceName( deviceName ),
            		std::runtime_error( "" )
            {
            	messageBuild( );
           	};

            cameraException( char* deviceName, int cameraNumber ) :
            	cameraNumber( cameraNumber ),deviceName( deviceName ),
            		std::runtime_error( "" )
            {
            	messageBuild( );
           	};

            cameraException( const cameraException& other ) :
            	cameraNumber( other.cameraNumber ), deviceName( other.deviceName ),
                		std::runtime_error( "" ){ this->messageBuild(); };

            virtual ~cameraException() throw(){};

            const char* what() const throw(){ return helper.str().c_str(); };

    }; /* cameraException */

}; /* libV4l2Capture */

#endif /* CAMERAEXCEPTION_H_ */
