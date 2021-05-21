#ifndef _BIGMAP_H
#define _BIGMAP_H

#include "VGA_t4.h"
#include <vector>
#define MAX_TILES = 512;

class Tilelist{
public:
  uint8_t tile_size_px;
  uint16_t num_tiles;
  uint16_t max_tiles;
  uint16_t tile_size_bytes;
  vga_pixel* pixels;

  Tilelist(uint16_t _tile_size_px, uint16_t maxtiles);
  void add_tile_with_color(uint8_t _color, bool _dotted);
  void add_tile(vga_pixel*);
  vga_pixel* get_tile(uint16_t _index);
};

class Tilemap{
public:
  uint16_t* tiles;
  uint16_t  num_rows;
  uint16_t  num_cols;

  Tilemap(uint16_t _num_cols, uint16_t _num_rows);
  void setTile(uint16_t _col, uint16_t _row, uint16_t _index);
  uint16_t get_tile_index(uint16_t _col, uint16_t _row);
};

class Viewport{
public:
  Tilemap* tilemap;
  uint16_t inner_x_offset_px;
  uint16_t inner_y_offset_px;
  uint16_t x_px;
  uint16_t y_px;
  uint16_t w_px; 
  uint16_t h_px;
  Viewport(Tilemap* _tilemap, uint16_t _inner_x_offset_px, uint16_t _inner_y_offset_px, uint16_t _x_px, uint16_t _y_px, uint16_t _w_px, uint16_t _h_px);
  void set_inner_offset_px(uint16_t _x, uint16_t _y);
};

class Screen { 
public:
  uint8_t    num_viewports;
  std::vector<Viewport*>* vviewports;

  Screen();
  void add_viewport(Viewport* _viewport); 
};

class Sprite {
public:
  Tilelist* tilelist;
  uint16_t index;
  uint16_t x_px;
  uint16_t y_px;
  uint8_t  w_px;
  uint8_t  h_px;
  Sprite(Tilelist* _tilelist, uint16_t _index, uint16_t _x_px, uint16_t _y_px);
};

class BigMapEngine {
public:
  Screen*               screen;
  VGA_T4*               vga; 
  Tilelist*             tilelist; 
  std::vector<Sprite*>* sprites;
  
  BigMapEngine(Screen* _screen, VGA_T4* _vga, Tilelist* _tilelist);
  void render_next_frame(bool _render);
  uint32_t framecounter;
  void add_sprite(Sprite* _sprite);

private:
  void render_viewport(Viewport* viewport, bool _render);
};

#endif
