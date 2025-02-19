/*
	This file is part of VGA_t4 library.

	VGA_t4 library is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Copyright (C) 2020 J-M Harvengt

	Inspired from the original Teensy3 uVGA library of Eric PREVOTEAU.
	QTIMER/FlexIO code based on Teensy4 examples of KurtE, Manitou and easone 
	from the Teensy4 forum (https://forum.pjrc.com)
*/

#ifndef _VGA_T4_H
#define _VGA_T4_H

#include <Arduino.h>
#include <avr_emulation.h>
#include <DMAChannel.h>
#include <math.h>


// Enable debug info (requires serial initialization)
//#define DEBUG

// Enable 12bits mode
// Default is 8bits RRRGGGBB (332) 
// But 12bits GBB0RRRRGGGBB (444) feasible BUT NOT TESTED !!!!
//#define BITS12



#ifdef BITS12
typedef uint16_t vga_pixel;
#define VGA_RGB(r,g,b)  ( (((r>>3)&0x1f)<<11) | (((g>>2)&0x3f)<<5) | (((b>>3)&0x1f)<<0) )
#else
typedef uint8_t vga_pixel;
#define VGA_RGB(r,g,b)          ( (((r>>5)&0x07)<<5) | (((g>>5)&0x07)<<2) | (((b>>6)&0x3)<<0) )
#define VGA_TIRRGGBB(t,i,r,g,b) ( (t<<7) | (i<<6) | ((r<<4)&0b110000) | ((g<<2)&0b1100) | (b&0b11) )
#endif

typedef enum vga_mode_t
{
  VGA_MODE_320x240 = 0,
  VGA_MODE_320x480 = 1,
  VGA_MODE_352x240 = 2,
  VGA_MODE_352x480 = 3,
  VGA_MODE_512x240 = 4,
  VGA_MODE_512x480 = 5,
  VGA_MODE_640x240 = 6,
  VGA_MODE_640x480 = 7
} vga_mode_t;


typedef enum vga_error_t
{
	VGA_OK = 0,
	VGA_ERROR = -1
} vga_error_t;

#define MaxPolyPoint    100

#define AUDIO_SAMPLE_BUFFER_SIZE 256

// 2D point structure
typedef struct {
	int16_t x;			// X Coordinate on screen
	int16_t y;			// Y Coordinate on screen
}Point2D;

// Polygon structure
typedef struct {
	Point2D		Center;				// Polygon Center (point where the polygon can rotate arround)
	Point2D		Pts[MaxPolyPoint];	// Points for the polygon
}PolyDef;


#define DEFAULT_VSYNC_PIN 8

#ifndef ABS
#define ABS(X)  ((X) > 0 ? (X) : -(X))
#endif

extern PolyDef PolySet;  // polygon data to declare in c file


