/*
* Copyright I3D Robotics Ltd, 2020
* Author: Josh Veitch-Michaelis, Ben Knight (bknight@i3drobotics.com)
*/

#ifndef ABSTRACTSTEREOCAMERA_H
#define ABSTRACTSTEREOCAMERA_H

//#define _USE_MATH_DEFINE

#include <abstractstereomatcher.h>

#include<memory>

#include <opencv2/opencv.hpp>
#ifdef WITH_CUDA
#include <opencv2/cudastereo.hpp>
#include <opencv2/cudawarping.hpp>
#endif

#include "cvsupport.hpp"
#include "pclsupport.hpp"

// Point Cloud Library
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/io/ply_io.h>
#include <pcl/filters/passthrough.h>
#include <pcl/visualization/pcl_visualizer.h>

#include <QCoreApplication>
#include <QObject>
#include <QThread>
#include <QMessageBox>
#include <QtConcurrent/QtConcurrent>
#include <QDir>
#include <QDebug>
#include <QFile>
#include <QProgressDialog>
#include <QMutex>

//!  Stereo camera base class
/*!
  An abstract class to process stereo images from arbitrary cameras. This class should not be used directly,
  instead you should subclass it (e.g. StereoCameraDeimos()). The bare minimum is to implment a capture function
  that will capture a stereo pair from your camera.

  Listed below are the required functions to impliment in the subclass.
  public:
    - listSystems
  public slots:
    - captureSingle
    - openCamera
    - enableCapture
    - closeCamera
    - enableAutoExposure
    - setExposure
    - enableAutoGain
    - setGain
    - setBinning
    - enableTrigger
    - setFPS
    - setPacketSize

   See the function definition for details on implimentation.
*/

class AbstractStereoCamera : public QObject {
    Q_OBJECT

public:

    //! Enum defined errors. This defines the type of error that occured.
    enum StereoCameraError { CONNECT_ERROR, CAPTURE_ERROR, LOST_FRAMES_ERROR, RECTIFY_ERROR, MATCH_ERROR };
    //! Enum defined camera type. This defines the type of camera being used.
    enum StereoCameraType { CAMERA_TYPE_TARA, CAMERA_TYPE_TIS, CAMERA_TYPE_VIMBA, CAMERA_TYPE_USB,
                            CAMERA_TYPE_BASLER_GIGE, CAMERA_TYPE_BASLER_USB,
                            CAMERA_TYPE_PHOBOS_BASLER_GIGE, CAMERA_TYPE_PHOBOS_BASLER_USB, CAMERA_TYPE_PHOBOS_TIS_USB,
                            CAMERA_TYPE_DEIMOS,
                            CAMERA_TYPE_TITANIA_VIMBA_USB, CAMERA_TYPE_TITANIA_BASLER_GIGE, CAMERA_TYPE_TITANIA_BASLER_USB,
                            CAMERA_TYPE_INVALID };

    static std::string CAMERA_NAME_TARA,CAMERA_NAME_TIS,CAMERA_NAME_VIMBA;
    static std::string CAMERA_NAME_USB,CAMERA_NAME_BASLER_GIGE,CAMERA_NAME_BASLER_USB;
    static std::string CAMERA_NAME_DEIMOS,CAMERA_NAME_PHOBOS_BASLER_GIGE,CAMERA_NAME_PHOBOS_BASLER_USB,CAMERA_NAME_PHOBOS_TIS_USB;
    static std::string CAMERA_NAME_TITANIA_BASLER_GIGE, CAMERA_NAME_TITANIA_BASLER_USB, CAMERA_NAME_TITANIA_VIMBA_USB;
    static std::string CAMERA_NAME_INVALID;

    enum StereoCalibrationType { CALIBRATION_TYPE_YAML, CALIBRATION_TYPE_XML };

    enum PointCloudTexture { POINT_CLOUD_TEXTURE_IMAGE, POINT_CLOUD_TEXTURE_DEPTH };

    enum VideoSource { VIDEO_SRC_STEREO_RG, VIDEO_SRC_STEREO_CONCAT, VIDEO_SRC_LEFT, VIDEO_SRC_RIGHT, VIDEO_SRC_DISPARITY, VIDEO_SRC_RGBD };

