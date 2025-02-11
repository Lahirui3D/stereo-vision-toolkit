/*
* Copyright I3D Robotics Ltd, 2020
* Author: Ben Knight (bknight@i3drobotics.com)
*/

#include "stereocamerabasler.h"

//see: https://github.com/basler/pypylon/blob/master/samples/grabmultiplecameras.py

bool StereoCameraBasler::openCamera(){
    camControl = new ArduinoCommsCameraControl();
    std::vector<QSerialPortInfo> serial_device_list = camControl->getSerialDevices();
    camControl->open(serial_device_list.at(0),115200);

    int binning = stereoCameraSettings_.binning;
    bool trigger;
    if (stereoCameraSettings_.trigger == 1){
        trigger = true;
    } else {
        trigger = false;
    }
    int fps = stereoCameraSettings_.fps;;
    double exposure = stereoCameraSettings_.exposure;
    int gain = stereoCameraSettings_.gain;
    int packet_size = stereoCameraSettings_.packetSize;
    int packet_delay = stereoCameraSettings_.packetDelay;
    bool autoExpose;
    if (stereoCameraSettings_.autoExpose == 1){
        autoExpose = true;
    } else {
        autoExpose = false;
    }
    bool autoGain;
    if (stereoCameraSettings_.autoGain == 1){
        autoGain = true;
    } else {
        autoGain = false;
    }

    try
    {

        // Create an instant camera object with the camera device found first.
        Pylon::CTlFactory& tlFactory = Pylon::CTlFactory::GetInstance();

        Pylon::DeviceInfoList_t devices;
        tlFactory.EnumerateDevices(devices);

        this->cameras = new Pylon::CInstantCameraArray(2);
        Pylon::CInstantCameraArray all_cameras(devices.size());

        std::string camera_left_serial = stereoCameraSerialInfo_.left_camera_serial;
        std::string camera_right_serial = stereoCameraSerialInfo_.right_camera_serial;

        bool cameraLeftFind = false;
        for (size_t i = 0; i < all_cameras.GetSize(); ++i)
        {
            all_cameras[i].Attach(tlFactory.CreateDevice(devices[i]));

            if (all_cameras[i].GetDeviceInfo().GetSerialNumber() == camera_left_serial.c_str())
            {
                cameras->operator[](0).Attach(tlFactory.CreateDevice(devices[i]));
                cameraLeftFind = true;
                break;
            }
        }

        if (!cameraLeftFind){
            std::cerr << "Failed to find left camera with serial: " << camera_left_serial << std::endl;
            return false;
        }

        bool cameraRightFind = false;
        for (size_t i = 0; i < all_cameras.GetSize(); ++i)
        {
            all_cameras[i].Attach(tlFactory.CreateDevice(devices[i]));

            if (all_cameras[i].GetDeviceInfo().GetSerialNumber() == camera_right_serial.c_str())
            {
                cameras->operator[](1).Attach(tlFactory.CreateDevice(devices[i]));
                cameraRightFind = true;
                break;
            }
        }

        if (!cameraRightFind){
            std::cerr << "Failed to find right camera with serial: " << camera_right_serial << std::endl;
            return false;
        }

        getImageSize(cameras->operator[](0),image_width,image_height,image_bitdepth);
        emit update_size(image_width, image_height, image_bitdepth);

        for (size_t i = 0; i < cameras->GetSize(); ++i)
        {
            cameras->operator[](i).MaxNumBuffer = 1;
            cameras->operator[](i).OutputQueueSize = cameras->operator[](i).MaxNumBuffer.GetValue();
        }

        // Assign image event handlers
        leftImageEventHandler = new CPylonImageEventHandler;
        rightImageEventHandler = new CPylonImageEventHandler;
        cameras->operator[](0).RegisterImageEventHandler(leftImageEventHandler, Pylon::RegistrationMode_Append, Pylon::Cleanup_Delete);
        cameras->operator[](1).RegisterImageEventHandler(rightImageEventHandler, Pylon::RegistrationMode_Append, Pylon::Cleanup_Delete);

        formatConverter = new Pylon::CImageFormatConverter();

        formatConverter->OutputPixelFormat = Pylon::PixelType_Mono8;
        formatConverter->OutputBitAlignment = Pylon::OutputBitAlignment_MsbAligned;
    }
    catch (const Pylon::GenericException &e)
    {
        // Error handling.
        std::cerr << "An exception occurred whilst setting up cameras." << std::endl
                  << e.GetDescription() << std::endl;
        connected = false;
    }

    setBinning(binning);
    enableTrigger(trigger);
    setPacketSize(packet_size);
    setFPS(fps);

    enableAutoGain(autoGain);
    enableAutoExposure(autoExpose);
    setExposure(exposure);
    setGain(gain);
    setPacketDelay(packet_delay);

    connected = true;
    return connected;
}

