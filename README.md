# graphics_final
### Stanley Lin and Anthony Hom
### Period 4
A primitive graphics engine written in C

Just run `make` to compile and generate the test image. Rendering will take a while (~10 min on an i7 processor).

Different parameters for the program can be edited in the `options` struct found in `my_main.c`.
* `fov`: field of view, how much of the scene is visible, max of PI
* `recursion_depth`: how detailed reflections/refractions will be
* `bias`: used to minimize artifacts (bigger values elimate more, but shift shadow position)
* `camera_depth`: position of the camera in the world
* `bkgd_color`: color of the background

# Features
* Vertex normals
	* Smooth shading
* Ray tracing
	* Reflection
	* Refraction
	* Shadows
* MDL `behavior` command: define different object properties

### MDL Functionality
The additional MDL functionality works with spheres and tori. Use `behavior ...` after all other arguments.

Ex: `sphere 0 0 0 1 behavior diffuse`

The following properties are supported:
* `diffuse`
	* Define an object as diffuse (matte)
* `reflect_refract`
	* Define an object as both refractive and reflective
* `reflect`
	* Define an object as only reflective

Note that the additional MDL functionality doesn't work with other additional commands that may be used with the two shapes, as the other functionality was not used for this project.

### Additional Notes
Ray tracing is a computationally intensive method of image generation, so rendering an image will take a few minutes.

Boxes are treated differently from tori and spheres in that they act as an "opaque mirror." This was used to easily demonstrate the reflection and shadows.

Acceleration structures weren't added to this engine. As such, please be conservative with the amount of shapes added to the scene (especially for spheres and tori).

Animation was not tested with this. However, given the amount of time needed to generate one frame, animation is not recommended.

# Bugs
* The front polygon of boxes doesn't render
* There are some artifacts on the reflection of the torus
* Random shadows may pop up in places
* Very hard to distinguish between `reflect` and `reflect_refract` behavior
