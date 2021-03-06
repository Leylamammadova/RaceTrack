# Race Track

------------------

Key Bindings:

| Command  | Key |
| ------------- | ------------- |
| Switch to QUADRATIC_BEZIER  | F1  |
| Switch to CUBIC_BEZIER  | F2  |
| Switch to CATMULL_ROM  | F3  |
| Randomise Track | F5  |
| Save Track | F6  |
| Toggle Debug mode  | Space bar  |
| Increase Track Length  | Up Arrow  |
| Decrease Track Length  | Down Arrow  |
| Increase Track Width  | Right Arrow  |
| Decrease Track Width  | Left Arrow  |
| Increase Height Amplitude  | F8  |
| Decrease Height Amplitude  | F7  |
| Increase Mesh Detail  | F10  |
| Decrease Mesh Detail  | F9  |


# octet

Octet is a framework for teaching OpenGL and the rudiments of game programming such
as Geometry construction, Shaders, Matrices, Rigid body Physics and Fluid dynamics.

It has a number of examples in the src/examples directory.

To use with visual studio, fork this repository into your own account and then
"Clone Into Desktop" using the GitHub tool and open one of the .sln files in src/examples.

There is a python script for generating your own projects from a template.

From the octet directory run:

packaging\make_example.py my_example

To create your own project in src/examples

Examples should also work with Xcode, although testing is a lot less thorough. If it does not work, send
me a pull request with fixes, please...

Octet is a bit unusual in that it does not use external libraries such as libjpeg or zlib.
These are implemented in source form in the framework so that you can understand the code.
The source of most academic libraries is almost unreadble, so we aim to help your understanding
of coding codecs such as GIF, JPEG, ZIP and so on.
