truss-genetic v1.01

SUMMARY
This is a computerised approach to creating an optimal planar truss designed to carry a point load using a genetic algorithm. 
This application is NO LONGER being worked upon and is merely an example of using genetic algorithms for truss building.
Do not expect it to be optimised completely, or suitable for commercial applications or serious truss building.

LICENSE
Copyright (c) 2016 Tim Finucane

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and 
 associated documentation files (the "Software"), to deal in the Software without restriction, 
 including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, 
 subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or 
 substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 THE USE OR OTHER DEALINGS IN THE SOFTWARE.

INSTALLATION AND USE
To use simply download and compile the application (changing any compile-time settings as noted below beforehand),
 and give a seeding value for the random number generator.
This application has been built and tested only in Visual Studio 2015.

COMPILE-TIME SETTINGS
The following are some useful constant values in the application that can be modified to produce different results:
 - TIME, main.cpp. Determines the time in seconds the algorithm will run for
 - FAMILY_SIZE, main.cpp. Determines the initial size of the population the algorithm will then try and maintain.
 - INTENSITY, truss.cpp. Determines the weighting attributed to the maximum load capacity to determine fitness.
 - MAXIMUM_TENSION, MAXIMUM_COMPRESSION, truss.h. Functions and values determining the maximum forces a given member
    can experience before breaking.
 - MAX_MEMBER_LENGTH, truss.h. Maximum length of any member in the application.
 - MAX_THICKNESS, truss.h. Maximum number of sticks, or sum of thicknesses that can be attached to a node.

AUTHORS
Tim Finucane, timfinucane@outlook.com
