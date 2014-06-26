/* 
 * File:   main.cpp
 * Author: Richard Greene
 *
 * Created on March 26, 2014, 4:01 PM
 */

#include <PrintEngine.h>
#include <EventHandler.h>
#include <TerminalUI.h>
#include <Logger.h>
#include <NetworkInterface.h>
#include <CommandInterpreter.h>
#include <Settings.h>

using namespace std;

int main(int argc, char** argv) 
{
    // force settings file to be regenerated if it's missing
    SETTINGS.GetString(JOB_NAME_SETTING);
    
    // create an event handler
    EventHandler eh;
    
    // create a print engine that communicates with actual hardware
    PrintEngine pe(true);
    
    // give it to the settings singleton as an error handler
    SETTINGS.SetErrorHandler(&pe);
    
    // create the front panel
    FrontPanel fp(UI_SLAVE_ADDRESS); 
 
    // set the I2C devices
    eh.SetI2CDevice(MotorInterrupt, pe.GetMotorBoard(), MOTOR_STATUS);
    eh.SetI2CDevice(ButtonInterrupt, &fp, UI_STATUS);
    
    // subscribe logger singleton first, so that it will show 
    // its output in the logs ahead of any other subscribers that actually 
    // act on those events
    eh.Subscribe(PrinterStatusUpdate, &LOGGER);
    eh.Subscribe(MotorInterrupt, &LOGGER);
    eh.Subscribe(ButtonInterrupt, &LOGGER);
    eh.Subscribe(DoorInterrupt, &LOGGER);
    eh.Subscribe(Keyboard, &LOGGER);
    eh.Subscribe(UICommand, &LOGGER);
    
    // subscribe the print engine to interrupt events
    eh.Subscribe(MotorInterrupt, &pe);
    eh.Subscribe(DoorInterrupt, &pe);
    
    // subscribe the print engine to timer events
    eh.SetFileDescriptor(PrintEnginePulse, pe.GetPulseTimerFD()); 
    eh.Subscribe(PrintEnginePulse, &pe);
    
    eh.SetFileDescriptor(ExposureEnd, pe.GetExposureTimerFD());
    eh.Subscribe(ExposureEnd, &pe);
    
    eh.SetFileDescriptor(MotorTimeout, pe.GetMotorTimeoutTimerFD());
    eh.Subscribe(MotorTimeout, &pe);
    
    CommandInterpreter cmdInterpreter(&pe);
    // subscribe the command interpreter to command input events,
    // from UI, keyboard, or front panel buttons
    eh.Subscribe(UICommand, &cmdInterpreter);    
    eh.Subscribe(Keyboard, &cmdInterpreter);
    eh.Subscribe(ButtonInterrupt, &cmdInterpreter);    
    
    // subscribe the front panel to printer status events
    eh.SetFileDescriptor(PrinterStatusUpdate, pe.GetStatusUpdateFD()); 
    eh.Subscribe(PrinterStatusUpdate, &fp);
    
    // also connect a network interface, subscribed to UI commands and 
    // printer status events
    NetworkInterface networkIF;
    eh.Subscribe(PrinterStatusUpdate, &networkIF);
    eh.Subscribe(UICommand, &networkIF);
    
    // also connect a terminal UI, subscribed to printer status events
    TerminalUI terminal;
    eh.Subscribe(PrinterStatusUpdate, &terminal);
    
    // start the print engine's state machine
    pe.Begin();
    // begin handling events
    eh.Begin();
    
    return 0;
}

