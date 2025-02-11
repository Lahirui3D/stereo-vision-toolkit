/*
* Copyright I3D Robotics Ltd, 2020
* Author: Josh Veitch-Michaelis, Ben Knight (bknight@i3drobotics.com)
*/

#ifndef MATCHEROPENCVSGBM_H
#define MATCHEROPENCVSGBM_H

#include <abstractstereomatcher.h>
#ifdef WITH_OPENCV_CONTRIB
    #include <opencv2/ximgproc.hpp>
#endif
#include <QFile>

//!  Matcher OpenCV SGBM
/*!
  Stereo matcher using OpenCV's SGBM algorithm
*/

class MatcherOpenCVSGBM : public AbstractStereoMatcher
{
    Q_OBJECT
public:
    explicit MatcherOpenCVSGBM(QObject *parent = 0)
        : AbstractStereoMatcher(parent) {
        init();
    }

    bool isLicenseValid(){return true;}

public slots:
    void setMinDisparity(int min_disparity);
    void setDisparityRange(int disparity_range);
    void setBlockSize(int block_size);
    void setDisp12MaxDiff(int diff);
    void setUniquenessRatio(int ratio);
    void setSpeckleFilterWindow(int window);
    void setSpeckleFilterRange(int range);
    void setWLSFilterEnabled(bool enable);
    int getErrorDisparity(void);

    void saveParams();

    bool forwardMatch(cv::Mat left_img, cv::Mat right_img);
    bool backwardMatch(cv::Mat left_img, cv::Mat right_img);


    int getMinDisparity(){return matcher->getMinDisparity();}
    int getDisparityRange(){return matcher->getNumDisparities();}
    int getBlockSize(){return matcher->getBlockSize();}
    int getDisp12MaxDiff(){return matcher->getDisp12MaxDiff();}
    int getPrefilterCap(){return matcher->getPreFilterCap();}
    int getUniquenessRatio(){return matcher->getUniquenessRatio();}
    int getSpeckleFilterWindow(){return matcher->getSpeckleWindowSize();}
    int getSpeckleFilterRange(){return matcher->getSpeckleRange();}
    bool isWLSFilterEnabled(){return wls_filter;}

private:
    cv::Ptr<cv::StereoSGBM> matcher;

    void init(void);
    void setupDefaultMatcher(void);

    bool wls_filter = false;
    double wls_lambda = 8000;
    double wls_sigma = 1.5;
};

#endif // MATCHEROPENCVSGBM_H
