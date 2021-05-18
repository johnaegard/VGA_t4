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

void BigMapEngine::render_next_frame() { 
  vga->waitLine(480+40);
  for(int v=0;v<screen->num_viewports;v++) {
    Viewport* viewport = screen->get_viewport(v); 
    render_viewport(viewport); 
  } 
  framecounter++; 
}

void BigMapEngine::render_viewport(Viewport* viewport) {

  Tilemap* tilemap = viewport->tilemap;

  uint16_t col1   = viewport->inner_x_offset_px / tilelist->tile_size_px;
  uint16_t width  = viewport->w_px/tilelist->tile_size_px;
  uint16_t col2   = col1+width;

  uint16_t row1   = viewport->inner_y_offset_px / tilelist->tile_size_px;
  uint16_t height = viewport->h_px/tilelist->tile_size_px;
  uint16_t row2   = row1+height;
   
  uint16_t voff    = viewport->inner_y_offset_px % tilelist->tile_size_px;

  if (framecounter % 200 == 0) {
    Serial.print("frame=");
    Serial.print(framecounter);
    Serial.print(" inner_yoff=");
    Serial.print(viewport->inner_y_offset_px);
    Serial.print(" voff=");
    Serial.print(voff);
    Serial.print(" col=");
    Serial.print(col1);
    Serial.print("...");
    Serial.print(col2);
    Serial.print(" row=");
    Serial.print(row1);
    Serial.print("...");
    Serial.println(row2);
  }

//  for(uint8_t r=row1; r<row1+height; r++) {
//    for(uint8_t c=col1; c<col1+width; c++) {
  for(uint8_t r=row1; r<row2; r++) {
    for(uint8_t c=col1; c<col2; c++) {
      uint8_t tile_index = viewport->tilemap->get_tile_index(c,r); 
      vga->drawBitmap(
        tilelist->get_tile(tile_index),
        tilelist->tile_size_px,
        viewport->x_px + (c * tilelist->tile_size_px), 
        viewport->y_px + (r * tilelist->tile_size_px),
        voff,
        (r==0),
        (r==row1+height)
      );
    } 
  }
}

//void VGA_T4::run_bigmap_engine()
//{
//  waitLine(480+40);
//
//  unsigned char * tilept;
//
//  // Layer 0
//  for (int j=0; j<TILES_ROWS; j++)
//  {
//    tilept = &tilesram[j*TILES_COLS];
//    if ( (j>=hscr_beg[0]) && (j<=hscr_end[0]) ) {     
//      int modcol = (hscr[0] >> TILES_HBITS) % TILES_COLS;
//      for (int i=0; i<TILES_COLS; i++)
//      {
//        (i == 0) ? drawTileCropL(tilept[modcol], (i<<TILES_HBITS) - (hscr[0] & TILES_HMASK), j*TILES_H) : 
//          (i == (TILES_COLS-1))?drawTileCropR(tilept[modcol], (i<<TILES_HBITS) - (hscr[0] & TILES_HMASK), j*TILES_H) : 
//            drawTile(tilept[modcol], (i<<TILES_HBITS) - (hscr[0] & TILES_HMASK), j*TILES_H);
//        modcol++;
//        modcol = modcol % TILES_COLS; 
//      }
//    }  
//    else {
//      for (int i=0; i<TILES_COLS; i++)
//      {
//        (i == (TILES_COLS-1)) ? drawTileCropR(tilept[i], (i<<TILES_HBITS), j*TILES_H) :
//          drawTile(tilept[i], (i<<TILES_HBITS), j*TILES_H);
//      }      
//    }
//  }
//
//  // Other layers
//  if (nb_layers > 1) {
//    int lcount = 1;
//    while (lcount < nb_layers) {
//      for (int j=0; j<TILES_ROWS; j++)
//      {
//        tilept = &tilesram[(j+lcount*TILES_ROWS)*TILES_COLS];
//        if ( (j>=hscr_beg[lcount]) && (j<=hscr_end[lcount]) ) {     
//          int modcol = (hscr[lcount] >> TILES_HBITS) % TILES_COLS;
//          for (int i=0; i<TILES_COLS; i++)
//          {
//            (i == 0) ? drawTransTileCropL(tilept[modcol], (i<<TILES_HBITS) - (hscr[lcount] & TILES_HMASK), j*TILES_H) : 
//              (i == (TILES_COLS-1))?drawTransTileCropR(tilept[modcol], (i<<TILES_HBITS) - (hscr[lcount] & TILES_HMASK), j*TILES_H) : 
//                drawTransTile(tilept[modcol], (i<<TILES_HBITS) - (hscr[lcount] & TILES_HMASK), j*TILES_H);
//            modcol++;
//            modcol = modcol % TILES_COLS; 
//          }
//        }          
//        else {
//          for (int i=0; i<TILES_COLS; i++)
//          {
//            drawTransTile(tilept[i], (i<<TILES_HBITS), j*TILES_H);
//          }      
//        }
//      }
//      lcount++;
//    }  
//  } 
//
///*
// static char * ro="01234567890123456789012345678901234567";
// for (int i=0; i<TILES_ROWS*2; i++)
// {
//  tileTextOverlay(0, i*8, ro, VGA_RGB(0x00,0xff,0x00)); 
// }
//*/
//
// for (int i=0; i<SPRITES_MAX; i++)
// {
//   drawSpr(spritesdata[i].index, spritesdata[i].x, spritesdata[i].y);
// }
//}

