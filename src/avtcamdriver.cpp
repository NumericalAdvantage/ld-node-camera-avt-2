/*
 * This file is part of project link.developers/ld-node-camera-avt-2.
 * It is copyrighted by the contributors recorded in the version control history of the file,
 * available from its original location https://gitlab.com/link.developers/ld-node-camera-avt-2.
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#include <iostream>
#include <opencv2/imgproc/imgproc.hpp>
#include <link_dev/Interfaces/ImageCompression.h>
#include <link_dev/Interfaces/OpenCvToImage.h>
#include "avtcamdriver.h"
#include "GenericMatrix3D_generated.h"
#include "Image_generated.h"

#define NUM_FRAMES 3
#define JUMBO_PACKET_SIZE 1500
#define DEFAULT_COMPRESSION_LEVEL 0

void link_dev::Services::AVTCamDriver::init() 
{
    if (m_vimbaSystem.Startup() != VmbErrorSuccess)
    {
        throw new VimbaException("Cannot start-up Allied Vision Camera System.");
    }

    if (m_vimbaSystem.OpenCameraByID(m_cameraID.c_str(), VmbAccessModeFull, 
                                     m_cameraPtr) !=  VmbErrorSuccess)
    {
        throw new VimbaException("Cannot open camera in full access mode.");
    }
}

void link_dev::Services::AVTCamDriver::configure() 
{
	AVT::VmbAPI::FeaturePtr pFeature;
	bool xmlImportSuccess = false;

    //choose if settings get imported from xml-file
    if(m_XMLFile.compare("F") != 0)
    {
        VmbErrorType err = VmbErrorSuccess;

        //create settings struct to determine behaviour during loading
        VmbFeaturePersistSettings_t settingsStruct;
        settingsStruct.loggingLevel = 4;
        settingsStruct.maxIterations = 5;
        settingsStruct.persistType = VmbFeaturePersistNoLUT;

        //re-load saved settings from file
        err = m_cameraPtr->LoadCameraSettings(m_XMLFile, &settingsStruct);
        if(err != VmbErrorSuccess)
        {
            std::cout << "ERROR: Can not import settings from " << m_XMLFile << " [VimbaCamera] error code: " << err << std::endl;		
        }
        else if(VmbErrorSuccess == err)
        {
            std::cout << "Settings successfully imported from " << m_XMLFile << std::endl;
            xmlImportSuccess = true;
        }
    }

    //when XMLFile is set to "F" in config (.ini) -> set integrated config (.ini) values
    //when PreferConfig is set to true in config (.ini) -> overwrite loaded values of xml-file with config (.ini) values for respective variables
    if (m_preferConfig)
    {
        std::cout << "Instance file user configuration would be preferred." << std::endl;

        if(xmlImportSuccess)
            std::cout << "WARNING: Config.ini values are used - overwrites values of according variables of XML settings file!" << std::endl;

         if(!m_useJumboPackets) 
        {
            if(m_cameraPtr->GetFeatureByName("GVSPPacketSize", pFeature) == VmbErrorType::VmbErrorSuccess)
                pFeature->SetValue(JUMBO_PACKET_SIZE);
            else
                std::cout << "WARNING: Setting JumboPackets not found!" << std::endl;
        }

        if(m_cameraPtr->GetFeatureByName("PixelFormat", pFeature) == VmbErrorType::VmbErrorSuccess)
            pFeature->SetValue(VmbPixelFormatMono8);
        else
            std::cout << "WARNING: Setting PixelFormat not found!" << std::endl;

        VmbInt64_t maxWidth = 2048, maxHeight = 1088;

        if(m_cameraPtr->GetFeatureByName("WidthMax", pFeature) == VmbErrorType::VmbErrorSuccess)
            pFeature->GetValue(maxWidth);
        else
            std::cout << "WARNING: Setting WidthMax not found!" << std::endl;

        if(m_cameraPtr->GetFeatureByName("HeightMax", pFeature) == VmbErrorType::VmbErrorSuccess)
            pFeature->GetValue(maxHeight);
        else
            std::cout << "WARNING: Setting HeightMax not found!" << std::endl;

        if(m_cameraPtr->GetFeatureByName("Width", pFeature) == VmbErrorType::VmbErrorSuccess)
            pFeature->SetValue((VmbInt32_t) m_imageWidth);
        else
            std::cout << "WARNING: Setting Width not found!" << std::endl;

        if(m_cameraPtr->GetFeatureByName("Height", pFeature) == VmbErrorType::VmbErrorSuccess)
            pFeature->SetValue((VmbInt32_t) m_imageHeight);
        else
            std::cout << "WARNING: Setting Height not found!" << std::endl;
		
        if(m_cameraPtr->GetFeatureByName("BinningHorizontal", pFeature) == VmbErrorType::VmbErrorSuccess)
            pFeature->SetValue((VmbInt32_t)m_binning);
        else
            std::cout << "WARNING: Setting BinningHorizontal not found!" << std::endl;

        if(m_cameraPtr->GetFeatureByName("BinningVertical", pFeature) == VmbErrorType::VmbErrorSuccess)
            pFeature->SetValue((VmbInt32_t)m_binning);
        else
            std::cout << "WARNING: Setting BinningVertical not found!" << std::endl;

        if(m_cameraPtr->GetFeatureByName("OffsetX", pFeature) == VmbErrorType::VmbErrorSuccess)
            pFeature->SetValue((VmbInt64_t)(maxWidth - m_imageWidth) / 2);
        else
            std::cout << "WARNING: Setting OffsetX not found!" << std::endl;

        if(m_cameraPtr->GetFeatureByName("OffsetY", pFeature) == VmbErrorType::VmbErrorSuccess)
            pFeature->SetValue((VmbInt64_t)(maxHeight - m_imageHeight) / 2);
        else
            std::cout << "WARNING: Setting OffsetY not found!" << std::endl;

        if(m_cameraPtr->GetFeatureByName("TriggerMode", pFeature) == VmbErrorType::VmbErrorSuccess)
            pFeature->SetValue(VmbBoolFalse);
        else
            std::cout << "WARNING: Setting TriggerMode not found!" << std::endl;

        if(m_cameraPtr->GetFeatureByName("AcquisitionFrameRateAbs", pFeature) == VmbErrorType::VmbErrorSuccess)
            pFeature->SetValue((VmbInt32_t)m_Fps);
        else
            std::cout << "WARNING: Setting AcquisitionFrameRateAbs not found!" << std::endl;

        if(m_cameraPtr->GetFeatureByName("ExposureAuto", pFeature) == VmbErrorType::VmbErrorSuccess)
        {
            if(m_autoExposure) 
            {
                if(m_continuousAutoExposure) 
                {
                    pFeature->SetValue("Continuous");
                }
                else
                {
                    pFeature->SetValue("Once");
                }
            }
            else 
            {
                pFeature->SetValue("Off");
            }
        }
        else
            std::cout << "WARNING: Setting ExposureAuto not found!" << std::endl;
    }
}

int link_dev::Services::AVTCamDriver::run() 
{
    try 
    {
        init();
        configure();
        m_cameraPtr->StartContinuousImageAcquisition(NUM_FRAMES, 
        AVT::VmbAPI::IFrameObserverPtr(new AVTCamDriver::FrameObserver(m_cameraPtr, this)));
    }
    catch(std::exception &e)
    {
        std::cout << "[AVTCamera] Critical startup error: " << e.what() << std::endl;
        m_vimbaSystem.Shutdown();
        return 1;
    }

    while(m_signalHandler.receiveSignal() != LINK2_SIGNAL_INTERRUPT); 

    std::cout << "Ending the run() function." << std::endl;
	m_cameraPtr->StopContinuousImageAcquisition();
	m_vimbaSystem.Shutdown();
}

/*! FrameObserver Object initialization */
link_dev::Services::AVTCamDriver::FrameObserver::FrameObserver(
                               AVT::VmbAPI::CameraPtr pCamera, 
                               AVTCamDriver *parent) : IFrameObserver(pCamera) 
{
	m_parent = parent;
}

