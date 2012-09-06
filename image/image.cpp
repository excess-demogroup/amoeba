/*
 * virtual image class :-)
 */
#include "image.h"
#include <stdio.h>
#include <string.h>

Image::Image() {}
Image::~Image() {}

int Image::get_width()
{
	return this->width;
}
int Image::get_height()
{
	return this->height;
}
int Image::get_bpp()
{
	return this->bpp;
}
