# macos_keylogger

A keylogger for macOS that makes use of the system CoreFoundation, CoreGraphics, and Carbon frameworks. The program must be run as an elevated process because Quartz event taps must be run as root in order to capture keyboard events.

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes.

Compile the programe with the following flags set.

```
 gcc -o keylogger.o keylogger.c -framework CoreGraphics -framework CoreFoundation -framework Carbon
```

### Prerequisites

A system running OS X version 10.4 up to macOS High Sierra (current)
and the GNU Compiler Collection (GCC).

### Installing

Enter the project parent directory and compile the project with the following flags set.

```
 gcc -o keylogger.o keylogger.c -framework CoreGraphics -framework CoreFoundation -framework Carbon
```

This will create an executable object file named "keylogger.o" in the current directory.

### Running the Program

After successful compilation of the project, execute the following to run the executable object file.

```
sudo ./keylogger
```

In macOS, keyboard events are considered secure input events. This means that while the project will still run without elevated privileges, the event tap it utilizes will not be able to deliver key press events up to the main thread's run loop where they are processed and logged.

Furthermore if you are using OS X 10.4, then "Access for assistive devices" must be enabled. On macOS High Sierra, the project will work by default.

This project is uploaded for educational purposes, so others can have an idea of how to go about the research and documentation hunting required to make something like this. Apple is notorious for their unclear documentation habits. In the context of security, there are decent arguments both for and against these habits, but I will not get into those. The point is that I hope this will help give some people an idea of the kind of detective work is needed when confronted with an absence of official information. Google is your friend.

This was not uploaded so you can do something illegal or immoral.

It is also important to note that the project cannot log keystrokes in secure text fields, like those used for password input. In macOS High Sierra, the AppKit framework provides a NSSecureTextField class, which inherits from the NSTextField class. Apple being Apple, the documentation on how this works is sparse. I assume however that it provides a similar functionality to the EnableSecureEventInput function provided by the depreciated Carbon framework, which essentially creates a secure communication channel where keyboard events are delivered to and only to the process associated with the textfield identified for secure input entry. Any attempt by another process (i.e. our keylogger) to access these events will be blocked. This is not a security feature specific to macOS; I do not know of an operating system that does not employ something like this.

Keystrokes will be logged to the file "keystroke.log" found in the project directory.

## How it Works

When you register an event tap, you supply a bit mask that identifies the set of events to be observed. To create the bit mask, use the CGEventMaskBit macro to convert each constant into an event mask and then OR the individual masks together.

```
CGEventMask trackedEvents = CGEventMaskBit(kCGEventKeyDown) | CGEventMaskBit(kCGEventFlagsChanged);
```

Before going further you should have a basic understanding of mach ports and how they are used by macOS.

Mach ports are a kernel-provided inter-process communication (IPC) mechanism that is used extensively throughout macOS. They are unidirectional kernel-protected channels and can have multiple send-points but only one receive point.

Mach undertakes multiple roles in macOS, but in the context of this program it provides object-based APIs with communication channels (i.e. ports) as object references.

A Core Foundation mach port represents the new event tap, or NULL if the event tap could not be created. We pass a callback function, keyPressed, to the event tap, which contains logic to handle key press events and write unicode characters associated with those events to a log file.

```
CFMachPortRef eventTap = CGEventTapCreate(kCGSessionEventTap, kCGHeadInsertEventTap, kCGEventTapOptionDefault, trackedEvents, keyPressed, NULL);
```

The newly created event tap will register events if both of the following conditions are met:
1) The current process is running as the root user.
2) If you are on OS X 10.4, "Access for assistive devices" is enabled. You can enable this feature using System Preferences, Universal Access panel, Keyboard view. Ignore this if you are using macOS High Sierra.

Now that we have our event tap, we need to add it to our thread's run loop. CoreFoundation provides run loop objects that allow you to configure and manage a thread's run loop.

There is a lot of documentation out there on how run loops work in macOS, but this explanation will only cover how they are used in context of this code. Getting into the details is beyond the scope of this explanation, but I encourage you to look that up on your own, they are not very difficult to understand.

The event tap can be passed to the CFMachPortCreateRunLoopSource function to generate a CFRunLoopSource object, which is is an abstraction of an input source (the event tap) that can be put into a run loop. A CFRunLoopSourceRef structure must be used to reference the CFRunLoopSource object.

```
CFRunLoopSourceRef runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0);
```

Next, we will take that CFRunLoopSource object and add it to a run loop mode. A run loop mode is a collection of input sources and timers to be monitored and a collection of run loop observers to be notified. When a run loop is run, a mode in which it is run is specified, either explicitly or implicitly. During a pass of the run loop, only input sources, such as a mouse click or a keypress, associated with that mode are allowed to deliver their events. If an input source is not allowed to deliver its event during the current pass, it will wait for a subsequent pass of the run loop to enter a mode in which delivery is permitted.

Modes are used to filter out events from unwanted input sources during a particular pass through a run loop. In our case we are not interested in filtering out events but instead in making sure that events tapped by our input source, a CFRunLoopSource object, can be delivered to our run loop regardless of the mode it is currently in.

We add our input source to the run loop with the CFRunLoopAddSource function and pass it the constant CFRunLoopCommonModes, a collection of commonly used modes defined by the CoreFoundation framework, to ensure that our input source will deliver key press events in these modes.

```
CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);
```

The CFRunLoopRun function runs the run loop we previously configured indefiniely in our processes's current thread. The run loop may be stopped with a call to the CFRunLoopStop function or if the process is ended/killed.

```
CFRunLoopRun();
```

While this is does not cover everything in the code (such as how the TIS functions are used to process keyboard events in preparation for logging), it does explain the core concepts and functions behind it.

## Built With

* [CoreFoundation](https://developer.apple.com/documentation/corefoundation) - Framework that provides fundamental software services useful to application services, application environments, and to applications themselves.
* [CoreGraphics](https://developer.apple.com/documentation/applicationservices) - Framework based on the Quartz advanced drawing engine.
* [Carbon](https://developer.apple.com/library/content/navigation/index.html?filter=carbon) - Framework that provides TIS functions to translate key codes into Unicode characters (Carbon is depreciated and Apple no longer provides most of the documentation for it, so going through its header files and some googling is necessary).

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details
