/* USER CODE BEGIN Header */
/**
 ******************************************************************************
  * File Name          : libjpeg.c
  * Description        : This file provides code for the configuration
  *                      of the libjpeg instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "libjpeg.h"

/* USER CODE BEGIN 0 */
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include "jpeglib.h"
#include "LCD.h"
/* USER CODE END 0 */

/* USER CODE BEGIN 1 */
struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;
/* USER CODE END 1 */

/* Global variables ---------------------------------------------------------*/

/* USER CODE BEGIN 2 */
/* USER CODE END 2 */

/* LIBJPEG init function */
void MX_LIBJPEG_Init(void)
{
/***************************************/
   /** 
  */

  /* USER CODE BEGIN 3 */
  /* USER CODE END 3 */

}

/* USER CODE BEGIN 4 */
//----------------------------------------------------------------------------
void my_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}

//----------------------------------------------------------------------------
int load_image(const char *name){
	struct jpeg_decompress_struct cinfo;
	struct my_error_mgr jerr;
	FIL infile;
	JSAMPARRAY buffer;
	int row_stride;
	FRESULT ret;

	ret = f_open(&infile, name, FA_OPEN_EXISTING | FA_READ);
	if(ret!=FR_OK){
		printf("fail f_open file[%s]\n",name);
		return _ERR;
	}

	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	if (setjmp(jerr.setjmp_buffer)) {
	    /* If we get here, the JPEG code has signaled an error.
	     * We need to clean up the JPEG object, close the input file, and return.  */
		jpeg_destroy_decompress(&cinfo);
		f_close(&infile);
	    return _ERR;
	}

	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, &infile);
	jpeg_read_header(&cinfo, TRUE);
	printf("image w=%d, h=%d, color=%d/%d\n",cinfo.image_width,cinfo.image_height,cinfo.jpeg_color_space,cinfo.num_components);

	cinfo.out_color_space = JCS_RGB;
	cinfo.scale_num = 1;
	cinfo.scale_denom =  cinfo.image_width / LCD_WIDTH;
	//scale_denom supported :1,2,4,8

	jpeg_start_decompress(&cinfo);
	printf("scale :1/%d, w=%d, h=%d\n",cinfo.scale_denom,cinfo.output_width,cinfo.output_height);

	row_stride = cinfo.output_width * cinfo.output_components;
	buffer = (*cinfo.mem->alloc_sarray)	((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

	LCD_Clear(BLACK);
	while (cinfo.output_scanline < cinfo.output_height) {
		jpeg_read_scanlines(&cinfo, buffer, 1);
		LCD_DisplayImageLine(0, cinfo.output_scanline-1, (char*)buffer[0], cinfo.output_width);
	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	f_close(&infile);

	LCD_DisplayString(0,LCD_HEIGHT-Font8.Height,name,&Font8,BLACK,WHITE);

	return _OK;
}

/* USER CODE END 4 */

/**
  * @}
  */
 
/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
