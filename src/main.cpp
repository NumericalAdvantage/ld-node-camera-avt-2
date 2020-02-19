/*
 * This file is part of project link.developers/ld-node-camera-avt-2.
 * It is copyrighted by the contributors recorded in the version control history of the file,
 * available from its original location https://gitlab.com/link.developers/ld-node-camera-avt-2.
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#include <iostream>
#include "avtcamdriver.h"

int main(int argc, char** argv)
{
    try {
        DRAIVE::Link2::NodeResources nodeResources { "l2spec:/link_dev/ld-node-camera-avt-2", argc, argv };
        DRAIVE::Link2::NodeDiscovery nodeDiscovery { nodeResources };
        
        DRAIVE::Link2::ConfigurationNode rootNode = nodeResources.getUserConfiguration();
        DRAIVE::Link2::OutputPin outputPin{nodeDiscovery, nodeResources, "avt-camera-2-output-pin"};
       
        DRAIVE::Link2::SignalHandler signalHandler {};
        signalHandler.setReceiveSignalTimeout(-1);

        link_dev::Services::AVTCamDriver avtcamdriver{signalHandler,
                                                      nodeResources,
                                                      nodeDiscovery,
                                                      outputPin, 
                                                      rootNode.getString("CameraID"),
                                                      rootNode.getUInt("ImageWidth"),
                                                      rootNode.getUInt("ImageHeight"),
                                                      rootNode.getString("OutputFormat"),
                                                      rootNode.getUInt("CompressionQuality"),
                                                      rootNode.getUInt("Binning"),
                                                      rootNode.getUInt("FPS"),
                                                      rootNode.getBoolean("UseJumboPackets"),
                                                      rootNode.getBoolean("AutoExposure"),
                                                      rootNode.getBoolean("ContinuousAutoExposure"),
                                                      rootNode.getString("XMLFile"),
                                                      rootNode.getBoolean("PreferConfig")};

        avtcamdriver.run();     //Runs until interrupt signal is sent to the program.
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
