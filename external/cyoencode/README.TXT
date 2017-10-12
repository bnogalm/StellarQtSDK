===============================================================================
CyoEncode
http://cyoencode.sourceforge.net/

Copyright (c) 2009-2014, Graham Bull. All rights reserved.
===============================================================================

Version 1.1.1
Release Date 25th February 2014

-------------------------------------------------------------------------------
1. License
-------------------------------------------------------------------------------

CyoEncode is made available under the terms of the Berkeley Software
Distribution (BSD) licence, as detailed in LICENSE.TXT. This allows you
complete freedom to use and distribute the code in source and/or binary form,
as long as you respect the original copyright.

-------------------------------------------------------------------------------
2. Instructions
-------------------------------------------------------------------------------

Simply copy the required source files (CyoEncode.h/cpp and CyoDecode.h/cpp)
into your C/C++ project.

Examples of usage can be found in the test.c file.

For Unix/Linux developers, there's a shell script that will build the test
using GCC.

For Windows developers, Visual Studio projects are included.

-------------------------------------------------------------------------------
3. Release Notes
-------------------------------------------------------------------------------

1.1.1 - 25th February 2014
- A terminator char shouldn't be appended to the output when decoding.

1.1.0 - 20th February 2014
- Added the cyoBase??EncodeBlock() and cyoBase??DecodeBlock() functions.
- Added VS2012 and VS2013 project files.

1.0.2 - 5th January 2012
- A little refactoring, added some shared functions.
- Added VS2010 project file.
- Added x64 build configurations.

1.0.1 - 25th September 2009
- Added the cyoBase??Validate() functions.
- Added detection of invalid encodings in the cyoBase??Decode() functions,
  rather than relying on assertions.

1.0.0 - 19th August 2009
- First release.

