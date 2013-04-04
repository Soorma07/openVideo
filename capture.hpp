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

            struct v4l2_buffer v4lBuffer;

            std::string     dev_name;
            int             fileDescriptor;
            buffer          *buffers;
            unsigned int    numBuffers;
            int             outBuffer;
            int             forceFormat;
            int             frameCount;

            void    errno_exit( const std::string &message );
            int     xioctl( int fileDescriptor, int request, void *arg );
            void    processImage( const void *outStream, int size );
            int     readFrame( void );
            void    uninitDevice( void );
            void    initRead( unsigned int bufferSize ); //Why wouldn't bufferSize be a data member?
            void    init_mmap( void );
            void    initUserPtr( unsigned int bufferSize ); //Same question.
            void    initDevice( void );
            void    closeDevice( void );
            void    openDevice( void );

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
