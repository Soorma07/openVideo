/** capture.hpp **/
#ifndef __V4LCAPTURE__
#define __V4LCAPTURE__

#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <getopt.h>             /* getopt_long() */
#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <cerrno>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

namespace v4lCapture{

#define CLEAR(x) memset(&(x), 0, sizeof(x))

    class capture
    {

        private:
            struct buffer {
                void    *start;
                size_t  length;
            };
            buffer          *buffers;
            unsigned int    bufferSize;
            std::string     dev_name;
            int             frameCount;
            int             fileDescriptor;
            int             forceFormat;
            unsigned int    numBuffers;
            int             outBuffer;
            struct v4l2_buffer v4lBuffer;

            void    closeDevice( void );
            void    errno_exit( const std::string &message );
            void    initDevice( void );
            void    init_mmap( void );
            void    initRead( );
            void    initUserPtr( );
            void    openDevice( void );
            void    processImage( const void *outStream, int size );
            int     readFrame( void );
            void    uninitDevice( void );
            int     xioctl( int fileDescriptor, int request, void *arg );

        public:
            capture();
            enum io_method {
                IO_METHOD_READ,
                IO_METHOD_MMAP,
                IO_METHOD_USERPTR
            }io;

            void startCapture( void );
            void stopCapture( void );

    };
};
#endif
