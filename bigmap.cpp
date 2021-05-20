#include "VGA_t4.h"
#include "bigmap.h"
#include "Arduino.h"

Tilelist::Tilelist(uint8_t _tile_size_px, uint16_t _max_tiles) {
  tile_size_px    = _tile_size_px;
  max_tiles       = _max_tiles;
  pixels          = (vga_pixel*) calloc(tile_size_px * tile_size_px * max_tiles, sizeof(vga_pixel));
  num_tiles       = 0;
  tile_size_bytes = tile_size_px * tile_size_px * sizeof(vga_pixel);
}

void Tilelist::add_tile_with_color(uint8_t _color){
  uint16_t offset = num_tiles++ * tile_size_bytes;
  memset( (void*) &pixels[offset], _color, tile_size_bytes);
  uint8_t random_color = random(0,256);
  pixels[offset+27] =random_color;
  pixels[offset+28] =random_color;
  pixels[offset+35] =random_color;
  pixels[offset+36] =random_color;
}

vga_pixel* Tilelist::get_tile(uint16_t _index) {
  return &pixels[_index * tile_size_bytes];
}

Tilemap::Tilemap(uint16_t _num_cols, uint16_t _num_rows){
  num_rows = _num_rows;
  num_cols = _num_cols;
  tiles    = (uint16_t*) calloc(num_rows * num_cols, sizeof(uint16_t));
}

void Tilemap::setTile(uint16_t _col, uint16_t _row, uint16_t _index) { 
  uint16_t offset = (_row * num_cols) + _col;
  tiles[offset] = _index;
}

uint16_t Tilemap::get_tile_index(uint16_t _col, uint16_t _row) { 
  uint16_t offset = (_row * num_cols) + _col;
  return tiles[offset];
}

Viewport::Viewport(Tilemap* _tilemap, uint16_t _inner_x_offset_px, uint16_t _inner_y_offset_px, uint16_t _x_px, uint16_t _y_px, uint16_t _w_px, uint16_t _h_px) { 
  tilemap = _tilemap;
  inner_y_offset_px = _inner_y_offset_px;
  inner_x_offset_px = _inner_x_offset_px;
  x_px = _x_px;
  y_px = _y_px;
  w_px = _w_px;
  h_px = _h_px;
}

void Viewport::set_inner_offset_px(uint16_t _x, uint16_t _y) {
  inner_x_offset_px = _x;
  inner_y_offset_px = _y;
}

Screen::Screen(uint8_t _max_viewports) {
  max_viewports = _max_viewports;
  num_viewports = 0;
  //TODO this allocation is not right. 
  viewports     = (Viewport **) calloc(max_viewports,sizeof(Viewport*));
}

void Screen::add_viewport(Viewport* _viewport) {
  viewports[num_viewports++] = _viewport;
}

Viewport* Screen::get_viewport(uint8_t _index) {
  return viewports[_index];
}

//TODO make these local to BigMapEngine

BigMapEngine::BigMapEngine(Screen* _screen, VGA_T4* _vga, Tilelist* _tilelist) {
  screen = _screen;
  vga    = _vga;
  tilelist = _tilelist;
  framecounter = 0;
}

void BigMapEngine::render_next_frame(bool _render) { 
  vga->waitLine(480+40);
  for(int v=0;v<screen->num_viewports;v++) {
    Viewport* viewport = screen->get_viewport(v); 
    render_viewport(viewport, _render); 
    if (framecounter % 600 == 0) {
      Serial.print("rendered viewport=");
      Serial.println(v);
    }
  } 
  framecounter++; 
}

void BigMapEngine::render_viewport(Viewport* viewport, bool _render) {

  uint16_t col1   = viewport->inner_x_offset_px / tilelist->tile_size_px;
  uint16_t width  = viewport->w_px/tilelist->tile_size_px;
  uint16_t col2   = col1+width+2;
  uint16_t xoff   = viewport->inner_x_offset_px % tilelist->tile_size_px;

  uint16_t row1   = viewport->inner_y_offset_px / tilelist->tile_size_px;
  uint16_t height = viewport->h_px/tilelist->tile_size_px;
  uint16_t row2   = row1+height+2; 
  uint16_t voff   = viewport->inner_y_offset_px % tilelist->tile_size_px;

  uint16_t crop_top    = viewport->y_px;
  uint16_t crop_left   = viewport->x_px;
  uint16_t crop_bottom = viewport->y_px + viewport->h_px -1;
  uint16_t crop_right  = viewport->x_px + viewport->w_px -1;

  if (framecounter % 600 == 0) {
    Serial.print(" frame=");
    Serial.print(framecounter);
    Serial.print(" height=");
    Serial.print(height);
    Serial.print(" inner_yoff=");
    Serial.print(viewport->inner_y_offset_px);
    Serial.print(" voff=");
    Serial.print(voff);
    Serial.print(" cols=");
    Serial.print(col1);
    Serial.print("...");
    Serial.print(col2);
    Serial.print(" rows=");
    Serial.print(row1);
    Serial.print("...");
    Serial.println(row2);
  }

  for(uint8_t r=row1; r<row2; r++) {

    int16_t viewport_line = ((r-row1) * tilelist->tile_size_px) - voff;
    int16_t screen_line   = viewport->y_px + viewport_line;

    if (framecounter % 600 == 0) {
      Serial.print("rendering maprow=");
      Serial.print(r);
      Serial.print(" viewport line=");
      Serial.print(viewport_line);
      Serial.print(" screen line=");
      Serial.print(screen_line);
      Serial.print(" crop_top=");
      Serial.print(crop_top);
      Serial.print(" crop_bottom=");
      Serial.println(crop_bottom);
    }
    for(uint8_t c=col1; c<col2; c++) {

      int16_t viewport_col = ((c-col1) * tilelist->tile_size_px) - xoff;
      int16_t screen_col   = viewport->x_px + viewport_col;

      uint8_t tile_index = viewport->tilemap->get_tile_index(c,r); 
      vga->drawBitmap(
        tilelist->get_tile(tile_index),
        tilelist->tile_size_px,
        screen_col,
        screen_line,
        crop_top,
        crop_bottom,
        crop_left,
        crop_right,
        framecounter % 600 == 0 && c==0,
        _render 
      );
    } 
  }
}