// Precomputed sinus and cosinus table from 0 to 359 degrees
// The tables are in Degrees not in Radian !
const float calcsi[360]={
		0.000001 ,                                                                                                                            //  0
		0.01745239 , 0.03489947 , 0.05233591 , 0.06975641 , 0.08715567 , 0.1045284 , 0.1218692 , 0.139173 , 0.1564343 , 0.173648 ,            // 1 à  10
		0.1908088 , 0.2079115 , 0.2249509 , 0.2419217 , 0.2588188 , 0.2756371 , 0.2923715 , 0.3090167 , 0.3255679 , 0.3420198 ,               // 11 à  20
		0.3583677 , 0.3746063 , 0.3907308 , 0.4067363 , 0.4226179 , 0.4383708 , 0.4539901 , 0.4694712 , 0.4848093 , 0.4999996 ,               // 21 à  30
		0.5150377 , 0.5299189 , 0.5446386 , 0.5591925 , 0.573576 , 0.5877848 , 0.6018146 , 0.615661 , 0.62932 , 0.6427872 ,                   // 31 à  40
		0.6560586 , 0.6691301 , 0.6819978 , 0.6946579 , 0.7071063 , 0.7193394 , 0.7313532 , 0.7431444 , 0.7547091 , 0.7660439 ,               // 41 à  50
		0.7771455 , 0.7880103 , 0.798635 , 0.8090165 , 0.8191515 , 0.8290371 , 0.8386701 , 0.8480476 , 0.8571668 , 0.8660249 ,                // 51 à  60
		0.8746193 , 0.8829472 , 0.8910061 , 0.8987936 , 0.9063074 , 0.913545 , 0.9205045 , 0.9271835 , 0.9335801 , 0.9396922 ,                // 61 à  70
		0.9455183 , 0.9510562 , 0.9563044 , 0.9612614 , 0.9659255 , 0.9702954 , 0.9743698 , 0.9781474 , 0.981627 , 0.9848075 ,                // 71 à  80
		0.9876881 , 0.9902679 , 0.992546 , 0.9945218 , 0.9961946 , 0.9975639 , 0.9986295 , 0.9993908 , 0.9998476 , 0.99999 ,                  // 81 à  90
		0.9998477 , 0.9993909 , 0.9986296 , 0.9975642 , 0.9961948 , 0.994522 , 0.9925463 , 0.9902682 , 0.9876886 , 0.984808 ,                 // 91 à  100
		0.9816275 , 0.9781479 , 0.9743704 , 0.9702961 , 0.9659262 , 0.9612621 , 0.9563052 , 0.9510571 , 0.9455191 , 0.9396932 ,               // 101 à  110
		0.933581 , 0.9271844 , 0.9205055 , 0.9135461 , 0.9063086 , 0.8987948 , 0.8910073 , 0.8829485 , 0.8746206 , 0.8660263 ,                // 111 à  120
		0.8571682 , 0.8480491 , 0.8386716 , 0.8290385 , 0.8191531 , 0.8090182 , 0.7986366 , 0.7880119 , 0.7771472 , 0.7660457 ,               // 121 à  130
		0.7547108 , 0.7431462 , 0.7313551 , 0.7193412 , 0.7071083 , 0.6946598 , 0.6819999 , 0.6691321 , 0.6560606 , 0.6427892 ,               // 131 à  140
		0.629322 , 0.6156631 , 0.6018168 , 0.5877869 , 0.5735782 , 0.5591948 , 0.5446408 , 0.5299212 , 0.5150401 , 0.5000019 ,                // 141 à  150
		0.4848116 , 0.4694737 , 0.4539925 , 0.4383733 , 0.4226205 , 0.4067387 , 0.3907333 , 0.3746087 , 0.3583702 , 0.3420225 ,               // 151 à  160
		0.3255703 , 0.3090193 , 0.2923741 , 0.2756396 , 0.2588214 , 0.2419244 , 0.2249534 , 0.2079142 , 0.1908116 , 0.1736506 ,               // 161 à  170
		0.156437 , 0.1391758 , 0.1218719 , 0.1045311 , 0.08715825 , 0.06975908 , 0.05233867 , 0.03490207 , 0.01745508 , 0.0277 ,              // 171 à  180
		-0.01744977 , -0.03489676 , -0.05233313 , -0.06975379 , -0.08715296 , -0.1045256 , -0.1218666 , -0.1391703 , -0.1564316 , -0.1736454 ,// 181 à  190
		-0.1908061 , -0.207909 , -0.2249483 , -0.241919 , -0.2588163 , -0.2756345 , -0.2923688 , -0.3090142 , -0.3255653 , -0.3420173 ,       // 191 à  200
		-0.3583652 , -0.3746038 , -0.3907282 , -0.4067339 , -0.4226155 , -0.4383683 , -0.4539878 , -0.4694688 , -0.4848068 , -0.4999973 ,     // 201 à  210
		-0.5150353 , -0.5299166 , -0.5446364 , -0.5591902 , -0.5735739 , -0.5877826 , -0.6018124 , -0.615659 , -0.6293178 , -0.642785 ,       // 211 à  220
		-0.6560566 , -0.6691281 , -0.6819958 , -0.694656 , -0.7071043 , -0.7193374 , -0.7313514 , -0.7431425 , -0.7547074 , -0.7660421 ,      // 221 à  230
		-0.7771439 , -0.7880087 , -0.7986334 , -0.8090149 , -0.8191499 , -0.8290355 , -0.8386687 , -0.8480463 , -0.8571655 , -0.8660236 ,     // 231 à  240
		-0.8746178 , -0.882946 , -0.8910049 , -0.8987925 , -0.9063062 , -0.9135439 , -0.9205033 , -0.9271825 , -0.9335791 , -0.9396913 ,      // 241 à  250
		-0.9455173 , -0.9510553 , -0.9563036 , -0.9612607 , -0.9659248 , -0.9702948 , -0.9743692 , -0.9781467 , -0.9816265 , -0.9848071 ,     // 251 à  260
		-0.9876878 , -0.9902675 , -0.9925456 , -0.9945215 , -0.9961944 , -0.9975638 , -0.9986293 , -0.9993907 , -0.9998476 , -0.99999 ,       // 261 à  270
		-0.9998478 , -0.9993909 , -0.9986298 , -0.9975643 , -0.9961951 , -0.9945223 , -0.9925466 , -0.9902686 , -0.987689 , -0.9848085 ,      // 271 à  280
		-0.981628 , -0.9781484 , -0.974371 , -0.9702968 , -0.965927 , -0.9612629 , -0.9563061 , -0.9510578 , -0.9455199 , -0.9396941 ,        // 281 à  290
		-0.933582 , -0.9271856 , -0.9205065 , -0.9135472 , -0.9063097 , -0.898796 , -0.8910086 , -0.8829498 , -0.8746218 , -0.8660276 ,       // 291 à  300
		-0.8571696 , -0.8480505 , -0.8386731 , -0.8290402 , -0.8191546 , -0.8090196 , -0.7986383 , -0.7880136 , -0.777149 , -0.7660476 ,      // 301 à  310
		-0.7547125 , -0.7431479 , -0.7313569 , -0.7193431 , -0.7071103 , -0.6946616 , -0.6820017 , -0.6691341 , -0.6560627 , -0.6427914 ,     // 311 à  320
		-0.6293243 , -0.6156651 , -0.6018188 , -0.5877892 , -0.5735805 , -0.5591971 , -0.5446434 , -0.5299233 , -0.5150422 , -0.5000043 ,     // 321 à  330
		-0.484814 , -0.4694761 , -0.4539948 , -0.4383755 , -0.4226228 , -0.4067413 , -0.3907359 , -0.3746115 , -0.3583725 , -0.3420248 ,      // 331 à  340
		-0.325573 , -0.3090219 , -0.2923768 , -0.2756425 , -0.2588239 , -0.2419269 , -0.2249561 , -0.2079169 , -0.1908143 , -0.1736531 ,      // 341 à  350
		-0.1564395 , -0.1391783 , -0.1218746 , -0.1045339 , -0.08716125 , -0.06976161 , -0.0523412 , -0.03490484 , -0.01745785 };             // 351 à  359

