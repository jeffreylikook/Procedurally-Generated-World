# COMP371_Project
Repository for Comp 371 to procedurally generate a world

2. Walk Through a Procedurally Modeled World
The goal of this project is to create a graphics program using OpenGL for walking through
a procedurally created virtual world, say a forest of trees, plants, bushes, etc., or an urban
city with roads, buildings, etc. All repeating items, like the trees in the virtual forest or the
buildings in the virtual city, should be procedurally created, using a few user specified
parameters, if necessary. The terrain has to be procedurally created. You are allowed to
borrow digital assets for nonrepeating items, like a bridge, for textures, and so on, as
needed in order to create the virtual world. However ensure that you credit the source
properly. You should have simple interaction mechanisms to navigate through the virtual
world and view it from different angles. During navigation, the user must not be allowed
to pass through objects. 

How to start
-------------
- Download Project
- Add assets/Models folder https://drive.google.com/drive/folders/10g8iQjlGi6Igzah1Z2KmhXIoVW4upYAJ?usp=sharing
- Add Assets for cube, cylinder, sphere
- Run Project


Functionality
-------------
- Create a GLFW window 1024x768
- resizable window
- render models in window
- Enable Backface culling
- Create a 60x60 grid
- Create procedurally generated terrain
- Create procedurally generated elements
- Place multiple virtual cameras in the world space
- Shadow mapping
- Phong Illumination
- Texture Mapping


Inputs
-------
- [A] move camera in -x direction
- [W] move camera in -z direction
- [S] move camera in z direction
- [D] move camera in x direction

- [1] Change camera 1
- [2] Change camera 2
- [3] Change camera 3

- [<] Change World orientation around x axis
- [>] Change world orientation around -x axis


- [x] Toggle Models
