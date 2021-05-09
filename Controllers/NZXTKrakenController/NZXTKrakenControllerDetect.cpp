#include "Detector.h"
#include "NZXTKrakenController.h"
#include "FanController_NZXTKraken.h"
#include "RGBController.h"
#include "RGBController_NZXTKraken.h"
#include <vector>
#include <hidapi/hidapi.h>

#define NZXT_KRAKEN_VID     0x1E71
#define NZXT_KRAKEN_X2_PID  0x170E
#define NZXT_KRAKEN_M2_PID  0x1715

/******************************************************************************************\
*                                                                                          *
*   DetectNZXTKrakenControllers                                                            *
*                                                                                          *
*       Detect devices supported by the NZXTKraken driver                                  *
*                                                                                          *
\******************************************************************************************/

void DetectNZXTKrakenControllers(hid_device_info* info, const std::string& name)
{
    hid_device* dev = hid_open_path(info->path);
    if( dev )
    {
        NZXTKrakenController* controller = new NZXTKrakenController(dev, info->path);
        RGBController_NZXTKraken* rgb_controller = new RGBController_NZXTKraken(controller);
        rgb_controller->name = name;
        ResourceManager::get()->RegisterRGBController(rgb_controller);

        if(info->product_id == NZXT_KRAKEN_X2_PID)
        {
            /*---------------------------------------------*\
            | Kraken M22 doesn't have liquid temp sensor or |
            | ability to report or set fan or pump speeds   |
            \*---------------------------------------------*/
            FanController_NZXTKraken* fan_controller = new FanController_NZXTKraken(controller);
            fan_controller->name = name;
            ResourceManager::get()->RegisterFanController(fan_controller);
        }
    }
}   /* DetectNZXTKrakenControllers() */

REGISTER_HID_DETECTOR("NZXT Kraken X2", DetectNZXTKrakenControllers, NZXT_KRAKEN_VID, NZXT_KRAKEN_X2_PID);
REGISTER_HID_DETECTOR("NZXT Kraken M2", DetectNZXTKrakenControllers, NZXT_KRAKEN_VID, NZXT_KRAKEN_M2_PID);
