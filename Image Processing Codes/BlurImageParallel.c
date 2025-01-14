/*
  Basic Image Processing Library using OpenMp
  Task : Parallel Image blurring operation in C language on png images
          On every iteration the cores are allocated with the images i.e. one image on each core in each iteration
 


*/

#include <stdlib.h>
#include <stdio.h>
#include<string.h>
#include <png.h>
#include<omp.h>

int maskDimensions;

void read_png_file(char *filename,char *outputFileName) {
  int width,height,dimensions;
    png_byte color_type;
    png_byte bit_depth;
    png_bytep *row_pointers = NULL;
    fflush(stdin);
  FILE *fp = fopen(filename, "rb");

  png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if(!png) abort();

  png_infop info = png_create_info_struct(png);
  if(!info) abort();

  if(setjmp(png_jmpbuf(png))) abort();

  png_init_io(png, fp);

  png_read_info(png, info);
 
  width     = png_get_image_width(png, info);
  height    = png_get_image_height(png, info);

  color_type = png_get_color_type(png, info);
  bit_depth  = png_get_bit_depth(png, info);
 
  // Read any color_type into 8bit depth, RGBA format.
  // See http://www.libpng.org/pub/png/libpng-manual.txt

  if(bit_depth == 16)
    png_set_strip_16(png);

  if(color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_palette_to_rgb(png);

  

  // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
  if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
    png_set_expand_gray_1_2_4_to_8(png);

  if(png_get_valid(png, info, PNG_INFO_tRNS))
    png_set_tRNS_to_alpha(png);

  // These color_type don't have an alpha channel then fill it with 0xff.
  if(color_type == PNG_COLOR_TYPE_RGB ||
     color_type == PNG_COLOR_TYPE_GRAY ||
     color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
  if(color_type == PNG_COLOR_TYPE_GRAY ||
     color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    png_set_gray_to_rgb(png);

  png_read_update_info(png, info);

  //if (row_pointers) abort();

  row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
  for(int y = 0; y < height; y++) {
    row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png,info));
  }
    
  png_read_image(png, row_pointers);
 
  fclose(fp);

  png_destroy_read_struct(&png, &info, NULL);

  
  int **maskMatrix;
  int maskD = maskDimensions * maskDimensions;
  maskMatrix = (int**)malloc(maskDimensions*sizeof(int*));
  for(int i=0;i<maskDimensions;i++){
    maskMatrix[i] = (int*)malloc(maskDimensions*sizeof(int));
  }
  for(int i=0;i<maskDimensions;i++){
    for(int j=0;j<maskDimensions;j++)
      maskMatrix[i][j]=1;
  }


  int size =  height * width;
    int averageR,averageG,averageB;
    int *maskedImageR = (int*)malloc(size*sizeof(int));
    int *maskedImageG = (int*)malloc(size*sizeof(int));
    int *maskedImageB = (int*)malloc(size*sizeof(int));
    int val=0;
    for(int i=0 ; i < height; i++)
    {

          for(int j=0 ; j < width ; j++)
          {

                averageR = 0;
                averageG = 0;
                averageB = 0;

              for(int itImageX = i - maskDimensions/2, itMaskX = 0; itImageX <= ( i+maskDimensions/2 ) && itMaskX < maskDimensions ;itImageX++,itMaskX++)
              {
                      if(itImageX>=0 && itImageX<=height){
                        png_bytep row = row_pointers[itImageX];
                        for(int itImageY = j - maskDimensions/2, itMaskY =0; itImageY <= (j+maskDimensions/2 ) && itMaskY < maskDimensions; itImageY++,itMaskY++)
                        {
                            png_bytep px = &(row[itImageY * 4]);
                            if(itImageX >= 0 && itImageX < height && itImageY >= 0 && itImageY < width){
                                      averageR += (px[0] *maskMatrix[itMaskX][itMaskY]);
                                      averageG += (px[1] *maskMatrix[itMaskX][itMaskY]);
                                      averageB += (px[2] *maskMatrix[itMaskX][itMaskY]);
                            }
                                
                        }
                      }
                    
              } 
              //storing Red
                      if(averageR/maskD > 255)
                        averageR = 255*maskD;
                      else if(averageR/maskD < 0)
                            averageR = 0;
                      maskedImageR[i*width + j] = averageR/maskD;
              
              
              //storing green 
                        if(averageG/maskD > 255)
                          averageG = 255*maskD;
                        else if(averageG/maskD < 0)
                              averageG = 0;
                        maskedImageG[i*width + j] = averageG/maskD;


              //storing blue
                        if(averageB/maskD > 255)
                          averageB = 255*maskD;
                        else if(averageB/maskD < 0)
                              averageB = 0;
                        maskedImageB[i*width + j] = averageB/maskD;

          }
    }
  
  
      int x=0;
      //storing to the image
      for(int i=0;i<height;i++)
      { 
        png_bytep row = row_pointers[i];
        for(int j=0;j<width;j++)
        {
          png_bytep px = &(row[j * 4]);
          px[0] = maskedImageR[x];
          px[1] = maskedImageG[x];
          px[2] = maskedImageB[x];
          x++;
        }
      }
  fp = fopen(outputFileName, "wb");
  if(!fp) abort();

  png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png) abort();

  info = png_create_info_struct(png);
  if (!info) abort();

  if (setjmp(png_jmpbuf(png))) abort();

  png_init_io(png, fp);
  png_set_IHDR(
    png,
    info,
    width, height,
    8,
    PNG_COLOR_TYPE_RGBA,
    PNG_INTERLACE_NONE,
    PNG_COMPRESSION_TYPE_DEFAULT,
    PNG_FILTER_TYPE_DEFAULT
  );
 
  png_write_info(png, info);
	

	free(maskedImageR);
	free(maskedImageG);
	free(maskedImageB);
	

    png_write_image(png, row_pointers);
    png_write_end(png, NULL);

for(int y = 0; y < height; y++) {
     free(row_pointers[y]);
   }
  free(row_pointers);
	
	for(int j=0;j<maskDimensions;j++)
      free(maskMatrix[j]);
	free(maskMatrix);
    fclose(fp);

    png_destroy_write_struct(&png, &info);
}

int main() 
{
	printf("\nEnter the size of Mask:\n");
  scanf("%d",&maskDimensions);
  double startTime = omp_get_wtime();
  #pragma omp parallel
  {
    #pragma omp for schedule(static)
      for(int i=1;i<=20;i++)
      {
              char str[25]="cat (";
                char out[25]="out (";
                int a=i;
                int tmp=i;
                int cnt=0;
                while(tmp)
                {
                    tmp=tmp/10;
                    cnt++;
                }
        
                int j=cnt-1;
                char pok[25]=").png";
                char lok[25];
                while(a)
                {
                    int k=a%10;
                    lok[j]=(char)('0'+k);
                    a=a/10;
                    j--;
                }
                lok[cnt]='\0';
                strcat(str,lok);
                strcat(str,pok);
                strcat(out,lok);
                strcat(out,pok);
                char* s=out;
                char* p=str;
    
        read_png_file(p,s);
        //free(maskMatrix);
      }

  }
  
	double endTime = omp_get_wtime();
	printf("Time taken is %lf",endTime - startTime);
  return 0;
}

