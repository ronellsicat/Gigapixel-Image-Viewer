# README #

* Quick summary
This is a simple gigapixel image viewer implemented as a Qt widget.

### How do I get set up? ###

* Set up

Use Cmake to generate an MS VS solution (tested on VS 2013 only).

Here is an example CMAKE set up:
![cmake.png](https://bitbucket.org/repo/5rKxxb/images/2492520297-cmake.png)

* Dependencies

Qt 5.4

* How to run tests:

Run the binary, press "o" then open the folder where your image data is. See example image folder: gigapixel_example. The folder contains the tiled images. Each image has name level-tile_coord_x-tile_coord_y.jpg. Each folder has an _info.txt file which indicates:

tile size
number of levels
number of tiles in x   |   number of tiles in y      |    full image width       |      full image height    (for each level)