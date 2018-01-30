#ifndef GIGAPATCHEXPLORER_COMMON_H_
#define GIGAPATCHEXPLORER_COMMON_H_

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <vector>

#define USE_QT
#ifdef USE_QT
#include <qsize.h>
#endif // USE_QT

struct Size2DInt {
  int width;
  int height;
  Size2DInt(const int w, const int h) : width(w), height(h) {}
  Size2DInt() : width(0), height(0) {}
};

#ifdef USE_QT
QSize Size2DIntToQSize(Size2DInt in_size);
#endif // USE_QT

enum SourceType {
  SourceType_TILEDIMAGE,
  SourceType_IMAGEDB,
  SourceType_PATCHSET,
  SourceType_INVALID
};

// This uniquely identifies a patch source.
struct SourceDesc {
  SourceType type;
  int id;
  SourceDesc() : type(SourceType_INVALID), id(-1) {}
  SourceDesc(SourceType _type, int _id) : type(_type), id(_id) {}
  bool operator==(const SourceDesc& other) const {
    return (type == other.type && id == other.id);
  }

};

// TODO (ronell): Change this!
struct SourceDescHashFunc {
  size_t operator() (SourceDesc const &value) {
    size_t k = 0;
    k += static_cast<size_t>(int(value.type));
    k *= 2531011;
    k += static_cast<size_t>(int(value.id));
    k *= 2531011;
    return k;
  }
};

struct PatchIdLevel {
  int id;
  int level;
  PatchIdLevel(int _id, int _level) : id(_id), level(_level) {}
  PatchIdLevel() : id(-1), level(-1) {}

  bool operator==(const PatchIdLevel& other) const {
    return id == other.id && level == other.level;
  }

  bool operator<(const PatchIdLevel& other) const {
    if (id < other.id) return true;
    if (id == other.id) {
      return (level < other.level);
    }
    return false;
  }

  
};

struct PatchTile {
  int tx;
  int ty;

  PatchTile(int _tx, int _ty) : tx(_tx), ty(_ty) {}
  PatchTile() : tx(-1), ty(-1) {}

  bool operator==(const PatchTile& other) const {
    return tx == other.tx && ty == other.ty;
  }

  bool operator<(const PatchTile& other) const {
    if (tx < other.tx) return true;
    if (tx == other.tx) {
      return (ty < other.ty);
    }
    return false;
  }

 
};

// TODO (ronell): Change this!
struct PatchTileHashFunc {
  size_t operator() (PatchTile const &value) {
    size_t k = 0;

    k += static_cast<size_t>(value.tx);
    k *= 2531011;
    k += static_cast<size_t>(value.ty);
    k *= 2531011;
    return k;
  }
};

struct PatchXY {
  int x;
  int y;
  PatchXY(int _x, int _y) : x(_x), y(_y) {}
  PatchXY() : x(-1), y(-1) {}

  bool operator==(const PatchXY& other) const {
    return x == other.x && y == other.y;
  }

  bool operator<(const PatchXY& other) const {
    if (x < other.x) return true;
    if (x == other.x) {
      return (y < other.y);
    }
    return false;
  }

 
};

// TODO (ronell): Change this!
struct PatchXYHashFunc {
  size_t operator() (PatchXY const &value) {
    size_t k = 0;

    k += static_cast<size_t>(value.x);
    k *= 2531011;
    k += static_cast<size_t>(value.y);
    k *= 2531011;
    return k;
  }
};

struct PatchCoords {
  int image_id;
  int level;
  int x;
  int y;
  PatchCoords(int _image_id, int _level, int _x, int _y)
    : image_id(_image_id), level(_level), x(_x), y(_y) {}
  PatchCoords() : image_id(-1), level(-1), x(-1), y(-1) {}
  PatchCoords(const PatchCoords& other) : image_id(other.image_id), level(other.level),
  x(other.x), y(other.y) {}
};

// TODO (ronell): Change this!
struct UShortHashFunc {
  size_t operator() (ushort const &value) {
    size_t k = static_cast<size_t>(value);
    return k;
  }
};

typedef std::vector<float> BinKey;

// Hash function used in this implementation. A simple base conversion.
// TODO (ronell): Replace this with boost hasher.
struct BinKeyHashFunc {
  size_t operator() (BinKey const &value) {
    size_t k = 0;
    for (int i = 0; i < int(value.size()); i++) {
      k += static_cast<size_t>(value[i]);
      k *= 2531011;
    }
    return k;
  }
};

struct BinKeyEqualFunc {
  bool operator() (BinKey const &left, BinKey const &right) {
    if (left.size() != right.size())
      return false;
    for (size_t s = 0; s < left.size(); ++s) {
      if (left[s] != right[s]) {
        return false;
      }
    }
    return true;
  }
};

struct QuantizedPatchCoord {
  PatchCoords coords;
  float distance;
  std::vector<int> entries_indices;
  QuantizedPatchCoord(PatchCoords _coords) : coords(_coords), distance(0.0f) {}
  QuantizedPatchCoord() : distance(0.0f) {}
};

inline bool QuantizedPatchCoordGreaterThan(const QuantizedPatchCoord& lhs, 
                                           const QuantizedPatchCoord& rhs) {
  return lhs.entries_indices.size() > rhs.entries_indices.size();
}

inline bool PatchCoordIdGreaterThan(const PatchCoords& lhs,
                                           const PatchCoords& rhs) {
  return lhs.image_id > rhs.image_id;
}

inline float ComputeSquaredDistance(PatchCoords& p1, PatchCoords& p2) {
  return ((p1.x - p2.x)*(p1.x - p2.x)) + ((p1.y - p2.y)*(p1.y - p2.y));
}