    //! Structure to hold camera settings
    struct StereoCameraSettings {
        double exposure;
        int gain;
        int fps;
        int trigger;
        int hdr;
        int binning;
        int autoExpose;
        int autoGain;
        int isGige;
        int packetDelay;
        int packetSize;
        bool flip_left_x;
        bool flip_left_y;
        bool flip_right_x;
        bool flip_right_y;
    };

    //! Structure to hold stereo camera information
    struct StereoCameraSerialInfo {
        std::string left_camera_serial;
        std::string right_camera_serial;
        StereoCameraType camera_type; // type of camera
        std::string i3dr_serial; // defined i3dr serial for camera pair
    };

    explicit AbstractStereoCamera(StereoCameraSerialInfo serial_info,
                                  StereoCameraSettings camera_settings,
                                  QObject *parent = 0);

    ~AbstractStereoCamera(void);

    AbstractStereoMatcher *matcher = nullptr;
    AbstractStereoMatcher *new_matcher = nullptr;

    //! Convert between stereo camera type enum and string
    static std::string StereoCameraType2String(StereoCameraType camera_type);
    //! Convert between string and camera type enum
    static StereoCameraType String2StereoCameraType(std::string camera_type);

    StereoCameraSerialInfo getCameraSerialInfo(){return stereoCameraSerialInfo_;}

    StereoCameraSettings getCameraSettings(){return stereoCameraSettings_;}

    //! List avaiable stereo camera systems
    /*! This function should be implmented by a particular camera driver to only return camera of that type */
    static std::vector<StereoCameraSerialInfo> listSystems(void) {
        std::vector<StereoCameraSerialInfo> empty_list;
        return empty_list;
    };

    //! Load known camera serials
    /*!
    * @param[in] camera_type Camera type of serials to read (usb/basler/tis)
    * @param[in] filename Camera serials parameter file (default is: camera_serials.ini)
    */
    static std::vector<StereoCameraSerialInfo> loadSerials(StereoCameraType camera_type, std::string filename=qApp->applicationDirPath().toStdString() + "/camera_serials.ini");

    //! Assign the stereo camera object to a thread so as not to block the GUI. Typically called just after instantiation.
    void assignThread(QThread *thread);

    void stopThread();

    //! Returns weather camera image capture is enabled
    bool isCapturing();

    //! Returns whether matching is enabled
    bool isMatching();

    //! Returns whether rectification is being performed
    bool isRectifying();

    //! Returns wheather video saving is being performed
    bool isCapturingVideo() { return capturing_video; };

    //! Returns wheather video saving rectified or non recitifed images
    bool isCapturingRectifiedVideo() { return capturing_rectified_video; };

    //! Returns whether left and right images are being swapped
    bool isSwappingLeftRight();

    //! Returns whether downsampling is also being applied to calibration
    bool isDownsamplingCalibration();

    //! Returns wheather to save disparity along with stereo pair
    bool isSavingDisparity();

    //! Returns wheather the camera is connected
    bool isConnected();

    void getQ(cv::Mat &Q);

    //! Get the left stereo image (un-rectified)
    /*!
  * @param[out] dst OpenCV matrix to store image into
  */
    void getLeftRawImage(cv::Mat &dst);

    //! Get the left stereo image (un-rectified)
    /*!
  * @param[out] dst OpenCV matrix to store image into
  */
    void getRightRawImage(cv::Mat &dst);

    //! Get the left stereo image (un-rectified)
    /*!
  * @return OpenCV matrix containing left image
  */
    cv::Mat getLeftRawImage();

    //! Get the left stereo image (un-rectified)
    /*!
  * @return OpenCV matrix containing left image
  */
    cv::Mat getRightRawImage();

    //! Get the left stereo image
    /*!
  * @param[out] dst OpenCV matrix to store image into
  */
    void getLeftImage(cv::Mat &dst);

    //! Get the right stereo image
    /*!
  * @param[out] dst OpenCV matrix to store image into
  */
    void getRightImage(cv::Mat &dst);

    //! Get the left stereo image
    /*!
  * @return OpenCV matrix containing left image
  */
    cv::Mat getLeftImage();

    //! Get the right stereo image
    /*!
  * @return OpenCV matrix containing right image
  */
    cv::Mat getRightImage();

