/** capture.cpp **/

#include "capture.hpp"

using namespace v4lCapture;

capture::capture(){}

void capture::closeDevice( void )
{
    if( -1 == close( fileDescriptor ) )
            errno_exit("close");

    fileDescriptor = -1;
}

void capture::errno_exit( const std::string &message )
{
    fprintf( stderr, "%s error %d, %s\n", message.c_str(), errno, strerror( errno ) );
    exit( EXIT_FAILURE );
}

void capture::initDevice( void )
{
    struct v4l2_capability cap;
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
    struct v4l2_format fmt;
    unsigned int min;

    if( -1 == xioctl( fileDescriptor, VIDIOC_QUERYCAP, &cap ) )
    {
        if( EINVAL == errno )
        {
            fprintf( stderr, "%s is no V4L2 device\n", dev_name.c_str() );
            exit(EXIT_FAILURE);
        }
        else
        {
            errno_exit("VIDIOC_QUERYCAP");
        }
    }

    if( !( cap.capabilities & V4L2_CAP_VIDEO_CAPTURE ) )
    {
        fprintf( stderr, "%s is no video capture device\n", dev_name.c_str() );
        exit(EXIT_FAILURE);
    }

    switch( io )
    {
        case IO_METHOD_READ:
            if( !( cap.capabilities & V4L2_CAP_READWRITE ) )
            {
                fprintf( stderr, "%s does not support read i/o\n", dev_name.c_str() );
                exit(EXIT_FAILURE);
            }
            break;
        case IO_METHOD_MMAP:
        case IO_METHOD_USERPTR:
            if( !( cap.capabilities & V4L2_CAP_STREAMING ) )
            {
                fprintf( stderr, "%s does not support streaming i/o\n", dev_name.c_str() );
                exit(EXIT_FAILURE);
            }
            break;
    }

    CLEAR(cropcap);

    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if( 0 == xioctl( fileDescriptor, VIDIOC_CROPCAP, &cropcap ) )
    {
        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        crop.c = cropcap.defrect; /** reset to default **/

        if( -1 == xioctl( fileDescriptor, VIDIOC_S_CROP, &crop ) )
        {
            switch( errno )
            {
                case EINVAL:
                    /** Cropping not supported **/
                    break;
                default:
                    /** Errors ignored **/
                    break;
            }
        }
        else
        {
            /** Errors ignored. **/
        }
    }

    CLEAR(fmt);

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if( forceFormat )
    {
        fmt.fmt.pix.width       = 640;
        fmt.fmt.pix.height      = 480;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
        fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

        if( -1 == xioctl( fileDescriptor, VIDIOC_S_FMT, &fmt ) )
            errno_exit("VIDIOC_S_FMT");

        /* Note VIDIOC_S_FMT may change width and height. */
    }
    else
    {
        /* Preserve original settings as set by v4l2-ctl for example */
        if( -1 == xioctl( fileDescriptor, VIDIOC_G_FMT, &fmt ) )
            errno_exit("VIDIOC_G_FMT");
    }

    /* Buggy driver paranoia. This came from the original, not sure it's important */
    min = fmt.fmt.pix.width * 2;
    if( fmt.fmt.pix.bytesperline < min )
        fmt.fmt.pix.bytesperline = min;
    min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
    if( fmt.fmt.pix.sizeimage < min )
        fmt.fmt.pix.sizeimage = min;

    switch( io )
    {
        case IO_METHOD_READ:
            bufferSize = fmt.fmt.pix.sizeimage;
            initRead( );
            break;

        case IO_METHOD_MMAP:
            init_mmap();
            break;

        case IO_METHOD_USERPTR:
            bufferSize = fmt.fmt.pix.sizeimage;
            initUserPtr( );
            break;
    }
}

