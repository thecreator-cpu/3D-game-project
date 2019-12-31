
---Milestone 2---
Anne - L-system

New Class:L-system
New functions in terrain class: makeRiver(), makeDeltaRiver()

First I used a L-system visualizer online to find the rules that makes a desired river shaped. I made a L-system class that stores a terrain and functions to draw on the terrain with a given string. The class also has functions for expanding a string with the rules and functions that correspond to each character in the string such as storing turtle, rotating, draw line.  I implemented a data structure for the turtle, which stores position, rotation and width. The width is decreased when a turtle is stored and increased when turtle from stack is popped. Also, There are different rules for the main river and the delta river. 

The depth of the water is determined by the square of their distance to the shore.  The river is placed at 128 and the terrain above and near it are carved out.  I used a ray march function to carve out the land perpendicular to the river’s direction. 
Inside the terrain class, I made 2 functions for making the 2 rivers, which calls the functions in L-system. 

It was difficult to get the river’s shape to look good at first because we need to determine the length of the increment and the rotation angle.  I had to test many different numbers to understand how each parameter affects the look. 
Also, determining how to carve out the river so that it looks natural was difficult.  By using ray marching, I found all the blocks perpendicular to river’s direction and decreased their height. 



Catherine - Texturing and Texture Animation

Texture & normals
I used slot 0 for textures and slot 1 for normals, since using different slots would require images to be loaded only once. I also added the ìinî UV attribute to determine where within the normal map to read the UV. I had to change the interleaf VBO from storing vec4ís to storing floats in order to enter UV as a vec2, and later to store cosine power and the animateable flag. Based on the block type, I assigned different UVs, and also differentiated the top, bottom, and side UVs to create grass and wood blocks.  I also attempted to add normals by transforming the initial normal map, which represents the side with normals (0, 0, 1), to each side. But since the initial terrain is generated with the front having a normal of (0, 0, -1), and the back having (0, 0, 1), and assumes that the z axis goes backwards rather than forward, I had to make adjustments to the usual transformations to create a rendering that makes sense.

Transparency
I created two VBOs, one to hold non-transparent blocks and one to hold transparent blocks. Based on the block type, they are either added into the opaque VBO or the transparent VBO. In the same draw function in ShaderProgram, I rendered the opaque blocks first, then the transparent blocks.

Texture Animation
I added an animateable flag to the end of the interleaf vector as the 16th element. For water and lava, animateable is set to 1, and for others it is set to 0. I also added a uniform Time variable, set by timerUpdate based on the number of milliseconds since Epoch. In frag.glsl, when animateable = 1, the time elapsed from the start of the program is scaled to be 1/10,000th of the original integer, or 1/10th of a second. Then it mods 1/16, which maps the number to a value between 0 and 1/16 to set as offset for x. This way, each fragment in the river would move in the x direction from the given UVís x to x+1/16 based on time.

Blinn Phong
Blinn Phong shading uses cosine power, the light direction, and the view vector.  Light direction is constant. The cosine power is added to the interleaf vector as the 15th element. For shinier elements like water and ice, I assigned a cosine power of 15, while for duller elements like dirt and grass, I assigned a power of 1.5. To calculate the view vector, I added a uniform variable CameraPosition to vert.glsl, which is set every time the user moves and the camera position is recomputed. Then CameraPosition is used to compute the view vector of the current vertex by doing CameraPosition - u_Model * vs_Pos, which would be the world camera position subtracted by the world vertex position.

Spencer - Swimming and Multithreaded Terrain Generation

Swimming Physics
This was the easiest part. After reading my partner's player physics code multiple times, I finally understood their logic so that I could edit it such that velocity and acceleration were two-thirds of what it usually is. In my partner's velocity and acceleration update functions, I simply multiply the velocity and acceleration by 2/3, but also reset the acceleration to what it was orginally, otherwise the acceleration would consantly decrease. This is problematic because then the acceleration would approach zero rendering the user completely motionless.

Swimming Color Distortion
In order to achieve thi effect, I first attempted to use a post-processing render pass like how we did in OpenGLFun since I thought this was mandatory. However, after many attempts at fixing my post-processing render pipeline so that it would render something instead of nothing I found a Piazza post saying that we don't have to use post-processing. So, instead I followed advice in the Piazza post and created a colored, semi-transparent quad that was set to a particular color depending on whether the player was in water, lava, or empty blocks. I didn't apply a model matrix to the quad since it should always be directly in front of the user at all times.

Multithreading and Mutexes
This gave me more trouble than I expected. It was crashing for unknown reasons and would sometimes throw errors (P.S. if it crashes immediately after attempting to run, then just try to run it again; this usually works). I created a thread object in the terrain class. Each thread was responsible for computing the fbm function in parallel for each surface block within 4 chunks of the 16 chunks. This was working fine but then I was having trouble with the initially generated terrain objects. Specifically with how the river is generated on top of these terrain objects. I think the generate river function and the fbm function were editting the terrain at the same time, which caused incorrect assigning of blocks. But I tried to fix this by having these initial terrain objects generate fbm data sequentially since this won't really affect gameplay since the game hasn't really started, but now my game crashes whenever I attempt to cross a boundary.


---Milestone 1---