    //! Get the left stereo image used in latest stereo match
    /*!
  * @param[out] dst OpenCV matrix to store image into
  */
    void getLeftMatchImage(cv::Mat &dst);

    //! Get the right stereo image used in latest stereo match
    /*!
  * @param[out] dst OpenCV matrix to store image into
  */
    void getRightMatchImage(cv::Mat &dst);

    //! Get the disparity image
    /*!
  * @return OpenCV matrix containing disparity image
  */
    cv::Mat getDisparity();

    //! Get the disparity image
    /*!
  * @param[out] dst OpenCV matrix to store image into
  */
    void getDisparity(cv::Mat &dst);

    //! Get the disparity image
    /*!
  * @return OpenCV matrix containing disparity image
  */
    cv::Mat getDisparityFiltered();

    //! Get the disparity image
    /*!
  * @param[out] dst OpenCV matrix to store image into
  */
    void getDisparityFiltered(cv::Mat &dst);

    //! Get the depth image
    /*!
  * @return OpenCV matrix containing depth image
  */
    cv::Mat getDepth();

    //! Get the depth image
    /*!
  * @param[out] dst OpenCV matrix to store image into
  */
    void getDepth(cv::Mat &dst);


    //! Get a pointer to the current point cloud
    /*!
  * @return Pointer to the current point cloud
  */
    pcl::PointCloud<pcl::PointXYZRGB>::Ptr getPointCloud();

    //! Get the image width
    /*!
  * @return Image width
  */
    int getWidth(void){ return image_width; }

    //! Get the image height
    /*!
  * @return Image height
  */
    int getHeight(void){ return image_height; }

    //! Get the image bitdepth
    /*!
  * @return Image bit depth
  */
    int getBitDepth(void){ return image_bitdepth; }

    //! Get the image size
    /*!
  * @return Image size
  */
    cv::Size getSize(void){ return cv::Size(getWidth(),getHeight()); }

    void setPointCloudTexture(PointCloudTexture pct){
        this->pct = pct;
    }

    PointCloudTexture getPointCloudTexture(void){
        return this->pct;
    }

    //! Initalise video stream for writing a video stream to a file
    /*!
   * If no filename is supplied, a timestamped video will be stored in the current selected save folder.
   *
   * @param[in] filename The output filename
   * @param[in] fps frames per second
   * @param[in] codec video codec
   * @param[in] vid_src video source type (enum)
   * @return success
  */
    bool setVideoStreamParams(int fps = 0, VideoSource vid_src = VIDEO_SRC_STEREO_RG);
    bool addVideoStreamFrame(cv::Mat frame);

    bool connected = false;
    float downsample_factor = 1;

    cv::Mat left_remapped;
    cv::Mat right_remapped;
    cv::Mat left_unrectified;
    cv::Mat right_unrectified;
    cv::Mat left_match;
    cv::Mat right_match;
    cv::Mat left_raw;
    cv::Mat right_raw;
    cv::Mat Q;
    double fx;
    double baseline;

    cv::Mat stereo_reprojected;

signals:
    //! Emitted when stereo pair has been captured
    void captured();

    //! Emmited when captured successfully
    void captured_success();

    //! Emmited when capture failed
    void captured_fail();

    //! Emmited when first image stereo pair received
    void first_image_ready(bool ready);

    //! Emitted on error. Enum output for error type (See StereoCameraError enum for type of error)
    void error(int error_index);

    //! Emitted when stereo pair has been rectified
    void rectified();

    //! Emitted when an image has been matched
    void matched();

    //! Emitted when a disparity map has been reprojected to a point cloud
    void reprojected();

    //! Emitted when a stereo pair is processed
    void stereopair_processed();

    //! Emitted after a frame has been processed to indicate the process time (fps = 1000/frametime)
    void frametime(qint64 time);

    //! Emitted after a match has been processed to indicate the process time (fps = 1000/matchtime)
    void matchtime(qint64 time);

    //! Emitted after a reproject has been processed to indicate the process time (fps = 1000/reprojecttime)
    void reprojecttime(qint64 time);

    //! Emitted after a frame has been processed to indicates the current frame count
    void framecount(qint64 count);

    //! Emitted when the frame size of a camera changes
    void update_size(int width, int height, int bitdepth);

    //! Emitted when an image has been saved
    void savedImage();

