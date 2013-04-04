#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <exception>
#include <getopt.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include "libConfig.h"


//OpenCV is required for this version.

class cameraException: public std::exception
{
    private:
        int cameraNumber;
    public:
    inline int getCameraNumber() { return cameraNumber; };
    inline void setCameraNumber( int cameraNum )
                                { cameraNumber = cameraNum; };
    virtual const char* what() const throw()
    {
        return "Camera failed to open: ";
    }
}camExcept;

int main( int argc, char** argv )
{
    //Setup stderr.
    std::ofstream outputFileStream;
    outputFileStream.open( "./openVideoError.log",
                            std::ios::out | std::ios::app | std::ios::ate );
    std::streambuf* errorBuffer = outputFileStream.rdbuf();
    std::streambuf* save_cerr_buffer;
    save_cerr_buffer = std::cerr.rdbuf();
    std::cerr.rdbuf( errorBuffer );

    //Constants for camera image size
    const int cameraWidth = 352;
    const int cameraHeight = 288;
    //Setup matrix to hold frame data.
    cv::Mat rightFrame;
    cv::Mat leftFrame;

    //Variable camera number
    int leftCamNum = 2;  //These a magic defaults that work when my system udevs the v4l2 driver.
    int rightCamNum = 1; //These are changed with command line args.

    //Setup variables for command line options
    int option_index = 0;
    int optchr;

    while( 1 ) //Infinite loop, mind termination conditions.
    {
        static struct option long_options[ 6 ] =
        {
            { "debug", 0, 0, 0 },
            { "version", 0, 0, 'v' },
            { "help", 0, 0, 'h' },
            { "leftCamera", required_argument, 0, 'l' },
            { "rightCamera", required_argument, 0, 'r' },
            { 0, 0, 0, 0 }
        };

        optchr = getopt_long( argc, argv, "hHvVrRlL",
                                long_options, &option_index );
        if( optchr == -1 )
            break;

        switch( optchr )
        {
            case 'l':
            case 'L':
                if( strcmp( long_options[ option_index ].name, "leftCamera" ) == 0 )
                {
                    leftCamNum = std::stoi( std::string( optarg ) );
                }
                if( leftCamNum < 0 || leftCamNum > 9 ) //Maximum of 10 options.
                    leftCamNum = 0; //Hoping there is at least a 0 camera.
                std::cout << "Found -L option with argument: " << optarg << std::endl;
                break;
            case 'r':
            case 'R':
                if( strcmp( long_options[ option_index ].name, "rightCamera" ) == 0 )
                {
                    rightCamNum = std::stoi( std::string( optarg ) );
                }
                if( rightCamNum < 0 || rightCamNum > 9 ) //Maximum of 10 options.
                    rightCamNum = 0; //Hoping there is at least a 0 camera.
                std::cout << "Found -R option with argument: " << optarg << std::endl;
                break;
            case 'v':
            case 'V':
                std::cout << "Version: " << libV4l2Capture_VERSION_MAJOR
                          << "." << libV4l2Capture_VERSION_MINOR  << std::endl;
               return 0;
            case 'h':
            case 'H':
               std::cout << "Usage:" << std::endl;
               std::cout << argv[ 0 ]
                            << " --version (-v) Return program version."
                            << std::endl
                            << argv[ 0 ]
                            << " --help (-h) Display this help."
                            << std::endl
                            << argv[ 0 ]
                            << " --leftCamera (-l) Select the left camera."
                            << std::endl
                            << argv[ 0 ]
                            << " --rightCamera (-r) Select the right camera."
                            << std::endl;
                return 0;
        }//end the switch.
    }//end the while.

    //Open the cameras
    cv::VideoCapture captureLeft( leftCamNum );
    cv::VideoCapture captureRight( rightCamNum );
    try
    {
        if( !captureLeft.isOpened() )
        {
            camExcept.setCameraNumber( leftCamNum );
            throw camExcept;
        }
        captureLeft.set( CV_CAP_PROP_FRAME_WIDTH, cameraWidth );
        captureLeft.set( CV_CAP_PROP_FRAME_HEIGHT, cameraHeight );
        if( !captureRight.isOpened( ) )
        {
            camExcept.setCameraNumber( rightCamNum );
            throw camExcept;
        }
        captureRight.set( CV_CAP_PROP_FRAME_WIDTH, cameraWidth );
        captureRight.set( CV_CAP_PROP_FRAME_HEIGHT, cameraHeight );
    }
    catch( cameraException e )
    {
        std::cerr << e.what() << e.getCameraNumber() << std::endl;
        std::cerr << "There may be more errors. " << std::endl;
        //Add code here to release cameras and try again.
    }

    //Setup variables to display fame rate.
    int fontFace = cv::FONT_HERSHEY_PLAIN;
    double fontScale = 1;
    int thickness = 1;
    cv::Point textOrigin( ( leftFrame.cols + 10 ),
                          ( leftFrame.rows + 30 ) ); //Making a guess
    std::ostringstream str_stm;
    str_stm.precision( 6 );
    std::string fpsString;

    //Setup variables to keep track of time.
    double t = (double)cv::getTickCount();
    double fps = 0;
    double delay = 1000.0 / 30;

    //Create a window, display the video, and wait for a key stroke.
    cv::namedWindow( "Open a video device", CV_WINDOW_AUTOSIZE );

    //Create palate for binocular image
    captureLeft >> leftFrame;
    captureRight >> rightFrame;

    cv::Mat binocularImage( cameraHeight, 2 * cameraWidth, leftFrame.type() );
    cv::Mat leftHalf( binocularImage, cv::Range( 0, cameraHeight ),
                        cv::Range( 0, cameraWidth ) );
    cv::Mat rightHalf( binocularImage, cv::Range( 0, cameraHeight ),
                        cv::Range( cameraWidth, 2 * cameraWidth ) );

    //Main loop, any key to exit.
    for(;;)
    {
        //Capture from each camera
        captureLeft >> leftFrame;
        captureRight >> rightFrame;

        //Calculate elapsed time and adjust units.
        t = ( (double)cv::getTickCount() - t )
              / cv::getTickFrequency();

        //Calculate frame rate
        fps = 1.0 / t;
        str_stm << "FPS: " << fps;
        fpsString = str_stm.str();
        str_stm.str(std::string());

        //Put the frame rate on the image.
        cv::putText( leftFrame, fpsString, textOrigin, fontFace,
                     fontScale, cv::Scalar::all(255), thickness, 8 );

        //Copy each camera image to the proper ROI.
        leftFrame.copyTo( leftHalf );
        rightFrame.copyTo( rightHalf );

        //Show the image in the window.
        cv::imshow( "Open a video device", binocularImage );

        //Get current time, and ensure new data for each iteration.
        t = (double)cv::getTickCount();
        if( cv::waitKey( delay ) >= 0 )
            break;
    }

    //Clean up the data structures.
    cv::destroyWindow( "Open a video device" );
    captureLeft.release();
    captureRight.release();
    binocularImage.release();

    //Put standard error back so that it can be called by the OS.
    std::cerr.rdbuf( save_cerr_buffer );
    outputFileStream.close();

    return 0;
}