std::vector<AbstractStereoCamera::StereoCameraSerialInfo> StereoCameraBasler::listSystems(void){
    Pylon::PylonInitialize();
    std::vector<AbstractStereoCamera::StereoCameraSerialInfo> known_serial_infos_gige = loadSerials(AbstractStereoCamera::CAMERA_TYPE_BASLER_GIGE);
    std::vector<AbstractStereoCamera::StereoCameraSerialInfo> known_serial_infos_usb = loadSerials(AbstractStereoCamera::CAMERA_TYPE_BASLER_USB);
    std::vector<AbstractStereoCamera::StereoCameraSerialInfo> known_serial_infos;
    known_serial_infos.insert( known_serial_infos.end(), known_serial_infos_gige.begin(), known_serial_infos_gige.end() );
    known_serial_infos.insert( known_serial_infos.end(), known_serial_infos_usb.begin(), known_serial_infos_usb.end() );

    std::vector<AbstractStereoCamera::StereoCameraSerialInfo> connected_serial_infos;
    // find basler systems connected
    // Initialise Basler Pylon
    // Create an instant camera object with the camera device found first.
    Pylon::CTlFactory& tlFactory = Pylon::CTlFactory::GetInstance();

    Pylon::DeviceInfoList_t devices;
    tlFactory.EnumerateDevices(devices);

    Pylon::CInstantCameraArray all_cameras(devices.size());
    Pylon::CDeviceInfo info;

    std::string DEVICE_CLASS_GIGE = "BaslerGigE";
    std::string DEVICE_CLASS_USB = "BaslerUsb";

    std::vector<std::string> connected_camera_names;
    std::vector<std::string> connected_serials;
    //TODO add generic way to recognise i3dr cameras whilst still being
    //able to make sure the correct right and left cameras are selected together
    for (size_t i = 0; i < all_cameras.GetSize(); ++i)
    {
        all_cameras[i].Attach(tlFactory.CreateDevice(devices[i]));
        std::string device_class = std::string(all_cameras[i].GetDeviceInfo().GetDeviceClass());
        std::string device_name = std::string(all_cameras[i].GetDeviceInfo().GetUserDefinedName());
        std::string device_serial = std::string(all_cameras[i].GetDeviceInfo().GetSerialNumber());
        //if (device_name.find("i3dr") != std::string::npos) {
        if (device_class == DEVICE_CLASS_GIGE || device_class == DEVICE_CLASS_USB){
            connected_serials.push_back(device_serial);
            connected_camera_names.push_back(device_name);
        } else {
            qDebug() << "Unsupported basler class: " << device_class.c_str();
        }
        //} else {
        //    qDebug() << "Unsupported basler camera with name: " << device_name.c_str();
        //}
    }

    for (auto& known_serial_info : known_serial_infos) {
        bool left_found = false;
        bool right_found = false;
        std::string left_serial;
        std::string right_serial;
        //find left
        for (auto& connected_serial : connected_serials)
        {
            left_serial = connected_serial;
            if (left_serial == known_serial_info.left_camera_serial){
                left_found = true;
                break;
            }
        }
        if (left_found){
            //find right
            for (auto& connected_serial : connected_serials)
            {
                right_serial = connected_serial;
                if (right_serial == known_serial_info.right_camera_serial){
                    right_found = true;
                    break;
                }
            }
        }
        if (left_found && right_found){ //only add if both cameras found
            connected_serial_infos.push_back(known_serial_info);
        }
    }

    //Pylon::PylonTerminate();
    return connected_serial_infos;
}