const float calcco[360]={
		0.99999 ,                                                                                                                            //  0
		0.9998477 , 0.9993908 , 0.9986295 , 0.9975641 , 0.9961947 , 0.9945219 , 0.9925462 , 0.9902681 , 0.9876884 , 0.9848078 ,              // 1 à  10
		0.9816272 , 0.9781477 , 0.9743701 , 0.9702958 , 0.9659259 , 0.9612617 , 0.9563049 , 0.9510566 , 0.9455186 , 0.9396928 ,              // 11 à  20
		0.9335806 , 0.927184 , 0.920505 , 0.9135456 , 0.906308 , 0.8987943 , 0.8910067 , 0.8829478 , 0.8746199 , 0.8660256 ,                 // 21 à  30
		0.8571675 , 0.8480483 , 0.8386709 , 0.8290379 , 0.8191524 , 0.8090173 , 0.7986359 , 0.7880111 , 0.7771463 , 0.7660448 ,              // 31 à  40
		0.75471 , 0.7431452 , 0.7313541 , 0.7193403 , 0.7071072 , 0.6946589 , 0.6819989 , 0.6691311 , 0.6560596 , 0.6427882 ,                // 41 à  50
		0.629321 , 0.6156621 , 0.6018156 , 0.5877859 , 0.5735771 , 0.5591936 , 0.5446398 , 0.52992 , 0.5150389 , 0.5000008 ,                 // 51 à  60
		0.4848104 , 0.4694724 , 0.4539914 , 0.438372 , 0.4226191 , 0.4067376 , 0.3907321 , 0.3746075 , 0.3583689 , 0.3420211 ,               // 61 à  70
		0.3255692 , 0.309018 , 0.2923728 , 0.2756384 , 0.2588201 , 0.241923 , 0.2249522 , 0.2079128 , 0.1908101 , 0.1736494 ,                // 71 à  80
		0.1564357 , 0.1391743 , 0.1218706 , 0.1045297 , 0.08715699 , 0.06975782 , 0.05233728 , 0.0349008 , 0.01745369 , 0.0138 ,             // 81 à  90
		-0.01745104 , -0.03489815 , -0.05233451 , -0.06975505 , -0.08715434 , -0.1045271 , -0.1218679 , -0.1391717 , -0.156433 , -0.1736467 ,// 91 à  100
		-0.1908075 , -0.2079102 , -0.2249495 , -0.2419204 , -0.2588175 , -0.2756359 , -0.2923701 , -0.3090155 , -0.3255666 , -0.3420185 ,    // 101 à  110
		-0.3583664 , -0.3746051 , -0.3907295 , -0.4067351 , -0.4226166 , -0.4383696 , -0.4539889 , -0.4694699 , -0.4848081 , -0.4999984 ,    // 111 à  120
		-0.5150366 , -0.5299177 , -0.5446375 , -0.5591914 , -0.5735749 , -0.5877837 , -0.6018136 , -0.6156599 , -0.6293188 , -0.6427862 ,    // 121 à  130
		-0.6560575 , -0.669129 , -0.6819969 , -0.6946569 , -0.7071053 , -0.7193384 , -0.7313522 , -0.7431435 , -0.7547083 , -0.7660431 ,     // 131 à  140
		-0.7771447 , -0.7880094 , -0.7986342 , -0.8090158 , -0.8191508 , -0.8290363 , -0.8386694 , -0.8480469 , -0.8571661 , -0.8660243 ,    // 141 à  150
		-0.8746186 , -0.8829465 , -0.8910055 , -0.898793 , -0.9063068 , -0.9135445 , -0.9205039 , -0.927183 , -0.9335796 , -0.9396918 ,      // 151 à  160
		-0.9455178 , -0.9510558 , -0.956304 , -0.9612611 , -0.9659252 , -0.9702951 , -0.9743695 , -0.978147 , -0.9816267 , -0.9848073 ,      // 161 à  170
		-0.9876879 , -0.9902677 , -0.9925459 , -0.9945216 , -0.9961945 , -0.9975639 , -0.9986294 , -0.9993907 , -0.9998476 , -0.99999 ,      // 171 à  180
		-0.9998477 , -0.9993909 , -0.9986297 , -0.9975642 , -0.9961949 , -0.9945222 , -0.9925465 , -0.9902685 , -0.9876888 , -0.9848083 ,    // 181 à  190
		-0.9816277 , -0.9781482 , -0.9743707 , -0.9702965 , -0.9659266 , -0.9612625 , -0.9563056 , -0.9510574 , -0.9455196 , -0.9396937 ,    // 191 à  200
		-0.9335815 , -0.927185 , -0.9205061 , -0.9135467 , -0.9063091 , -0.8987955 , -0.8910079 , -0.8829491 , -0.8746213 , -0.866027 ,      // 201 à  210
		-0.857169 , -0.8480497 , -0.8386723 , -0.8290394 , -0.8191538 , -0.8090189 , -0.7986375 , -0.7880127 , -0.7771481 , -0.7660466 ,     // 211 à  220
		-0.7547117 , -0.743147 , -0.731356 , -0.7193421 , -0.7071092 , -0.6946609 , -0.6820008 , -0.6691331 , -0.6560616 , -0.6427905 ,      // 221 à  230
		-0.6293229 , -0.6156641 , -0.6018178 , -0.5877882 , -0.5735794 , -0.5591961 , -0.5446419 , -0.5299222 , -0.5150412 , -0.5000032 ,    // 231 à  240
		-0.4848129 , -0.4694746 , -0.4539936 , -0.4383744 , -0.4226216 , -0.4067401 , -0.3907347 , -0.3746099 , -0.3583714 , -0.3420237 ,    // 241 à  250
		-0.3255718 , -0.3090207 , -0.2923756 , -0.2756409 , -0.2588227 , -0.2419256 , -0.2249549 , -0.2079156 , -0.1908126 , -0.1736519 ,    // 251 à  260
		-0.1564383 , -0.139177 , -0.1218734 , -0.1045326 , -0.08715951 , -0.06976035 , -0.05233994 , -0.03490358 , -0.01745659 , -0.0427 ,   // 261 à  270
		0.01744851 , 0.0348955 , 0.05233186 , 0.06975229 , 0.08715146 , 0.1045246 , 0.1218654 , 0.139169 , 0.1564303 , 0.1736439 ,           // 271 à  280
		0.1908047 , 0.2079078 , 0.224947 , 0.2419178 , 0.2588149 , 0.2756331 , 0.2923674 , 0.309013 , 0.3255641 , 0.3420161 ,                // 281 à  290
		0.3583638 , 0.3746024 , 0.3907273 , 0.4067327 , 0.4226143 , 0.4383671 , 0.4539864 , 0.4694674 , 0.4848059 , 0.4999962 ,              // 291 à  300
		0.5150342 , 0.5299154 , 0.5446351 , 0.559189 , 0.5735728 , 0.5877816 , 0.6018113 , 0.6156578 , 0.6293167 , 0.6427839 ,               // 301 à  310
		0.6560556 , 0.6691272 , 0.6819949 , 0.6946549 , 0.7071033 , 0.7193366 , 0.7313506 , 0.7431416 , 0.7547064 , 0.7660413 ,              // 311 à  320
		0.7771428 , 0.7880079 , 0.7986327 , 0.8090141 , 0.8191492 , 0.8290347 , 0.8386678 , 0.8480456 , 0.8571648 , 0.8660229 ,              // 321 à  330
		0.8746172 , 0.8829452 , 0.8910043 , 0.8987919 , 0.9063057 , 0.9135434 , 0.9205029 , 0.9271819 , 0.9335786 , 0.9396909 ,              // 331 à  340
		0.9455169 , 0.9510549 , 0.9563032 , 0.9612602 , 0.9659245 , 0.9702945 , 0.9743689 , 0.9781465 , 0.9816261 , 0.9848069 ,              // 341 à  350
		0.9876875 , 0.9902673 , 0.9925455 , 0.9945213 , 0.9961942 , 0.9975637 , 0.9986292 , 0.9993906 , 0.9998476 };                         // 351 à  359


