#include "Detector.h"
#include "ASRockPolychromeSMBusController.h"
#include "LogManager.h"
#include "RGBController.h"
#include "RGBController_ASRockPolychromeSMBus.h"
#include "i2c_smbus.h"
#include "pci_ids.h"
#include <vector>
#include <stdio.h>
#include <stdlib.h>

/******************************************************************************************\
*                                                                                          *
*   TestForPolychromeSMBusController                                                       *
*                                                                                          *
*       Tests the given address to see if an ASRock Polychrome RGB controller exists there.*
*       First does a quick write to test for a response                                    *
*                                                                                          *
\******************************************************************************************/

bool TestForPolychromeSMBusController(i2c_smbus_interface* bus, unsigned char address)
{
    bool pass = false;

    int res = bus->i2c_smbus_write_quick(address, I2C_SMBUS_WRITE);

    if (res >= 0)
    {
        pass = true;
    }

    return(pass);

}   /* TestForPolychromeController() */

/******************************************************************************************\
*                                                                                          *
*   DetectPolychromeControllers                                                            *
*                                                                                          *
*       Detect ASRock Polychrome RGB SMBus controllers on the enumerated I2C busses at     *
*       address 0x6A.                                                                      *
*                                                                                          *
*           bus - pointer to i2c_smbus_interface where Polychrome device is connected      *
*           dev - I2C address of Polychrome device                                         *
*                                                                                          *
\******************************************************************************************/

void DetectPolychromeSMBusControllers(std::vector<i2c_smbus_interface*>& busses)
{
    PolychromeController* new_polychrome;
    RGBController_Polychrome* new_controller;

    for (unsigned int bus = 0; bus < busses.size(); bus++)
    {
        IF_MOBO_SMBUS(busses[bus]->pci_vendor, busses[bus]->pci_device)
        {
            LOG_TRACE("%s Bus %02d is an Motherboard, checking for for a device at 0x6A", ASROCK_DETECTOR_NAME, bus);
            // Check for Polychrome controller at 0x6A
            if (TestForPolychromeSMBusController(busses[bus], 0x6A))
            {
                LOG_TRACE("%s Detected a device at address 0x6A, Testing for a known controller", ASROCK_DETECTOR_NAME);
                new_polychrome = new PolychromeController(busses[bus], 0x6A);

                if(new_polychrome->GetASRockType() != ASROCK_TYPE_UNKNOWN)
                {
                    LOG_TRACE("%s Found a known Polychrome device", ASROCK_DETECTOR_NAME);
                    new_controller = new RGBController_Polychrome(new_polychrome);
                    ResourceManager::get()->RegisterRGBController(new_controller);
                }
                else
                {
                    LOG_TRACE("%s Not a Polychrome device or unknown type", ASROCK_DETECTOR_NAME);
                    delete new_polychrome;
                }
            }
            else
            {
                LOG_TRACE("%s Bus %02d no response at 0x6A", ASROCK_DETECTOR_NAME, bus);
            }
        }
        else
        {
            LOG_TRACE("%s Bus %02d not a Motherboard", ASROCK_DETECTOR_NAME, bus);
        }
    }

}   /* DetectSMBusPolychromeControllers() */

REGISTER_I2C_DETECTOR("ASRock Polychrome SMBus", DetectPolychromeSMBusControllers);