    //! Emitted when an image has been saved, including the filename
    void savedImage(bool success);

    //! Emmitted when point cloud is saved
    void pointCloudSaveStatus(QString);

    //! Indicates that the camera has disconnected
    void disconnected();

    //! Indicates close of class for closing threads
    void finished();

public slots:

    //! Capture an image
    /*! Trigger the capture of a single image
    * @return True if an image was captured successfully, false otherwise
    * This is a virtual function which should be implmented by a particular camera driver */
    virtual bool captureSingle() = 0;

    //! Capture a frame in continuous capture mode
    /*! Trigger the capture of a frame in continous capture mode
    * @return True if an image was captured successfully, false otherwise
    * This is a virtual function which should be implmented by a particular camera driver */
    //virtual bool captureContinuous() = 0;

    //! Enable/disable camera image capture
    /*! Start and stop image capture from the camera (in thread)
     * @return Return success or failer of starting/stoping image capture
     * This is a virtual function which should be implmented by a particular camera driver */
    virtual bool enableCapture(bool enable) = 0;

    //! Open camera
    /*! Start connection to camera
     * This should only initalise the camera not start capturing. E.g. Start 3rd_party api's and setup default camera settings.
     * @return Return success or failer of closing the camera connection
     * This is a virtual function which should be implmented by a particular camera driver */
    virtual bool openCamera(void) = 0;

    //! Close camera
    /*! Safely stop capture and disconnect the camera
     * @return Return success or failer of closing the camera connection
     * This is a virtual function which should be implmented by a particular camera driver */
    virtual bool closeCamera(void) = 0;

    //! Enable/disable camera auto exposure
    /*! @return Return sucess or failer of changing the setting
     * This is a virtual function which should be implmented by a particular camera driver */
    virtual bool enableAutoExposure(bool) = 0;

    //! Adjust camera exposure
    /*! @return Return sucess or failer of changing the setting
     * This is a virtual function which should be implmented by a particular camera driver */
    virtual bool setExposure(double) = 0;

    //! Enable/disable camera auto gain
    /*! @return Return sucess or failer of changing the setting
     * This is a virtual function which should be implmented by a particular camera driver */
    virtual bool enableAutoGain(bool) = 0;

    //! Adjust camera gain
    /*! @return Return sucess or failer of changing the setting
     * This is a virtual function which should be implmented by a particular camera driver */
    virtual bool setGain(int) = 0;

    //! Adjust camera binning
    /*! @return Return sucess or failer of changing the setting
     * This is a virtual function which should be implmented by a particular camera driver */
    virtual bool setBinning(int) = 0;

    //! Enable/disable camera hardware trigger (disabled = software trigger)
    /*! @return Return sucess or failer of changing the setting
     * This is a virtual function which should be implmented by a particular camera driver */
    virtual bool enableTrigger(bool) = 0;

    //! Adjust camera FPS
    /*! @return Return sucess or failer of changing the setting
     * This is a virtual function which should be implmented by a particular camera driver */
    virtual bool setFPS(int) = 0;

    //! Adjust camera packet size (GigE cameras only but defined here for access)
    /*! @return Return sucess or failer of changing the setting
     * This is a virtual function which should be implmented by a particular camera driver */
    virtual bool setPacketSize(int) = 0;

    void setFileSaveDirectory(QString path){file_save_directory = path;};
    QString getFileSaveDirectory(){return file_save_directory;};

    bool hasTriggerFPSControl(){return has_trigger_fps_control;};

    bool startCapture(){return enableCapture(true);};
    bool stopCapture(){return enableCapture(false);};

    bool enableVideoStream(bool enable);
    bool setVideoSource(int source_index); // 0: stereo, 1: left, 2:right, 3:disparity
    bool startVideoStream(){return enableVideoStream(false);}
    bool stopVideoStream(){return enableVideoStream(false);}

    void setMatcher(AbstractStereoMatcher *matcher);

    //! Load calibration files from a directory and check them for validity
    /*!
    @param[in] directory The folder too check.
    @param[in] cal_type Calibration type (enum XML or YAML)
    @return True or false, depending on whether the parameters are valid.
    */
    bool loadCalibration(QString directory, StereoCalibrationType cal_type){
        if (directory == "") return false;
        bool res;
        if (cal_type == CALIBRATION_TYPE_XML){
            res = loadCalibrationXML(directory);
        } else if (cal_type == CALIBRATION_TYPE_YAML){
            res = loadCalibrationYaml(directory);
        } else {
            return false;
        }

        //TODO: scale calibration by downsample factor

        enableRectify(res);
        calibration_valid = res;

        return res;
    }

