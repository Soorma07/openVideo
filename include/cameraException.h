#ifndef __CAMERA_EXCEPTION
#define __CAMERA_EXCEPTION

#include <stdexcept>
#include <string>
#include <cstring>
#include <sstream>

namespace bob {
    static const int bobit = 1;
    class cameraException: public std::runtime_error
    {
        private:
            int cameraNumber;
            char* deviceName;
            std::stringstream helper;

        public:
            cameraException():
                std::runtime_error( "Camera error" ){};

            cameraException( char* deviceName, int cameraNumber ):
                std::runtime_error( this->helper.str() )
                    { messageBuild( deviceName, cameraNumber ); };

            cameraException( const cameraException& other ):
                std::runtime_error( other.helper.str() ){};

            virtual ~cameraException() throw(){};

            inline const char* messageBuild( char* deviceName = "",
                                            int cameraNumber = 0 )
            {
                this->cameraNumber = cameraNumber;
                this->deviceName = deviceName;
                this->helper << "Camera error: "
                             << "Camera number: " << this->cameraNumber
                             << "Device name: " << this->deviceName;
                return this->helper.str().c_str();
            };

    };
};
#endif //__CAMERA_EXCEPTION