void StereoCameraBasler::getImageSize(Pylon::CInstantCamera &camera, int &width, int &height, int &bitdepth)
{
    try
    {
        camera.Open();
        width = Pylon::CIntegerParameter(camera.GetNodeMap(), "Width").GetValue();
        height = Pylon::CIntegerParameter(camera.GetNodeMap(), "Height").GetValue();
        bitdepth = 1; //TODO get bitdepth
    }
    catch (const Pylon::GenericException &e)
    {
        // Error handling.
        std::cerr << "An exception occurred whilst getting camera image size." << std::endl
                  << e.GetDescription() << std::endl;
    }
}

bool StereoCameraBasler::setPacketSize(int packetSize)
{
    try
    {
        if (packetSize >= 220){
            for (size_t i = 0; i < cameras->GetSize(); ++i)
            {
                cameras->operator[](i).Open();
                Pylon::CIntegerParameter(cameras->operator[](i).GetNodeMap(), "GevSCPSPacketSize").SetValue(packetSize);
            }
        }
        return true;
    }
    catch (const Pylon::GenericException &e)
    {
        // Error handling.
        std::cerr << "An exception occurred." << std::endl
                  << e.GetDescription() << std::endl;
        return false;
    }
}

bool StereoCameraBasler::setPacketDelay(int interPacketDelay)
{
    try
    {
        if (interPacketDelay >= 0){
            cameras->operator[](0).Open();
            Pylon::CIntegerParameter(cameras->operator[](0).GetNodeMap(), "GevSCPD").SetValue(interPacketDelay);
        }
        return true;
    }
    catch (const Pylon::GenericException &e)
    {
        // Error handling.
        std::cerr << "An exception occurred." << std::endl
                  << e.GetDescription() << std::endl;
        return false;
    }
}


bool StereoCameraBasler::setFPS(int val){
    try
    {
        float fps_f = (float)val;
        for (size_t i = 0; i < cameras->GetSize(); ++i)
        {
            cameras->operator[](i).Open();
            if (stereoCameraSerialInfo_.camera_type == CAMERA_TYPE_BASLER_GIGE){
                Pylon::CFloatParameter(cameras->operator[](i).GetNodeMap(), "AcquisitionFrameRateAbs").SetValue(fps_f);
            }
            if  (stereoCameraSerialInfo_.camera_type == CAMERA_TYPE_BASLER_USB){
                Pylon::CFloatParameter(cameras->operator[](i).GetNodeMap(), "AcquisitionFrameRate").SetValue(fps_f);
            }
        }
        return true;
    }
    catch (const Pylon::GenericException &e)
    {
        // Error handling.
        std::cerr << "An exception occurred." << std::endl
                  << e.GetDescription() << std::endl;
        return false;
    }
}

bool StereoCameraBasler::enableFPS(bool enable){
    try
    {
        for (size_t i = 0; i < cameras->GetSize(); ++i)
        {
            cameras->operator[](i).Open();
            Pylon::CBooleanParameter(cameras->operator[](i).GetNodeMap(), "AcquisitionFrameRateEnable").SetValue(enable);
        }
        return true;
    }
    catch (const Pylon::GenericException &e)
    {
        // Error handling.
        std::cerr << "An exception occurred." << std::endl
                  << e.GetDescription() << std::endl;
        return false;
    }
}