Anne - Procedural Terrain
Terrain Generation (fbm)
I used the fbm method from lecture slides. The maximum highest is 140 because the rock goes to y = 128,  maxY = 140 allows  enough flat space for player to walk on. The terrains contains variable that stores the x and z position. This is necessary so that when drawing the terrain,the model matrix can be set to a translation matrix to shift the terrain. 

All the terrains are stored in a std map that maps a int pair(x and z position) to a terrain unique pointer. This way, we can check whether a terrain exist and decide if we have to create it. Also, we can find the block at any world coordinate by first determining the terrain a coordinate is in by using divide by 64 and floor. Then use modulo to decide local position within terrain.

New Terrain on playerís border:
I test both x and z direction to see if any side is close to the border, and generate the terrain close to that border. Additionally, I test if both x and z are near the border, which generates terrain in the diagonal.


Raycast
Method: distToBlock
Returns: Distance to first non empty block. -1 if none found.
It was written in MyGL because we need to access all the blocks in the world. 

addBlock/deleteBlock functions
These functions calls distToBlock() which returns the t value. Using t, we can find the position of the block it hits. For delete, lerp 0.001 from block position in the direction of ray so that the ray is inside block and we know which one to delete.For add, lerp 0.001 from block position in the opposite direction of ray and that gives the position of the box to add.




Spencer - Efficient Terrain Rendering & Chunking
Chunk class
The chunk class is an inner class of the Terrain class since it is an internal data structure that should not be manipulated directly unless the programmer knew exactly how everything was indexed inside of the class.
I gave my group mates functions that gave them access to the underlying chunking data structure so that they would not have to directly interact with the chunking data structure inside of the terrain objects
The Chunk class has a data member of type BlockType array of size 65536. I used a 1D array instead of a 3D array to represent the contents of the array because this would provide somewhat faster access time .
Leveraged the error throwing properties of the at() method instead of the [] operator when accessing maps and arrays to ensure that we can catch any accidental out of bounds access. For even more debugging assistance, I wrapped these at methods inside try-catch blocks which gave a detailed error message whenever out-of-bounds exceptions occurred.

Edited and expanded the terrain class functionality so that all of its functions still work in the case when there are multiple terrain objects.

Computed which faces of the cubes comprise the hull of the terrain to drastically increase render time. I then populate the CPU vectors with the corresponding position, normal , and color attributes of each vertex defining a visible face. Instead of dividing each of these attributes into 3 VBOs on the GPU, I combined them into a single interleaved VBO where all of the vertex attributes of a single vertex are contiguous. This allows for somewhat faster render times.

Removed the m_blocks 3D array and replaced with a map of uint6_t4 to uPtr<Chunk>. Since tuples and ivec2s donít have hash functions for mapping, I used the x, z position of the chunk relative to the chunkís corresponding terrain. I combined these two 32-bit integer values into a single 64-bit integer so that I can get automatic hashing. I was concerned about this implementation because I was unsure of how bitwise operators would behave on data types of different sizes. Specifically, I was concerned if C++ uses sign-extension which could potentially mess up how I combine binary numbers. To ensure that the key that I was calculating was correct, I wrote a recursive function that prints the binary representation of numbers.

I then needed to figure out how to modify the ShaderProgram and Drawable classes. I used the existing code as a template to determine which functions to call and in which order. I also changed how the lambert vertex shader determined which color to draw boxes. Instead of having a uniform value color that is set inside of mygl.cpp based on the block type, the shader uses an in vec4 for color that I set with the interleaved VBO.



Catherine - Game Engine Update Function and Player Physics
Character Dimensions
The character is 1x2 and aligned with the world axis. The characterís height is set to 1.8f so that the eyes are towards the top of the top block, but not at the exact top. This way, when the character collides at the top (in a cave or indoor environment), they can still look up and see what they are colliding with.

Camera Angles
I made sure to clamp the y camera angles to -89 degrees to 89 degrees to reduce jitteriness, and to ensure there is always a horizontal component that the character can walk towards.

Using Elapsed Time
The acceleration changes according to time elapsed. It is scaled by elapsed_time*base_accelartion/16.f since the original velocity is created for an average update time of 16ms. The scaling multiplies the number of actual milliseconds elapsed with the velocity per millisecond. Then velocity is calculated based on the acceleration*time using velocity=velocity+acceleration*time.

Aerial State
I decided to keep track of whether the character is aerial, and store it in a boolean in Player. The aerial state would tell us when to set a negative velocity, and remove the necessity of raycasting downwards constantly. Also, having an aerial state assists with jump. When a character is aerial, that character cannot jump and gain positive velocity from it. To check whether a character is aerial, in timerUpdate, I checked the blocks beneath the four corners on the base of the character. This way, if either one of the characterís corners is still on a non-empty block, the character should not fall. This ensures compliance with collisions.

Collisions
We decided to collaboratively write one raycast function distToBlock() in MyGL that takes in the origin of the raycast, the direction of the ray, and the max distance that it casts for. This is because raycast is used in both adding and removing blocks, and in player physics. Using the global raycast function, I casted rays of length current velocity from all 12 points (base, middle plain, top) to detect collision with world blocks. The minimum collision distance from the 12 points was used as the travel distance.