typedef std::unordered_set<BinKey, BinKeyHashFunc, BinKeyEqualFunc> BinKeySet;

typedef ushort Offset;
typedef std::unordered_set<Offset, UShortHashFunc> OffsetsSet;  // Stores unique offsets per tile.
typedef std::map<PatchTile, OffsetsSet> TileToOffsetsMap;       // Maps tile index to offsets.
typedef std::map<PatchIdLevel, TileToOffsetsMap> PatchIdLevelToTileMap; // Maps patch id and level
                                                                        // to TileToOffsetsMap.
typedef PatchIdLevelToTileMap PatchCoordsSet;               // Set of patch coords from one source.
typedef std::unordered_map<SourceDesc, PatchCoordsSet, SourceDescHashFunc> SourceDescToPatchSet;                                          // Maps source desc to patch set
                                                                // of a single source.
typedef std::pair<PatchTile, Offset> PatchTileAndOffsetPair;
typedef std::unordered_map<BinKey, int, BinKeyHashFunc> BinKeyToCountMap;
typedef std::pair<PatchIdLevel, PatchTile> IdLevelTileCoordsPair;

typedef std::map<IdLevelTileCoordsPair, QuantizedPatchCoord> IdLevelTileToQuantizedCoordMap;

// This is used to compress patch coordinates in one tile. Right now, we use a 256 x 256 
// spatial tile. We use the upper left corner coordinates as starting point and the offset
// is an index to the x and y offsets i.e. 8 bit offsets along x and y from the upper left corner.
const Size2DInt storage_tile_size = Size2DInt(256, 256);
inline PatchXY TileAndOffsetToXY(PatchTile tile, Offset offset) {
  int offset_int = static_cast<int>(offset);
  int offset_y = offset_int / storage_tile_size.width;
  int offset_x = offset_int % storage_tile_size.width;
  int coord_x = (tile.tx * storage_tile_size.width) + offset_x;
  int coord_y = (tile.ty * storage_tile_size.height) + offset_y;
  return PatchXY(coord_x, coord_y);
}

inline PatchTileAndOffsetPair XYToTileAndOffset(int x, int y) {
  PatchTile tile(x / storage_tile_size.width, y / storage_tile_size.height);
  int within_tile_coord_x = x - (tile.tx * storage_tile_size.width);
  int within_tile_coord_y =  y - (tile.ty * storage_tile_size.height);
  int offset_int = (within_tile_coord_y * storage_tile_size.width) + within_tile_coord_x;
  Offset offset = static_cast<Offset>(offset_int);
  return std::pair<PatchTile, Offset>(tile, offset);
}

struct ROI {
  int upper_left_x;
  int upper_left_y;
  Size2DInt size;
  ROI(int x, int y, int w, int h)
    : upper_left_x(x), upper_left_y(y), size(Size2DInt(w, h)) {}
};

// Computes the intersectin of two ROIs. Returns true if inputs intersect.
static bool IntersectROIs(ROI r1, ROI r2, ROI& out) {
  int xmin = std::max<int>(r1.upper_left_x, r2.upper_left_x);
  int xmax1 = r1.upper_left_x + r1.size.width;
  int xmax2 = r2.upper_left_x + r2.size.width;
  int xmax = std::min<int>(xmax1, xmax2);
  if (xmax > xmin) {
    int ymin = std::max<int>(r1.upper_left_y, r2.upper_left_y);
    int ymax1 = r1.upper_left_y + r1.size.height;
    int ymax2 = r2.upper_left_y + r2.size.height;
    int ymax = std::min<int>(ymax1, ymax2);
    if (ymax > ymin) {
      out.upper_left_x = xmin;
      out.upper_left_y = ymin;
      out.size.width = xmax - xmin;
      out.size.height = ymax - ymin;
      return true;
    }
  }
  return false;
}

inline bool AreStringsTheSame(std::string &lhs, std::string& rhs) {
  return lhs.compare(rhs) == 0;
}

class SourceDirToSourceDescMap {
public:
  SourceDirToSourceDescMap(SourceType source_type) : source_type_(source_type) {}

  bool SourceExists(std::string source_dir) {
    std::map<std::string, SourceDesc>::iterator it = dir_to_desc_map_.find(source_dir);
    return (it != dir_to_desc_map_.end());
  }

  // IMPORTANT: For correct id, call this before pushing to vector!
  void AddSource(std::string source_dir, SourceDesc source_desc) {

    if (!SourceExists(source_dir)) {
      dir_to_desc_map_.insert(std::pair<std::string, SourceDesc>(
        source_dir, source_desc));
      assert(source_desc.id == int(desc_id_to_dir_vec_.size()));
      desc_id_to_dir_vec_.push_back(source_dir);
    }
  }

  std::string GetSourceDir(SourceDesc source_desc) {
    if (source_desc.id >= int(desc_id_to_dir_vec_.size()) || source_desc.type != source_type_) {
      assert(!"Invalid source desc.\n");
      return std::string("");
    }
    return desc_id_to_dir_vec_[source_desc.id];
  }

  int GetSourceDescId(std::string source_dir) {
    std::map<std::string, SourceDesc>::iterator it = dir_to_desc_map_.find(source_dir);
    if (it == dir_to_desc_map_.end()) {
      return -1;
    }
    return int(it->second.id);
  }

private:
  SourceType source_type_;
  std::map<std::string, SourceDesc> dir_to_desc_map_;
  std::vector<std::string> desc_id_to_dir_vec_;
};

struct FocusPatchParams {
  Size2DInt patch_size;
  bool enabled;
  PatchCoords coords;
};

#endif  // GIGAPATCHEXPLORER_COMMON_H_