void capture::init_mmap( void )
{
    struct v4l2_requestbuffers req;
    CLEAR(req);

    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if( -1 == xioctl( fileDescriptor, VIDIOC_REQBUFS, &req ) )
    {
        if( EINVAL == errno )
        {
            fprintf( stderr, "%s does not support memory mapping\n", dev_name.c_str() );
            exit( EXIT_FAILURE );
        }
        else
        {
            errno_exit("VIDIOC_REQBUFS");
        }
    }

    if( req.count < 2 )
    {
        fprintf( stderr, "Insufficient buffer memory on %s\n", dev_name.c_str() );
        exit(EXIT_FAILURE);
    }

    buffers = (buffer*)calloc( req.count, sizeof( *buffers ) );

    if( !buffers )
    {
        fprintf( stderr, "Out of memory\n" );
        exit(EXIT_FAILURE);
    }

    for( numBuffers = 0; numBuffers < req.count; ++numBuffers )
    {

        CLEAR(v4lBuffer);

        v4lBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        v4lBuffer.memory = V4L2_MEMORY_MMAP;
        v4lBuffer.index = numBuffers;

        if( -1 == xioctl( fileDescriptor, VIDIOC_QUERYBUF, &v4lBuffer ) )
            errno_exit("VIDIOC_QUERYBUF");

        buffers[ numBuffers ].length = v4lBuffer.length;
        buffers[ numBuffers ].start =
            mmap( NULL,
                  v4lBuffer.length,
                  PROT_READ | PROT_WRITE,
                  MAP_SHARED,
                  fileDescriptor, v4lBuffer.m.offset );

        if( MAP_FAILED == buffers[ numBuffers ].start )
            errno_exit("mmap");
    }
}

void capture::initRead( )
{
    buffers = (buffer*)calloc( 1, sizeof(*buffers) );

    if( !buffers )
    {
        fprintf( stderr, "Out of memory\n" );
        exit(EXIT_FAILURE);
    }

    buffers[0].length = bufferSize;
    buffers[0].start = malloc(bufferSize);

    if( !buffers[0].start)
    {
         fprintf(stderr, "Out of memory\n" );
        exit(EXIT_FAILURE);
    }
}

void capture::initUserPtr( )
{
    struct v4l2_requestbuffers req;

    CLEAR(req);

    req.count  = 4;
    req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_USERPTR;

    if( -1 == xioctl( fileDescriptor, VIDIOC_REQBUFS, &req ) )
    {
        if( EINVAL == errno )
        {
            fprintf( stderr, "%s does not support user pointer i/o\n", dev_name.c_str() );
            exit(EXIT_FAILURE);
        }
        else
        {
            errno_exit("VIDIOC_REQBUFS");
        }
    }

    buffers = (buffer*)calloc(4, sizeof(*buffers));

    if( !buffers )
    {
        fprintf( stderr, "Out of memory\n" );
        exit(EXIT_FAILURE);
    }

    for( numBuffers = 0; numBuffers < 4; ++numBuffers )
    {
        buffers[numBuffers].length = bufferSize;
        buffers[numBuffers].start = malloc(bufferSize);

        if( !buffers[numBuffers].start )
        {
            fprintf( stderr, "Out of memory\n" );
            exit(EXIT_FAILURE);
        }
    }
}

