/*
 * This file is part of project link.developers/ld-node-camera-avt-2.
 * It is copyrighted by the contributors recorded in the version control history of the file,
 * available from its original location https://gitlab.com/link.developers/ld-node-camera-avt-2.
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#ifndef AVTCAMDRIVER_HPP
#define AVTCAMDRIVER_HPP

#include <VimbaCPP/Include/VimbaSystem.h>
#include <DRAIVE/Link2/NodeDiscovery.hpp>
#include <DRAIVE/Link2/NodeResources.hpp>
#include <DRAIVE/Link2/SignalHandler.hpp>
#include <DRAIVE/Link2/ConfigurationNode.hpp>
#include <DRAIVE/Link2/OutputPin.hpp>

namespace link_dev 
{
	namespace Services 
    {
		class AVTCamDriver
        {
            public:
                
                std::string m_cameraID = "";             //ID of the H/W Camera used 
                uint64_t m_imageWidth = 0;               //Width of the image resolution
                uint64_t m_imageHeight = 0;              //Height of the image resolution
                std::string m_outputFormat = "raw";
                uint64_t m_compressionQuality = 50;
                uint64_t m_binning = 1;                  //Number of bins
                uint64_t m_Fps = 0;                      //Frames Per Second in float
                bool m_useJumboPackets = false;          //True if Jumbo Packets are to be used
                bool m_autoExposure = false;             //True if AutoExposure is to be used
                bool m_continuousAutoExposure = false;   //True if Continuous Auto Exposure is to be used
                std::string m_XMLFile = "";              //Path of the XML configuration file to be used. If none used, pass "F"
                bool m_preferConfig = false;             //True if ini configuration file is to be preferred over XML settings

                AVTCamDriver(DRAIVE::Link2::SignalHandler signalHandler,
                             DRAIVE::Link2::NodeResources nodeResources,
                             DRAIVE::Link2::NodeDiscovery nodeDiscovery,
                             DRAIVE::Link2::OutputPin outputPin,
                             std::string cameraID, uint64_t imageWidth,
                             uint64_t imageHeight, std::string outputFormat,
                             uint64_t compressorQuality, uint64_t binning,
                             uint64_t Fps, bool useJumboPackets, 
                             bool autoExposure, 
                             bool continuousAutoExposure,
                             std::string XMLFile, bool preferConfig) :
                             m_signalHandler(signalHandler),
                             m_nodeResources(nodeResources),
                             m_nodeDiscovery(nodeDiscovery),
                             m_outputPin(outputPin),
                             m_cameraID(cameraID), m_imageWidth(imageWidth),
                             m_imageHeight(imageHeight), m_outputFormat(outputFormat),
                             m_compressionQuality(compressorQuality),
                             m_binning(binning),
                             m_Fps(Fps), m_useJumboPackets(useJumboPackets),
                             m_autoExposure(autoExposure),
                             m_continuousAutoExposure(continuousAutoExposure),
                             m_XMLFile(XMLFile), m_preferConfig(preferConfig)
                {
                }
                
                int run();      

                class VimbaException : public std::runtime_error 
                {
                    public:
			    	    VimbaException(const char *message) : std::runtime_error(message) { };
                };

            private:
                DRAIVE::Link2::SignalHandler m_signalHandler;
                DRAIVE::Link2::NodeResources m_nodeResources;
                DRAIVE::Link2::NodeDiscovery m_nodeDiscovery;
                DRAIVE::Link2::OutputPin m_outputPin;
                AVT::VmbAPI::VimbaSystem& m_vimbaSystem = 
                            AVT::VmbAPI::VimbaSystem::GetInstance(); 
                AVT::VmbAPI::CameraPtr m_cameraPtr;  //Pointer to the camera instance to be used

                void init();       //Initialization logic for the Vimba SDK
                void configure();  //Configuration logic for the Vimba SDK 
    		    
                // Frame Observer Class for an oject that keeps a watch out for the frame acquisition
		        class FrameObserver : public AVT::VmbAPI::IFrameObserver 
                {
		            public:
                        FrameObserver(AVT::VmbAPI::CameraPtr pCamera, AVTCamDriver *pParent);
                        void FrameReceived(const AVT::VmbAPI::FramePtr pFrame);  /*!< Handler logic which runs when a frame is received. */

                    private:
                        AVTCamDriver* m_parent;  /*!< Pointer to the parent camdriver class  */
                };

                friend class FrameObserver;
		};
	}
}

#endif
