# graphics_final
### Stanley Lin and Anthony Hom
### Period 4
A primitive graphics engine written in C

# Features
* Vertex normals
* Ray tracing
	* Moller Trombore Ray-Triangle Intersection
	* Reflections
	* Refraction
	* Shadows

Different parameters for the program can be edited in the options struct found in `my_main.c`.

Ray tracing is a computationally intensive method of image generation, so rending an image will take a few minutes.

Boxes are treated differently from tori and spheres in that they act as an "opaque mirror." This was used to easily demonstrate the reflection and shadows.

Acceleration structures weren't added to this engine. As such, please be conservative with the amount of shapes added to the scene (especially for spheres and tori).

Animation was not tested with this. However, given the amount of time needed to generate one frame, animation is not recommended.

#Bugs
* The front polygon of boxes doesn't render
