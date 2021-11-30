# Stereo Vision Toolkit

[Software Website](https://i3drobotics.github.io/stereo-vision-toolkit/)

## Latest Release
Download the latest release [here](https://github.com/i3drobotics/stereo-vision-toolkit/releases/download/v1.3.3/StereoVisionToolkit-1.3.3-Win64.exe)

For detailed instructions see the [User Guide](https://i3drobotics.github.io/stereo-vision-toolkit/app/UserGuide.pdf)

## Status
[![Deployment](https://github.com/i3drobotics/stereo-vision-toolkit/actions/workflows/deploy.yml/badge.svg)](https://github.com/i3drobotics/stereo-vision-toolkit/actions/workflows/deploy.yml)

## About
I3DR's stereo vision toolkit is an application provided for testing of stereo cameras and gathering of 3D data.

You can calibrate stereo cameras, acquire images and perform matching and 3D reconstruction. You can save raw stereo video to capture a scene and then replay it in the software to fine tune matching parameters.

SVTK is under active development. At the moment the software only officially supports the i3DR Deimos and Phobos cameras, but in principle any usb camera pair or pre-rectified stereo video of the correct format (side-by-side) will work.

## Roadmap
Currently SVTK is a useful and functional tool for exploring stereo imaging, and allows you to get going with your I3DR stereo camera quickly. There are a number of features/improvements in development including:

- Live camera calibration (currently in beta)
- Automated calibration correction
- Linux support
- ROS support
- Unit tests and other deployment improvements

See [issues](https://github.com/i3drobotics/stereo-vision-toolkit/issues) for details on known bugs and future enhancements

## Stereo matching support
We have included support for two of OpenCV's matchers: the basic block matcher and semi-global block matching. The block matcher will run at over 60fps on a fast CPU (e.g. i5.) SGBM should provide better results, but will run around a factor of five slower.

I3DR's own 3D matching algorithm (I3DRSGM) is also built into this application by default however a license is required to be able to run it. Please contact info@i3drobotics.com for requesting a license. 

## Installation
Download and install the windows installer for the latest release [here](https://github.com/i3drobotics/stereo-vision-toolkit/releases/download/v1.3.3/StereoVisionToolkit-1.3.3-Win64.exe).

See the [User Guide](https://i3drobotics.github.io/stereo-vision-toolkit/app/UserGuide.pdf) for details on using the software as well as building from source. 

## Development
This application is under active development and it is likely that in the short term there may be breaking changes to classes, or significant changes to the GUI. Have a look at the [project milestones](https://github.com/i3drobotics/stereo-vision-toolkit/milestones) for information on future improvements.

Feel free to send us pull requests!

## License
This code is provided under the MIT license, which essentially means it's open source, but we require you to add our copyright if you distribute it elsewhere.

## Previous Releases
See [release](https://github.com/i3drobotics/stereo-vision-toolkit/releases) for previous builds. 

## Developer zone
See [User Guide](https://i3drobotics.github.io/stereo-vision-toolkit/app/UserGuide.pdf) for details on building from source.

Documentation can be found [here](https://i3drobotics.github.io/stereo-vision-toolkit/definitions/html/index.html). 

### 3rd Party Content
This repository is used for internal development and so we include both debug and release libraries and DLLs. This makes for quite a large repository for a rather small codebase (around 500MB), so be warned. 

The project uses OpenCV for image processing, PCL and VTK for point cloud visualisation and hidapi for camera control. PCL requires Boost and Eigen which are included here. These dependencies are provided in accordance with their respective licenses which may be found in the license folder. We also use FontAwesome for icons via QtAwesome, along with QDarkStyle.

#### Automatic install
Some libraries have been moved to be externally downloaded to reduce the repostiory size. To download these libraries use the '3rdparty.bat' script provided in 'scripts'. This will download the libraries of the correct versions to the correct folders.  
*Note: This process builds boost with the required boost libraries so will take some time to complete.*  
*Note: These scripts use command line features that are only avaiable in Windows 10 'curl' and 'tar'. If you are on a Windows 8 or lower machine you will need to install the Windows version of these commands and add them to your PATH variable.*
```
cd PATH_TO_REPO/scripts
3rdparty.bat
```

#### Boost
Boost requires Visual Studio 2017, and needs cl.exe to be in the system PATH variable or to run '3rdparty.bat' in a Visual Studio command prompt. 

#### QT 
Tested with QT 5.14.2 with Visual Studio 2017.  
Requires QT Webengine module for use in Fervor. 

#### OpenCV Contrib & CUDA
There is limited usage of CUDA for certain image processing steps (e.g. rectification) and neural network support and OpenCV's ximageproc from contrib modules for WLS filter.

#### VCRedist
There is an issue where QT doesn't use the generic vc redist package and instead pulls from the system. Therefore the generic 2015-2019 vc redist is provided in this repository and can be updated using the script in '3rdparty/vcredist'.

### Phobos control
Arduino code for controlling Phobos cameras is provided in src/camera/camera_control.
This is for the serial communication between the arduino and this toolkit. See [issue](https://github.com/i3drobotics/stereo-vision-toolkit/issues/54) for more information.

### Procedure for new release
1. To make changes create a branch of the main repository with the name SVTK-XXX where XXX is the issue number of the task you are working on. If you are working on a code changing task (e.g. a task that will need building / testing of code) then you should create a feature branch e.g. feature/SVTK-XXX so that it is picked up by the automatic build process. 
2. Make your changes to the new branch.
2. Update version number in version.txt. Then run update_version.bat to update the version number across repo.
3. Update ReleaseNotes.html with improvements, bug fixes, and known issues.
4. Update the changelog.txt file with the changes the changes you have made and the associated GitHub issue number.
6. Create a pull request on GitHub for merging your branch to the main branch. 
7. Once the code has been tested and approved, merge the branch to the main branch. This will automatically create a new release.