    bool loadCalibrationYaml(QString directory);

    bool loadCalibrationXML(QString directory);

    //! Save an image from the camera with a timestamped filename
    /*!
    *  The timestamp format is: yyyyMMdd_hhmmss_zzz (year, month, day, hour, minute, second, millisecond)
    *
    * @sa setSavelocation()
    */
    void saveImageTimestamped();

    //! Enable or disable stereo matching
    void enableMatching(bool match);

    //! Enable or disable image rectification
    void enableRectify(bool rectify);

    //! Enable or disable saving rectified images in video capture
    void enableCaptureRectifedVideo(bool rectify);

    //! Emable or disable disparity map reprojection to 3D
    void enableReproject(bool reproject);

    //! Enable swap left and right images
    void enableSwapLeftRight(bool swap);

    //! Enable applying downsampling to calibration as well as image
    //! this is done to avoid needed to re-calibration when using downsampling
    void enableDownsampleCalibration(bool enable);

    //! Enable saving disparity image
    void enableSaveDisparity(bool enable);

    //! Change downsample factor
    void setDownsampleFactor(int factor);

    //! Set the point cloud clipping distance closest to the camera (i.e. specify the closest distance to display)
    /*!
  * @param[in] zmin Distance to the camera in metres
  */
    void setVisualZmin(double zmin);

    //! Set the point cloud clipping distance furthest from the camera (i.e. specify the farthest distance to display)
    /*!
  * @param[in] zmax Distance to the camera in metres
  */
    void setVisualZmax(double zmax);

    //! Toggle saving date in filename
    void enableDateInFilename(bool enable);

    //! Save the current 3D reconstruction to a .PLY file
    void savePointCloud();

    //! Set the save directory
    /*!
  * @param dir Desired save directory, will attempt to create if it doesn't exist.
  */
    void setSavelocation(QString dir){

        if(!QDir(dir).exists()){
            auto saved = QDir(dir);
            saved.mkpath(".");
        }

        save_directory = dir;
    }

private slots:

    //! Grab and process a frame from the camera
    /*!
    * This will perform an rectification, matching and reprojection if enabled.
    * @sa enableRectify(), enableMatching(), enableReproject()
    * Should be triggered after a image capture event
    */
    void processStereo(void);

    void imageSaved(bool);

    void resetFailFrameCount(void){
        failed_frames = 0;
    }

    void processNewCapture(void);

    void processNewStereo(void);

    //void processNewMatch(void);

    void processMatch();

private:
    qint64 frames = 0;

    PointCloudTexture pct = POINT_CLOUD_TEXTURE_IMAGE;

    bool match_busy = false;
    bool processing_busy = false;
    bool reproject_busy = false;
    bool changed_matcher = false;

    bool includeDateInFilename = false;
    bool matching = false;
    bool rectifying = false;
    bool capturing_video = false;
    bool capturing_rectified_video = true;
    VideoSource video_src = VIDEO_SRC_STEREO_RG;
    QMutex video_mutex;
    QMutex disparity_mutex;
    QMutex lr_image_mutex;
    QMutex lr_raw_image_mutex;
    bool swappingLeftRight = false;
    bool reprojecting = false;
    bool cuda_device_found = false;
    bool downsamplingCalibration = true;
    bool savingDisparity = true;
    pcl::PointCloud<pcl::PointXYZRGB>::Ptr ptCloud;
    QString save_directory = ".";

    QString file_save_directory = "";

    QFuture<void> match_future;
    QFuture<void> stereo_future;
    QFuture<void> reproject_future;

    QFutureWatcher<void> match_futureWatcher;

    cv::VideoWriter *cv_video_writer = nullptr;

    //! Block until a steroe processing has finished
    void stereo_process_finished(void);

    //! Save an image
    /*!
  * @param[in] fname Output filename
  */
    void saveImage(QString fname);

    //!  Save the disparity map
    /*!
    * @param[in] filename Output filename
    */
    void saveDisparity(QString filename);

