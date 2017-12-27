##### 12/17 - Rendering subsystem -WIP-
- More window subsystem work, started work on rendering subsystem, fixed a deadlock
- Fixed crash caused by negative keycodes in the event input objects
- Added wrappers around opengl functions (including shaders, textures, buffers and various enums)
- Added 1D, 2D, 3D, 1D_array and 2D_array textures (loading, uploading to gpu and options)
- Added some utility objects (Spherical coords, FPSCamera etc.)
- Added serialization for maths objects
- Added msgpack support
- Added brotli compression support
- Started work on animation/model subssystem and file format specifications [Will require blender export script]
- Various subtle datatype tweaks around the place

##### 10/17 - Window subsystem
- Start implementing windowing subsystem
- Replace reinvented vector math library with GLM
- Completely rework Property Tree to be more functional

##### 08/17 - Redesign virtual filesystem 
- Redesign virtual filesystem to use a more C-like style.
- Added Property Tree (Tree data structure, used for representing key/value data formats agnostically)

##### 07/17 - Project Started
- Started work on small core subsystems (threading, logging eg.)
- Started work on the various utility classes 