/*! \brief FrameReceived method
*    handle logic for the frameReceived event.
*    \param AVT::VmbAPI::FramePtr pFrame
*    \return void
*/
void link_dev::Services::AVTCamDriver::FrameObserver::FrameReceived(
                                             const AVT::VmbAPI::FramePtr pFrame) 
{
    uint32_t width, height;
    pFrame->GetHeight(height);
    pFrame->GetWidth(width);
    unsigned char *pBuffer;
    cv::Size imageSize(width, height);
    pFrame->GetBuffer(pBuffer);
    cv::Mat frame(imageSize, CV_8UC1, pBuffer);
    
    if (height == m_parent->m_imageHeight && width == m_parent->m_imageWidth) 
    {
        if(!frame.empty()) 
        {
            if(m_parent->m_outputFormat.compare("raw") == 0)
            {
                link_dev::ImageT currentImage 
                = link_dev::Interfaces::ImageFromOpenCV(frame, link_dev::Format::Format_GRAY_U8);   
                
                m_parent->m_outputPin.push(currentImage, "AVTCamImage");
            }
            else if(m_parent->m_outputFormat.compare("compressed") == 0)
            {
                link_dev::ImageT currentImage 
                = link_dev::Interfaces::ImageFromOpenCV(frame, link_dev::Format::Format_GRAY_U8);   
                link_dev::Interfaces::CompressionHandler compressorH;
                
                if(m_parent->m_compressionQuality > 0)
                {
                    compressorH.imageQuality = m_parent->m_compressionQuality;
                }
                else
                {
                    compressorH.imageQuality = DEFAULT_COMPRESSION_LEVEL;
                }
                
                link_dev::ImageT currentCompressedImage = 
                link_dev::Interfaces::Compress(currentImage, compressorH);
                m_parent->m_outputPin.push(currentCompressedImage, "AVTCamImage");
            }
        } 
        else 
        {
            throw std::runtime_error("Frame arriving from camera is.");
        }
    }
    m_pCamera->QueueFrame(pFrame);
}
