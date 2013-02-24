#ifndef __CAMERA_EXCEPTION
#define __CAMERA_EXCEPTION

#include <exception>

class cameraException: public std::exception
{
    private:
        int cameraNumber;

    public:
        inline int getCameraNumber() { return cameraNumber; };
        inline void setCameraNumber( int cameraNum )
            { cameraNumber = cameraNum; };
        virtual const char* what() const throw()
            { return "Camera failed to open: "; };
};

#endif //__CAMERA_EXCEPTION
