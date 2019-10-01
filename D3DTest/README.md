# D3DTest
This is a program of my own, which is used to test Direct3D functions.

It actully uses few features of Direct3D 9 (only LockRect), and all works are done by CPU.

Don't expect this program to work well.

You can press any key except L to draw a rectangle, and hold Space to immediately redraw when mouse moves. Press C to clear the scene with white.

In the time info, usr means CPU draw time, sys means Direct3D call time, and video means data transfer from RAM to VRAM time.

The program can be compiled with Visual Studio 2017. However, you can build it with MinGW:

	gcc *.c -msse2 -DUNICODE -Wall -lgdi32 -ld3d9 -mwindows -o D3DTest.exe