void capture::openDevice( void )
{
    struct stat st;

    if( -1 == stat( dev_name.c_str(), &st ) )
    {
        fprintf( stderr, "Cannot identify '%s': %d, %s\n", dev_name.c_str(), errno, strerror( errno ) );
        exit(EXIT_FAILURE);
    }

    if( !S_ISCHR( st.st_mode ) )
    {
        fprintf( stderr, "%s is no device\n", dev_name.c_str() );
        exit(EXIT_FAILURE);
    }

    fileDescriptor = open( dev_name.c_str(), O_RDWR /* required */ | O_NONBLOCK, 0 );

    if (-1 == fileDescriptor )
    {
        fprintf( stderr, "Cannot open '%s': %d, %s\n", dev_name.c_str(), errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void capture::processImage( const void *outStream, int size )
{
    if( outBuffer );
        fwrite( outStream, size, 1, stdout );

    fflush( stderr );
    fprintf( stderr, "." );
    fflush( stdout );
}

int  capture::readFrame( void )
{
    unsigned int i;

    switch( io )
    {
        case IO_METHOD_READ:
            if( -1 == read( fileDescriptor, buffers[0].start, buffers[0].length ) )
            {
                switch( errno )
                {
                    case EAGAIN:
                        return 0;
                    case EIO:
                        /** ignored **/
                    default:
                        errno_exit( "read" );
                }
            }
            processImage( buffers[0].start, buffers[0].length);
            break;
        case IO_METHOD_MMAP:
            CLEAR(v4lBuffer);

            v4lBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            v4lBuffer.memory = V4L2_MEMORY_MMAP;

            if( -1 == xioctl( fileDescriptor, VIDIOC_DQBUF, &v4lBuffer ) )
            {
                switch( errno )
                {
                    case EAGAIN:
                        return 0;
                    case EIO:
                    default:
                        errno_exit("VIDIOC_DQBUF");
                }
            }

            assert( v4lBuffer.index < numBuffers );

            processImage( buffers[v4lBuffer.index].start, v4lBuffer.bytesused );

            if( -1 == xioctl( fileDescriptor, VIDIOC_QBUF, &v4lBuffer ) )
                errno_exit( "VIDIOC_QBUF" );
            break;
        case IO_METHOD_USERPTR:
            CLEAR(v4lBuffer);

            v4lBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            v4lBuffer.memory = V4L2_MEMORY_USERPTR;

            if( -1 == xioctl( fileDescriptor, VIDIOC_DQBUF, &v4lBuffer ) )
            {
                switch( errno )
                {
                    case EAGAIN:
                        return 0;
                    case EIO:
                    default:
                        errno_exit("VIDIOC_DQBUF");
                }
            }
            for( i = 0; i < numBuffers; ++i )
            {
                if( v4lBuffer.m.userptr == (unsigned long)buffers[i].start &&
                        v4lBuffer.length == buffers[i].length )
                break;
            }
    }
    return 1;
}

void capture::startCapture( void )
{
    unsigned int i;
    enum v4l2_buf_type type;

    switch( io )
    {
        case IO_METHOD_READ:
            /** Nothing to do **/
            break;
        case IO_METHOD_MMAP:
            for( i = 0; i < numBuffers; ++i )
            {
                CLEAR(v4lBuffer);
                v4lBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                v4lBuffer.memory = V4L2_MEMORY_MMAP;
                v4lBuffer.index = i;

                if( -1 == xioctl( fileDescriptor, VIDIOC_QBUF, &v4lBuffer ) )
                        errno_exit("VIDIOC_QBUF");
            }
        case IO_METHOD_USERPTR:
            for( i = 0; i < numBuffers; ++i )
            {
                CLEAR(v4lBuffer);
                v4lBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                v4lBuffer.memory = V4L2_MEMORY_USERPTR;
                v4lBuffer.index = i;
                v4lBuffer.m.userptr = (unsigned long)buffers[i].start;
                v4lBuffer.length = buffers[i].length;

                if( -1 == xioctl( fileDescriptor, VIDIOC_QBUF, &v4lBuffer ) )
                    errno_exit("VIDIOC_QBUF");
            }
            type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            if( -1 == xioctl( fileDescriptor, VIDIOC_STREAMON, &type ) )
                errno_exit("VIDIOC_STREAMON");
            break;
    }

    unsigned int count;

    count = frameCount;

    while( count-- > 0 )
    {
        for(;;)
        {
            fd_set fds;
            struct timeval tv;
            int r;

            FD_ZERO(&fds);
            FD_SET(fileDescriptor, &fds);

            /** Timeout **/
            tv.tv_sec = 2;
            tv.tv_usec = 0;

            r = select( fileDescriptor + 1, &fds, NULL, NULL, &tv);

            if( -1 == r )
            {
                if( EINTR == errno )
                    continue;
                errno_exit("select");
            }

            if( 0 == r )
            {
                fprintf( stderr, "select timeout\n" );
                exit(EXIT_FAILURE);
            }

            if( readFrame() )
                break;
        }
    }
}


void capture::stopCapture( void )
{
    enum v4l2_buf_type type;

    switch ( io )
    {
        case IO_METHOD_READ:
            /** Nothing to do. **/
            break;
        case IO_METHOD_MMAP:
            /** Fall through. **/
        case IO_METHOD_USERPTR:
            type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            if( -1 == xioctl( fileDescriptor, VIDIOC_STREAMOFF, &type ))
                errno_exit( "VIDIOC_STREAMOFF" );
            break;
    }
}

void capture::uninitDevice( void )
{
    unsigned int i;

    switch( io )
    {
        case IO_METHOD_READ:
            free( buffers[0].start );
            break;
        case IO_METHOD_MMAP:
            for( i = 0; i < numBuffers; ++i )
            {
                if( -1 == munmap(buffers[i].start, buffers[i].length ) )
                    errno_exit("munmap");
            }
            break;
        case IO_METHOD_USERPTR:
            for( i = 0; i < numBuffers; ++i )
                free( buffers[i].start);
            break;
    }
    free( buffers );
}

int  capture::xioctl( int fileDescriptor, int request, void *arg )
{
    int r;
    do{
        r = ioctl( fileDescriptor, request, arg );
    } while ( -1 == r && EINTR == errno );
}

