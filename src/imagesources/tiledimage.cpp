#include <fstream>
#include <iomanip>
#include <sstream>

#include "imagesources/tiledimage.h"

TiledImageObject::TiledImageObject() {}

TiledImageObject::~TiledImageObject() {}

bool TiledImageObject::Init(std::string sourceDir) {
  params_.source_dir = sourceDir;

  std::stringstream ss;
  ss << params_.source_dir << "/_info.txt";
  std::ifstream f(ss.str().c_str());
  if (!f.is_open()) {
    printf("ERROR: Cannot find info file %s.", ss.str().c_str());
    return false;
  }

  std::string curLine;
  getline(f, curLine);
  std::stringstream s(curLine);
  std::string temp;
  int cur_index = 0;
  while (s) {
    if (cur_index == 0) {
      s >> temp;
      params_.tile_size.width = std::atoi(temp.c_str());
      cur_index++;
      if (s.eof())
        break;
    } else {
      s >> temp;
      params_.tile_size.height = std::atoi(temp.c_str());
      cur_index++;
      if (s.eof())
        break;
    }
    if (cur_index == 2) {
      break;
    }
  }
  if (cur_index == 1) {
    params_.tile_size.height = params_.tile_size.width;
  }
  
  if (params_.tile_size.width == 0 || params_.tile_size.height == 0)
    return false;

  getline(f, curLine);
  s = std::stringstream(curLine);
  s >> temp;
  params_.num_levels = std::atoi(temp.c_str());
  params_.total_num_tiles = 0;
  for (int l = 0; l < params_.num_levels; ++l) {

    getline(f, curLine);
    s = std::stringstream(curLine);
    Size2DInt tileRes(-1,-1);
    Size2DInt levelRes(-1,-1);
    s >> temp; tileRes.width = std::atoi(temp.c_str());
    s >> temp; tileRes.height = std::atoi(temp.c_str());
    s >> temp; levelRes.width = std::atoi(temp.c_str());
    s >> temp; levelRes.height = std::atoi(temp.c_str());
    params_.tileres_per_level.push_back(tileRes);
    params_.imgres_per_level.push_back(levelRes);
    params_.total_num_tiles += tileRes.width * tileRes.height;
  }

  f.close();

  printf("\nTiledImageObject created from %s with tile size %d x %d,"
         "%d levels, and %d total tiles.\n",
         params_.source_dir.c_str(), params_.tile_size.width, params_.tile_size.height, 
         params_.num_levels, params_.total_num_tiles);
  return true;
}

std::string TiledImageObject::GetTileFilename(int level, int tx, int ty) {
  std::stringstream tilefname;
  tilefname << params_.source_dir << "/";
  tilefname << std::setw(4) << std::setfill('0') << level << "-"
    << std::setw(4) << std::setfill('0') << tx << "-"
    << std::setw(4) << std::setfill('0') << ty << ".jpg";
  return tilefname.str();
}

std::string TiledImageObject::GetTileFilenameFromGlobalCoords(int level, 
                                                              int global_x, int global_y) {
  int tx = global_x / tile_size().width;
  int ty = global_y / tile_size().height;
  return GetTileFilename(level, tx, ty);
}

void TiledImageObject::ConvertGlobalXYPosToLocalTileXYPos(PatchCoords &patch_coords) {
  patch_coords.x = patch_coords.x % tile_size().width;
  patch_coords.y = patch_coords.y % tile_size().height;
}