class VGA_T4
{
public:

  VGA_T4(int vsync_pin = DEFAULT_VSYNC_PIN);

  // display VGA image
  vga_error_t begin(vga_mode_t mode);
  void begin_audio(int samplesize, void (*callback)(short * stream, int len));
  void end();
  void end_audio();
  void debug();
  void tweak_video(int shiftdelta, int numdelta, int denomdelta);

  // retrieve real size of the frame buffer
  void get_frame_buffer_size(int *width, int *height);

  // wait next Vsync
  void waitSync();
  void waitLine(int line);

  // =========================================================
  // graphic primitives
  // =========================================================

  void clear(vga_pixel col) ; 
  void drawPixel(int x, int y, vga_pixel color);
  vga_pixel getPixel(int x, int y);
  vga_pixel * getLineBuffer(int j);
  void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, vga_pixel color);
  void drawText(int16_t x, int16_t y, const char * text, vga_pixel fgcolor, vga_pixel bgcolor, bool doublesize);
  void drawSprite(int16_t x, int16_t y, const int16_t *bitmap);
  void drawSprite(int16_t x, int16_t y, const int16_t *bitmap, uint16_t croparx, uint16_t cropary, uint16_t croparw, uint16_t croparh);
  void writeScreen(const vga_pixel *pcolors);  
  void writeLine(int width, int height, int y, vga_pixel *buf);
  void writeLine(int width, int height, int stride, uint8_t *buffer, vga_pixel *palette);
  void writeLine16(int width, int height, int y, uint16_t *buf);  
  void writeScreen(int width, int height, int stride, uint8_t *buffer, vga_pixel *palette);
  void copyLine(int width, int height, int ysrc, int ydst);
  void drawBitmap(vga_pixel* _pixels, uint8_t _bitmap_size_px, int16_t _x, int16_t _y, uint16_t crop_top, uint16_t crop_bottom, uint16_t crop_left, uint16_t crop_right, bool _log, bool _render, bool _trans);

  // ************************************** GFX API extension from darthvader ******************************************************
  void drawline(int16_t x1, int16_t y1, int16_t x2, int16_t y2, vga_pixel color);
  void draw_h_line(int16_t x1, int16_t y1, int16_t lenght, vga_pixel color);
  void draw_v_line(int16_t x1, int16_t y1, int16_t lenght, vga_pixel color);
  void drawcircle(int16_t x, int16_t y, int16_t radius, vga_pixel color);
  void drawfilledcircle(int16_t x, int16_t y, int16_t radius, vga_pixel fillcolor, vga_pixel bordercolor);
  void drawellipse(int16_t cx, int16_t cy, int16_t radius1, int16_t radius2, vga_pixel color);
  void drawfilledellipse(int16_t cx, int16_t cy, int16_t radius1, int16_t radius2, vga_pixel fillcolor, vga_pixel bordercolor);
  void drawtriangle(int16_t ax, int16_t ay, int16_t bx, int16_t by, int16_t cx, int16_t cy, vga_pixel color);
  void drawfilledtriangle(int16_t ax, int16_t ay, int16_t bx, int16_t by, int16_t cx, int16_t cy, vga_pixel fillcolor, vga_pixel bordercolor);
  void drawquad(int16_t centerx, int16_t centery, int16_t w, int16_t h, int16_t angle, vga_pixel color);
  void drawfilledquad(int16_t centerx, int16_t centery, int16_t w, int16_t h, int16_t angle, vga_pixel fillcolor, vga_pixel bordercolor);
  void drawpolygon(int16_t cx, int16_t cy, vga_pixel bordercolor);
  void drawfullpolygon(int16_t cx, int16_t cy, vga_pixel fillcolor, vga_pixel bordercolor);
  void drawrotatepolygon(int16_t cx, int16_t cy, int16_t Angle, vga_pixel fillcolor, vga_pixel bordercolor, uint8_t filled);
  // *******************************************************************************************************************************


  // =========================================================
  // Game engine
  // =========================================================

  #define TILES_MAX_LAYERS  2

  // 16x16 pixels tiles or 8x8 if USE_8PIXTILES is set
  //#define USE_8PIXTILES 1
  #ifdef USE_8PIXTILES
  #define TILES_COLS        40
  #define TILES_ROWS        30
  #define TILES_W           8
  #define TILES_H           8
  #define TILES_HBITS       3
  #define TILES_HMASK       0x7
  #else
  #define TILES_COLS        20
  #define TILES_ROWS        15
  #define TILES_W           16
  #define TILES_H           16
  #define TILES_HBITS       4
  #define TILES_HMASK       0xf
  #endif
  
  // 32 sprites 16x32 or max 64 16x16 (not larger!!!)
  #define SPRITES_MAX       32
  #define SPRITES_W         16
  #define SPRITES_H         32

  void begin_gfxengine(int nblayers, int nbtiles, int nbsprites);
  void run_gfxengine();
  void tile_data(unsigned char index, vga_pixel * data, int len);
  void sprite_data(unsigned char index, vga_pixel * data, int len);
  void sprite(int id , int x, int y, unsigned char index);
  void sprite_hide(int id);
  void tile_draw(int layer, int x, int y, unsigned char index);
  void tile_draw_row(int layer, int x, int y, unsigned char * data, int len);
  void tile_draw_col(int layer, int x, int y, unsigned char * data, int len);
  void set_hscroll(int layer, int rowbeg, int rowend, int mask);
  void set_vscroll(int layer, int colbeg, int colend, int mask);
  void hscroll(int layer, int value);
  void vscroll(int layer, int value);


private:
  static uint8_t _vsync_pin;
  static DMAChannel flexio1DMA;
  static DMAChannel flexio2DMA; 
  static void QT3_isr(void);
  static void AUDIO_isr(void);  
  static void SOFTWARE_isr(void);  
};


#endif


