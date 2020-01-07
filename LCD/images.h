/*
 * images.h
 *
 *  Created on: 6 Νοε 2019
 *      Author: George
 */

#ifndef INC_IMAGES_H_
#define INC_IMAGES_H_

#include <stdint.h>

typedef struct _tImage
{
  const uint8_t *color_table;
  const uint8_t *image_table;
  uint16_t Colors;
  uint16_t Width;
  uint16_t Height;
} sIMAGE;

extern sIMAGE Image1;
extern sIMAGE Image2;

#endif /* INC_IMAGES_H_ */