bool StereoCameraBasler::setExposure(double val) {
    try
    {
        // convert from seconds to milliseconds
        int exposure_i = val * 10000;
        qDebug() << "Setting exposure..." << exposure_i;
        for (size_t i = 0; i < cameras->GetSize(); ++i)
        {
            cameras->operator[](i).Open();
            if (stereoCameraSerialInfo_.camera_type == CAMERA_TYPE_BASLER_GIGE){
                Pylon::CIntegerParameter(cameras->operator[](i).GetNodeMap(), "ExposureTimeRaw").SetValue(exposure_i);
            }
            if  (stereoCameraSerialInfo_.camera_type == CAMERA_TYPE_BASLER_USB){
                Pylon::CFloatParameter(cameras->operator[](i).GetNodeMap(), "ExposureTime").SetValue(exposure_i);
            }
        }
        return true;
    }
    catch (const Pylon::GenericException &e)
    {
        // Error handling.
        std::cerr << "An exception occurred." << std::endl
                  << e.GetDescription() << std::endl;
        return false;
    }
}

bool StereoCameraBasler::setBinning(int val){
    try
    {
        if (val >= 1){
            for (size_t i = 0; i < cameras->GetSize(); ++i)
            {
                cameras->operator[](i).Open();
                Pylon::CEnumParameter(cameras->operator[](i).GetNodeMap(), "BinningHorizontalMode").FromString("Average");
                Pylon::CIntegerParameter(cameras->operator[](i).GetNodeMap(), "BinningHorizontal").SetValue(val);
                Pylon::CEnumParameter(cameras->operator[](i).GetNodeMap(), "BinningVerticalMode").FromString("Average");
                Pylon::CIntegerParameter(cameras->operator[](i).GetNodeMap(), "BinningVertical").SetValue(val);
            }
        }

        getImageSize(cameras->operator[](0),image_width,image_height,image_bitdepth);
        emit update_size(image_width, image_height, image_bitdepth);
        return true;
    }
    catch (const Pylon::GenericException &e)
    {
        // Error handling.
        std::cerr << "An exception occurred." << std::endl
                  << e.GetDescription() << std::endl;
        return false;
    }
}

bool StereoCameraBasler::enableTrigger(bool enable){
    enableFPS(!enable);
    try
    {
        std::string enable_str = "Off";
        if (enable){
            enable_str = "On";
        }
        for (size_t i = 0; i < cameras->GetSize(); ++i)
        {
            cameras->operator[](i).Open();
            Pylon::CEnumParameter(cameras->operator[](i).GetNodeMap(), "TriggerMode").FromString(enable_str.c_str());
        }
        return true;
    }
    catch (const Pylon::GenericException &e)
    {
        // Error handling.
        std::cerr << "An exception occurred." << std::endl
                  << e.GetDescription() << std::endl;
        return false;
    }
}

bool StereoCameraBasler::setGain(int val) {
    try
    {
        int gain_i = val;
        for (size_t i = 0; i < cameras->GetSize(); ++i)
        {
            cameras->operator[](i).Open();
            if (stereoCameraSerialInfo_.camera_type == CAMERA_TYPE_BASLER_GIGE){
                Pylon::CIntegerParameter(cameras->operator[](i).GetNodeMap(), "GainRaw").SetValue(gain_i);
            }
            if  (stereoCameraSerialInfo_.camera_type == CAMERA_TYPE_BASLER_USB){
                Pylon::CFloatParameter(cameras->operator[](i).GetNodeMap(), "Gain").SetValue(gain_i);
            }
        }
        return true;
    }
    catch (const Pylon::GenericException &e)
    {
        // Error handling.
        std::cerr << "An exception occurred." << std::endl
                  << e.GetDescription() << std::endl;
        return false;
    }
}

bool StereoCameraBasler::enableAutoGain(bool enable){
    try
    {
        std::string enable_str = "Off";
        if (enable){
            enable_str = "Continuous";
        }
        for (size_t i = 0; i < cameras->GetSize(); ++i)
        {
            cameras->operator[](i).Open();
            Pylon::CEnumParameter(cameras->operator[](i).GetNodeMap(), "GainAuto").FromString(enable_str.c_str());
        }
        return true;
    }
    catch (const Pylon::GenericException &e)
    {
        // Error handling.
        std::cerr << "An exception occurred." << std::endl
                  << e.GetDescription() << std::endl;
        return false;
    }
}

