#ifndef GIGAPATCHEXPLORER_IMAGE_TILEDIMAGE_H_
#define GIGAPATCHEXPLORER_IMAGE_TILEDIMAGE_H_

#include <string>
#include <vector>

#include "common.h"
#include "imagesources/imagesource.h"

// Contains all information about an out-of-core, multi-resolution, tiled image representation.
// Currently works with images downloaded from Gigapan.com where each tile is in jpg format.
// Coarsest resolution level is indexed with 0.
struct TiledImageParams {
  std::vector<Size2DInt> tileres_per_level; // Number of tiles in x and y at each level.
  std::vector<Size2DInt> imgres_per_level;	// Pixel resolution of whole image at each level.
  Size2DInt tile_size;							        // Size of a single tile along x and y.
  int num_levels;								            // Number of resolution levels.
  int total_num_tiles;						          // Total number of tiles.
  std::string source_dir;					          // Source directory for image.
};

// Encapsulates an out-of-core, multi-resolution, tiled image data that resides in a single source
// directory i.e. all tiled images, and information are in source directory.
class TiledImageObject : public ImageSourceObject {

public:
  TiledImageObject();
  ~TiledImageObject();

  // Initializes to the image data found in sourceDir. Uses _info.txt inside sourceDir.
  // Returns true when successful.
  bool Init(std::string sourceDir);
  std::string GetTileFilename(int level, int tx, int ty);
  std::string GetTileFilenameFromGlobalCoords(int level, int global_x, int global_y);
  void ConvertGlobalXYPosToLocalTileXYPos(PatchCoords &patch_coords);
  TiledImageParams GetParamsCopy() { return params_; }
  std::string source_dir() { return params_.source_dir; }

  int num_levels() { return params_.num_levels; };
  Size2DInt tile_size() { return params_.tile_size; }
  Size2DInt imgres_for_level(int level) { return params_.imgres_per_level[level]; }
  Size2DInt tileres_for_level(int level) { return params_.tileres_per_level[level]; }
  std::vector<Size2DInt> tileres_per_level() { return params_.tileres_per_level; }
  int total_num_images() {
    return params_.total_num_tiles;
  };

private:
  TiledImageParams params_;
};

#endif  // GIGAPATCHEXPLORER_IMAGE_TILEDIMAGE_H_