    //!  Save the disparity map as normalised colormap
    /*!
    * @param[in] filename Output filename
    */
    void saveDisparityColormap(QString filename);

    //!  Save the disparity map as normalised colormap
    /*!
    * @param[in] fname Output filename
    * @param[in] enable_16bit Save with 16 bit depth
    * @param[in] scale_16bit Scaling factor to apply to depth before converting to 16-bit to increase precision
    */
    void saveRGBD(QString fname, bool enable_16bit=true, float scale_16bit=6553.0);

    //! Load rectification maps from calibration files
    /*!
  * @param[in] src_l Left image rectification map file
  * @param[in] src_r Right image rectification map file
  * @return true/false whether the file was loaded successfully
  */
    bool loadXMLRectificationMaps(QString src_l, QString src_r);

    void generateRectificationMaps(cv::Size image_size);

    //! Load camera intrinsic/extinrisc calibration files
    /*!
    * @param[in] left_cal Left camera calibration parameter file
    * @param[in] right_cal Right camera calibration parameter file
    * @param[in] stereo_cal Stereo camera calibration parameter file
    * @return true/false whether the filse were loaded successfully
    */
    bool loadCalibrationXMLFiles(QString left_cal, QString right_cal,
                         QString stereo_cal);

    bool loadCalibrationYamlFiles(QString left_cal, QString right_cal);

    //! Rectify the current stereo image pair
    /*!
    * @param[in] left left image
    * @param[in] right right image
    * @param[out] left_rect rectified left image
    * @param[out] right_rect rectified right image
    * @return true/false whether the rectification was successful
    */
    bool rectifyImages(cv::Mat left, cv::Mat right, cv::Mat& left_rect, cv::Mat& right_rect);

    //! Wrapper around OpenCV rectify function for paralell calls.
    /*!
  * @param[in] src Input image
  * @param[out] dst Output image
  * @param[in] rmapx X rectification map
  * @param[in] rmapy Y rectification map
  */
    void remap_parallel(cv::Mat src, cv::Mat &dst, cv::Mat rmapx,
                        cv::Mat rmapy);

    cv::Mat rectmapx_l;
    cv::Mat rectmapy_l;
    cv::Mat rectmapx_r;
    cv::Mat rectmapy_r;
    cv::Mat r_camera_matrix;
    cv::Mat l_camera_matrix;
    cv::Mat l_dist_coeffs;
    cv::Mat r_dist_coeffs;
    cv::Mat r_rect_mat;
    cv::Mat l_rect_mat;
    cv::Mat r_proj_mat;
    cv::Mat l_proj_mat;

    int cal_image_width;
    int cal_image_height;

    int video_fps = 0;
    int video_codec = cv::VideoWriter::fourcc('M','J','P','G');
    std::string video_filename = "";
    cv::Mat video_frame;

    cv::Mat left_output;
    cv::Mat right_output;

    cv::Mat left_match_input;
    cv::Mat right_match_input;

    cv::Mat disparity;
    cv::Mat disparity_filtered;
    cv::Mat depth;

    double visualisation_min_z = 0.2;
    double visualisation_max_z = 5;

    //TODO replace with frame timeout timer
    int max_failed_frames = 5;
    int failed_frames = 0;

protected:

    int frame_rate = 30;
    int image_width = 0;
    int image_height = 0;
    int image_bitdepth = 1;

    StereoCameraSettings stereoCameraSettings_;
    StereoCameraSerialInfo stereoCameraSerialInfo_;

    QElapsedTimer frametimer;
    QElapsedTimer matchtimer;
    //QElapsedTimer reprojecttimer;

    bool rectification_valid = false;
    bool calibration_valid = false;
    bool captured_stereo = false;
    bool first_image_received = false;
    bool capturing = false;
    bool has_trigger_fps_control = false;

    QThread *thread_;

    //! Convert from enum to int for sending error signal
    void send_error(StereoCameraError stereo_error){
        if (stereo_error == CAPTURE_ERROR){
            failed_frames += 1;
            if (failed_frames > max_failed_frames){
                send_error(LOST_FRAMES_ERROR);
            }
        }
        int error_index = stereo_error;
        emit error(error_index);
    }
};

#endif  // ABSTRACTSTEREOCAMERA_H