bool StereoCameraBasler::enableAutoExposure(bool enable){
    try
    {
        std::string enable_str = "Off";
        if (enable){
            enable_str = "Continuous";
        }
        for (size_t i = 0; i < cameras->GetSize(); ++i)
        {
            cameras->operator[](i).Open();
            Pylon::CEnumParameter(cameras->operator[](i).GetNodeMap(), "ExposureAuto").FromString(enable_str.c_str());
        }
        return true;
    }
    catch (const Pylon::GenericException &e)
    {
        // Error handling.
        std::cerr << "An exception occurred." << std::endl
                  << e.GetDescription() << std::endl;
        return false;
    }
}

void StereoCameraBasler::leftCaptured(bool success){
    bool grab_event_l = true;
    if (success){
        cv::Mat image_temp = leftImageEventHandler->getImage();
        image_temp.copyTo(left_raw);
    } else {
        emit error(CAPTURE_ERROR);
    }
    if (grab_event_l && grab_event_r){
        emit captured();
        grab_event_l = false;
        grab_event_r = false;
    }
}

void StereoCameraBasler::rightCaptured(bool success){
    bool grab_event_r = true;
    if (success){
        cv::Mat image_temp = rightImageEventHandler->getImage();
        image_temp.copyTo(right_raw);
    } else {
        emit error(CAPTURE_ERROR);
    }
    if (grab_event_l && grab_event_r){
        emit captured();
        grab_event_l = false;
        grab_event_r = false;
    }
}

bool StereoCameraBasler::captureSingle(){
    //Start capture
    capture_one = true;
    //Start capture thread
    for (size_t i = 0; i < cameras->GetSize(); ++i)
    {
        cameras->operator[](i).StartGrabbing(Pylon::EGrabStrategy::GrabStrategy_LatestImages);
    }
    connect(leftImageEventHandler, SIGNAL(image_grabbed(bool)), this, SLOT(leftCaptured(bool)));
    connect(rightImageEventHandler, SIGNAL(image_grabbed(bool)), this, SLOT(rightCaptured(bool)));
    grab_event_l = false;
    grab_event_r = false;
    capturing = true;
    return true;
}

bool StereoCameraBasler::enableCapture(bool enable){
    if (enable){
        capture_one = false;
        //Start capture
        for (size_t i = 0; i < cameras->GetSize(); ++i)
        {
            cameras->operator[](i).StartGrabbing(Pylon::EGrabStrategy::GrabStrategy_LatestImages);
        }
        connect(leftImageEventHandler, SIGNAL(image_grabbed(bool)), this, SLOT(leftCaptured(bool)));
        connect(rightImageEventHandler, SIGNAL(image_grabbed(bool)), this, SLOT(rightCaptured(bool)));
        grab_event_l = false;
        grab_event_r = false;
        capturing = true;
    } else {
        //Stop capture
        disconnect(leftImageEventHandler, SIGNAL(image_grabbed(bool)), this, SLOT(leftCaptured(bool)));
        disconnect(rightImageEventHandler, SIGNAL(image_grabbed(bool)), this, SLOT(rightCaptured(bool)));
        cameras->StopGrabbing();
        cameras->operator[](0).StopGrabbing();
        cameras->operator[](1).StopGrabbing();
        capturing = false;
    }
    return true;
}

//TODO add pylon disconnect callback

bool StereoCameraBasler::closeCamera() {
    if (connected){
        cameras->StopGrabbing();
        cameras->operator[](0).StopGrabbing();
        cameras->operator[](1).StopGrabbing();
        cameras->Close();
        cameras->operator[](0).Close();
        cameras->operator[](1).Close();

        //cameras->operator[](0).RegisterImageEventHandler(leftImageEventHandler, Pylon::RegistrationMode_Append, Pylon::Cleanup_Delete);
        //cameras->operator[](1).RegisterImageEventHandler(rightImageEventHandler, Pylon::RegistrationMode_Append, Pylon::Cleanup_Delete);
        //delete leftImageEventHandler;
        //delete rightImageEventHandler;
    }
    connected = false;
    emit disconnected();
    return true;
}

StereoCameraBasler::~StereoCameraBasler() {
    Pylon::PylonTerminate();
}
