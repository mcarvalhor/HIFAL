# HIFAL
HIFAL Is Fast And Light is a very fast and light HTTP server. It focuses mainly on memory and CPU usage, and it's perfect for use when a web service isn't the main purpose of a server. You can, for instance, use it on if you need to provide a brief explanation for the end user on the server purpose on a browser.

## Why do I need this?

You may need a very light HTTP Web Server if the main purpose of the server isn't web or if you need to link it to your code. For example:
- Gaming servers, in which resources should be dedicated mainly to gaming processes (instead of a huge web server). You can use HIFAL to provide browser instructions to your end users, so that they know how to use the server for gaming services.
- Personal computers, when you just want to share some files with other people without installing a huge web server.
- As a Web Server for your program. You can link HIFAL's libraries to your program, so that it serves a specific directory that your program works on. For example, if you are coding a photo editor and you want to provide a "Share on web" button without using your own infrastructure, HIFAL may be very useful.
- IoT or Embedded Devices, in which memory usage may be a problem. Remember, though, that HIFAL conforms to POSIX Operating Systems only. If you are to use it on a embedded device, maybe you need to code more to get it working.

## How to compile?

You can compile HIFAL using make. Just execute "make all" in a terminal.

## How to run?

You can run HIFAL using a terminal. Just execute the compiled binary "./hifal".

## How to link to my code?

When you compile HIFAL using "make all", the object file "obj/hifal.o" will be created. You can link this object file to your program and use header file "headers/hifal.h".

## How to contribute?

You have to follow some steps in order to contribute to the project:
1. Open a new Issue, describing the necessary contribution.
2. Fork the repository.
3. Do whatever editions in the forked repository.
4. Open a new Pull Request, with reference to the opened Issue.

