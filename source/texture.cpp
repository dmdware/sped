#include "platform.h"
#include "texture.h"
#include "gui/gui.h"
#include "utils.h"
#include "debug.h"
#include "tool/rendersprite.h"
#include "app/appmain.h"

Texture g_texture[TEXTURES];
std::vector<TextureToLoad> g_texload;

int g_texwidth;
int g_texheight;
//int g_texindex;
int g_lastLTex = -1;

char jpegBuffer[JPEG_BUFFER_SIZE];
JPEGSource   jpegSource;
FILE* g_src;
//CFile g_src;
int srcLen;


LoadedTex::LoadedTex()
{
	data = NULL;
}

void LoadedTex::destroy()
{
	if(data)
	{
		free(data);
		data = NULL;
	}
}

LoadedTex::~LoadedTex()
{
	destroy();
}

LoadedTex *LoadBMP(const char *fullpath)
{
	LoadedTex *pImage = new LoadedTex;
	FILE *pFile = NULL;

	if((pFile = fopen(fullpath, "rb")) == NULL)
	{
		char msg[SPE_MAX_PATH+1];
		sprintf(msg, "Unable to load BMP File: %s", fullpath);
		ErrMess("Error", msg);
		return NULL;
	}

	if(!pImage)
		OutOfMem(__FILE__, __LINE__);

	/*
	TODO
	Switch to SDL2_Image usage for
	formats that require platform split.
	*/

#ifdef PLATFORM_WIN
	AUX_RGBImageRec *pBitbldg = NULL;

	// Load the bitbldg using the aux function stored in glaux.lib
	pBitbldg = auxDIBImageLoad(fullpath);

	pImage->channels = 3;
	pImage->sizex = pBitbldg->sizeX;
	pImage->sizey = pBitbldg->sizeY;
	pImage->data  = pBitbldg->data;

	free(pBitbldg);
#endif // PLATFORM_WIN
#ifdef PLATFORM_LINUX
	SDL_Surface *s = nullptr;
	s = SDL_LoadBMP(fullpath);

	pImage->channels = 3;
	pImage->sizex = s->h;
	pImage->sizey = s->w;
	pImage->data  = (unsigned char*)s->pixels; // Dunno if this right. TODO Confirm.
#endif // PLATFORM_LINUX

	/*
	int stride = pImage->channels * pBitbldg->sizex;
	int i;
	int y2;
	int temp;

	for(int y = 0; y < pImage->sizey/2; y++)
	{
		y2 = pImage->sizey - y;
		// Store a pointer to the current line of pixels
		unsigned char *pLine = &(pImage->data[stride * y]);
		unsigned char *pLine2 = &(pImage->data[stride * y2]);

		// Go through all of the pixels and swap the B and R values since TGA
		// files are stored as BGR instead of RGB (or use GL_BGR_EXT verses GL_RGB)
		for(i = 0; i < stride; i += pImage->channels)
		{
			temp = pLine[i];
			pLine[i] = pLine2[i];
			pLine2[i] = temp;

			temp = pLine[i+1];
			pLine[i+1] = pLine2[i+1];
			pLine2[i+1] = temp;

			temp = pLine[i+2];
			pLine[i+2] = pLine2[i+2];
			pLine2[i+2] = temp;
		}
	}*/

	fclose(pFile);

	return pImage;
}

LoadedTex *LoadTGA(const char *fullpath)
{
	LoadedTex *pImageData = NULL;			// This stores our important image data
	WORD width = 0, height = 0;			// The dimensions of the image
	byte length = 0;					// The length in bytes to the pixels
	byte imageType = 0;					// The image type (RLE, RGB, Alpha...)
	byte bits = 0;						// The bits per pixel for the image (16, 24, 32)
	FILE *pFile = NULL;					// The file pointer
	int channels = 0;					// The channels of the image (3 = RGA : 4 = RGBA)
	int stride = 0;						// The stride (channels * width)
	int i = 0;							// A counter

	if((pFile = fopen(fullpath, "rb")) == NULL)
	{
		char msg[SPE_MAX_PATH+1];
		sprintf(msg, "Unable to load TGA File: %s", fullpath);
		ErrMess("Error", msg);
		return NULL;
	}

	// allocate the structure that will hold our eventual image data (must free it!)
	pImageData = new LoadedTex;

	if(!pImageData)
		OutOfMem(__FILE__, __LINE__);

	// Read in the length in bytes from the header to the pixel data
	fread(&length, sizeof(byte), 1, pFile);

	// Jump over one byte
	fseek(pFile,1,SEEK_CUR);

	// Read in the imageType (RLE, RGB, etc...)
	fread(&imageType, sizeof(byte), 1, pFile);

	// Skip past general information we don't care about
	fseek(pFile, 9, SEEK_CUR);

	// Read the width, height and bits per pixel (16, 24 or 32)
	fread(&width,  sizeof(WORD), 1, pFile);
	fread(&height, sizeof(WORD), 1, pFile);
	fread(&bits,   sizeof(byte), 1, pFile);

	// Now we move the file pointer to the pixel data
	fseek(pFile, length + 1, SEEK_CUR);

	// Check if the image is RLE compressed or not
	if(imageType != TGA_RLE)
	{
		// Check if the image is a 24 or 32-bit image
		if(bits == 24 || bits == 32)
		{
			// Calculate the channels (3 or 4) - (use bits >> 3 for more speed).
			// Next, we calculate the stride and allocate enough memory for the pixels.
			channels = bits / 8;
			stride = channels * width;
			pImageData->data = ((unsigned char*)malloc(sizeof(unsigned char)*stride*height));

			if(!pImageData->data)
			{
				OutOfMem(__FILE__, __LINE__);
				fclose(pFile);
				return NULL;
			}

			// Load in all the pixel data line by line
			for(int y = 0; y < height; y++)
			{
				// Store a pointer to the current line of pixels
				unsigned char *pLine = &(pImageData->data[stride * y]);

				// Read in the current line of pixels
				fread(pLine, stride, 1, pFile);

				// Go through all of the pixels and swap the B and R values since TGA
				// files are stored as BGR instead of RGB (or use GL_BGR_EXT verses GL_RGB)
				for(i = 0; i < stride; i += channels)
				{
					int temp     = pLine[i];
					pLine[i]     = pLine[i + 2];
					pLine[i + 2] = temp;
				}
			}
		}
		// Check if the image is a 16 bit image (RGB stored in 1 unsigned short)
		else if(bits == 16)
		{
			unsigned short pixels = 0;
			int r=0, g=0, b=0;

			// Since we convert 16-bit images to 24 bit, we hardcode the channels to 3.
			// We then calculate the stride and allocate memory for the pixels.
			channels = 3;
			stride = channels * width;
			pImageData->data = ((unsigned char*)malloc(sizeof(unsigned char)*stride*height));

			if(!pImageData->data)
			{
				OutOfMem(__FILE__, __LINE__);
				fclose(pFile);
				return NULL;
			}

			// Load in all the pixel data pixel by pixel
			for(int i = 0; i < width*height; i++)
			{
				// Read in the current pixel
				fread(&pixels, sizeof(unsigned short), 1, pFile);

				// Convert the 16-bit pixel into an RGB
				b = (pixels & 0x1f) << 3;
				g = ((pixels >> 5) & 0x1f) << 3;
				r = ((pixels >> 10) & 0x1f) << 3;

				// This essentially assigns the color to our array and swaps the
				// B and R values at the same time.
				pImageData->data[i * 3 + 0] = r;
				pImageData->data[i * 3 + 1] = g;
				pImageData->data[i * 3 + 2] = b;
			}
		}
		// Else return a NULL for a bad or unsupported pixel format
		else
			return NULL;
	}
	// Else, it must be Run-Length Encoded (RLE)
	else
	{
		// Create some variables to hold the rleID, current colors read, channels, & stride.
		byte rleID = 0;
		int colorsRead = 0;
		channels = bits / 8;
		stride = channels * width;

		// Next we want to allocate the memory for the pixels and create an array,
		// depending on the channel count, to read in for each pixel.
		pImageData->data = ((unsigned char*)malloc(sizeof(unsigned char)*stride*height));
		byte *pColors = ((byte*)malloc(sizeof(byte)*channels));

		if(!pImageData->data)
		{
			OutOfMem(__FILE__, __LINE__);
			fclose(pFile);
			return NULL;
		}

		// Load in all the pixel data
		while(i < width*height)
		{
			// Read in the current color count + 1
			fread(&rleID, sizeof(byte), 1, pFile);

			// Check if we don't have an encoded std::string of colors
			if(rleID < 128)
			{
				// Increase the count by 1
				rleID++;

				// Go through and read all the unique colors found
				while(rleID)
				{
					// Read in the current color
					fread(pColors, sizeof(byte) * channels, 1, pFile);

					// Store the current pixel in our image array
					pImageData->data[colorsRead + 0] = pColors[2];
					pImageData->data[colorsRead + 1] = pColors[1];
					pImageData->data[colorsRead + 2] = pColors[0];

					// If we have a 4 channel 32-bit image, assign one more for the alpha
					if(bits == 32)
						pImageData->data[colorsRead + 3] = pColors[3];

					// Increase the current pixels read, decrease the amount
					// of pixels left, and increase the starting index for the next pixel.
					i++;
					rleID--;
					colorsRead += channels;
				}
			}
			// Else, let's read in a std::string of the same character
			else
			{
				// Minus the 128 ID + 1 (127) to get the color count that needs to be read
				rleID -= 127;

				// Read in the current color, which is the same for a while
				fread(pColors, sizeof(byte) * channels, 1, pFile);

				// Go and read as many pixels as are the same
				while(rleID)
				{
					// Assign the current pixel to the current index in our pixel array
					pImageData->data[colorsRead + 0] = pColors[2];
					pImageData->data[colorsRead + 1] = pColors[1];
					pImageData->data[colorsRead + 2] = pColors[0];

					// If we have a 4 channel 32-bit image, assign one more for the alpha
					if(bits == 32)
						pImageData->data[colorsRead + 3] = pColors[3];

					// Increase the current pixels read, decrease the amount
					// of pixels left, and increase the starting index for the next pixel.
					i++;
					rleID--;
					colorsRead += channels;
				}

			}

		}
	}

	// Close the file pointer that opened the file
	fclose(pFile);

	// Flip upside-down
	int x;
	int y2;
	byte temp[4];
	for(int y=0; y<height/2; y++)
	{
		y2 = height - y - 1;

		unsigned char *pLine = &(pImageData->data[stride * y]);
		unsigned char *pLine2 = &(pImageData->data[stride * y2]);

		for(x=0; x<width*channels; x+=channels)
		{
			temp[0] = pLine[x + 0];
			temp[1] = pLine[x + 1];
			temp[2] = pLine[x + 2];
			if(bits == 32)
				temp[3] = pLine[x + 3];

			pLine[x + 0] = pLine2[x + 0];
			pLine[x + 1] = pLine2[x + 1];
			pLine[x + 2] = pLine2[x + 2];
			if(bits == 32)
				pLine[x + 3] = pLine2[x + 3];

			pLine2[x + 0] = temp[0];
			pLine2[x + 1] = temp[1];
			pLine2[x + 2] = temp[2];
			if(bits == 32)
				pLine2[x + 3] = temp[3];
		}
	}

	// Fill in our LoadedTex structure to pass back
	pImageData->channels = channels;
	pImageData->sizex    = width;
	pImageData->sizey    = height;

	// Return the TGA data (remember, you must free this data after you are done)
	return pImageData;
}

#if 0
void DecodeJPG(jpeg_decompress_struct* cinfo, LoadedTex *pImageData)
{
	// Read in the header of the jpeg file
	//jpeg_read_header(cinfo, TRUE);
	jpeg_read_header(cinfo, (boolean)TRUE);

	// Start to decompress the jpeg file with our compression info
	jpeg_start_decompress(cinfo);

	// Get the image dimensions and channels to read in the pixel data
	pImageData->channels = cinfo->num_components;
	pImageData->sizex    = cinfo->image_width;
	pImageData->sizey    = cinfo->image_height;

	// Get the row span in bytes for each row
	int rowSpan = cinfo->image_width * cinfo->num_components;

	// allocate memory for the pixel buffer
	pImageData->data = ((unsigned char*)malloc(sizeof(unsigned char)*rowSpan*pImageData->sizey));

	if(!pImageData->data)
	{
		OutOfMem(__FILE__, __LINE__);
		return;
	}

	// Create an array of row pointers
	unsigned char** rowPtr = new unsigned char*[pImageData->sizey];

	if(!rowPtr)
	{
		OutOfMem(__FILE__, __LINE__);
		return;
	}

	for (int i = 0; i < pImageData->sizey; i++)
		rowPtr[i] = &(pImageData->data[i * rowSpan]);

	// Now comes the juice of our work, here we extract all the pixel data
	int rowsRead = 0;
	while (cinfo->output_scanline < cinfo->output_height)
	{
		// Read in the current row of pixels and increase the rowsRead count
		rowsRead += jpeg_read_scanlines(cinfo,
		                                &rowPtr[rowsRead], cinfo->output_height - rowsRead);
	}

	// Delete the temporary row pointers
	delete [] rowPtr;

	// Finish decompressing the data
	jpeg_finish_decompress(cinfo);
}
#endif

LoadedTex* LoadJPG2(const char *strFileName)
{
#if 1
	LoadedTex *pImageData = NULL;
    struct jpeg_decompress_struct cinfo;

    jpeg_error_mgr jerr;
        
        //pImageData = (Image*)malloc(sizeof(Image));
		pImageData = new LoadedTex;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

	FILE* fp = fopen(strFileName, "rb");

	    if (!fp)
        {
                delete pImageData;
                Log("Error opening jpeg %s", strFileName);
                return NULL;
        }

	fseek(fp, 0, SEEK_END);
	long myAssetLength = ftell(fp);
    unsigned char* ucharRawData = (unsigned char*)malloc(myAssetLength);
	fseek(fp, 0, SEEK_SET);
	fread(ucharRawData, 1, myAssetLength, fp);
	fclose(fp);
        
        // the jpeg_stdio_src alternative func, which is also included in IJG's lib.
        jpeg_mem_src(&cinfo, ucharRawData, myAssetLength);

    jpeg_read_header(&cinfo, TRUE);

    jpeg_start_decompress(&cinfo);

    pImageData->channels = cinfo.num_components;
    pImageData->sizex    = cinfo.image_width;
    pImageData->sizey    = cinfo.image_height;

    int rowSpan = cinfo.image_width * cinfo.num_components;

    pImageData->data = ((unsigned char*)malloc(sizeof(unsigned char)*rowSpan*pImageData->sizey));

    unsigned char** rowPtr = new unsigned char*[pImageData->sizey];

    for (int i = 0; i < pImageData->sizey; i++)
        rowPtr[i] = &(pImageData->data[i * rowSpan]);

    int rowsRead = 0;

    while (cinfo.output_scanline < cinfo.output_height)
        rowsRead += jpeg_read_scanlines(&cinfo, &rowPtr[rowsRead], cinfo.output_height - rowsRead);

    delete [] rowPtr;

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

	free(ucharRawData);

    return pImageData;
#endif
	return 0;
}

LoadedTex *LoadJPG(const char *fullpath)
{
#if 0
	struct jpeg_decompress_struct cinfo;
	LoadedTex *pImageData = NULL;
	FILE *pFile;

	// Open a file pointer to the jpeg file and check if it was found and opened
	if((pFile = fopen(fullpath, "rb")) == NULL)
	{
		if(!g_hidetexerr)
		{
			// Display an error message saying the file was not found, then return NULL
			char msg[SPE_MAX_PATH+1];
			sprintf(msg, "Unable to load JPG File: %s \n errno = %d", fullpath, errno);
			ErrMess("Error", msg);
		}
		return NULL;
	}

	// Create an error handler
	jpeg_error_mgr jerr;

	// Have our compression info object point to the error handler address
	cinfo.err = jpeg_std_error(&jerr);

	// Initialize the decompression object
	jpeg_create_decompress(&cinfo);

	// Specify the data source (Our file pointer)
	jpeg_stdio_src(&cinfo, pFile);

	// allocate the structure that will hold our eventual jpeg data (must free it!)
	pImageData = new LoadedTex;

	if(!pImageData)
		OutOfMem(__FILE__, __LINE__);

	// Decode the jpeg file and fill in the image data structure to pass back
	DecodeJPG(&cinfo, pImageData);

	// This releases all the stored memory for reading and decoding the jpeg
	jpeg_destroy_decompress(&cinfo);

	// Close the file pointer that opened the file
	fclose(pFile);

	// Return the jpeg data (remember, you must free this data after you are done)
	return pImageData;
#endif
	return NULL;
}
#if 0
void read_data_fn (png_structp png_ptr, png_bytep outb, png_size_t readsz)
{
	if(png_ptr->io_ptr == NULL)
		return;   // error

	FILE* fp = (FILE*)png_ptr->io_ptr;
	unsigned int readsz2 = fread((unsigned char*)outb, (size_t)readsz, fp);

	if((png_size_t)readsz2 != readsz)
		return;   // error
}
#endif

static FILE* loadpngfp = NULL;

void png_file_read(png_structp png_ptr, png_bytep data, png_size_t length) 
{
	fread(data, 1, length, loadpngfp);
}

LoadedTex *LoadPNG2(const char *strFileName)
{
        LoadedTex *pImageData = NULL;

		loadpngfp = fopen(strFileName, "rb");
    
        if(!loadpngfp)
  {
    return NULL;
  }

  //header for testing if it is a png
  png_byte header[8];

  fread(header, 8, 1, loadpngfp);
  //g_src.read((void*)header, 8);

  //test if png
  int is_png = !png_sig_cmp(header, 0, 8);
  if (!is_png) 
  {
    Log("Not a png file : %s %d,%d,%d,%d,%d,%d,%d,%d", strFileName, 
                (int)header[0], (int)header[1], (int)header[2], (int)header[3], 
                (int)header[4], (int)header[5], (int)header[6], (int)header[7]);
               fclose(loadpngfp);
    return NULL;
  }

  //create png struct
  png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr) 
  {
    Log("Unable to create png struct : %s", strFileName);
               fclose(loadpngfp);
    return NULL;
  }

  //create png info struct
  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) 
  {
    png_destroy_read_struct(&png_ptr, (png_infopp) NULL, (png_infopp) NULL);
    Log("Unable to create png info : %s", strFileName);
               fclose(loadpngfp);
    return NULL;
  }

  //create png info struct
  png_infop end_info = png_create_info_struct(png_ptr);
  if (!end_info) 
  {
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
    Log("Unable to create png end info : %s", strFileName);
               fclose(loadpngfp);
    return NULL;
  }

  //png error stuff, not sure libpng man suggests this.
  if (setjmp(png_jmpbuf(png_ptr))) 
  {
    Log("Error during setjmp : %s", strFileName);
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
               fclose(loadpngfp);
    return NULL;
  }

  png_set_read_fn(png_ptr, NULL, png_file_read);

  //let libpng know you already read the first 8 bytes
  png_set_sig_bytes(png_ptr, 8);

  // read all the info up to the image data
  png_read_info(png_ptr, info_ptr);

  //variables to pass to get info
  int bit_depth, color_type;
  png_uint_32 twidth, theight;

  // get info about png
  png_get_IHDR(png_ptr, info_ptr, &twidth, &theight, &bit_depth, &color_type, NULL, NULL, NULL);

	bool alphaFlag;

        //pImageData = (Image*)malloc(sizeof(Image));
		pImageData = new LoadedTex;

        pImageData->sizex = twidth;
		pImageData->sizey = theight;

		png_bytep trans_alpha = NULL;
		int num_trans = 0;
		png_color_16p trans_color = NULL;

  switch( color_type )
        {
        case PNG_COLOR_TYPE_RGBA:
            pImageData->channels = 4;
            break;
        case PNG_COLOR_TYPE_RGB:
            pImageData->channels = 3;
            break;
		case PNG_COLOR_TYPE_PALETTE:
			{
				png_set_palette_to_rgb(png_ptr);
				pImageData->channels = 3;
#if 0
				png_get_tRNS(png_ptr, info_ptr, &trans_alpha, &num_trans, &trans_color);
				if (trans_alpha != NULL)
					alphaFlag = true;
				else
					alphaFlag = false;
				if(alphaFlag)
					pImageData->channels = 4;
#endif

				//if(alphaFlag)
				if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
				{
					pImageData->channels = 4;
					png_set_tRNS_to_alpha(png_ptr);
				}
			}
			break;
        default:
            Log("%s color type %d not supported", strFileName, (int)png_get_color_type(png_ptr, info_ptr));
      
            png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
                        delete pImageData;
                fclose(loadpngfp);
            return NULL;
    }

  // Update the png info struct.
  png_read_update_info(png_ptr, info_ptr);

  // Row size in bytes.
  int row_bytes = png_get_rowbytes(png_ptr, info_ptr);
  
        pImageData->data = (unsigned char*) malloc(row_bytes * pImageData->sizey);

  // Allocate the image_data as a big block, to be given to opengl
        if(!pImageData->data)
  {
    //clean up memory and close stuff
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    Log("Unable to allocate image_data while loading %s ", strFileName);
        delete pImageData;
               fclose(loadpngfp);
    return NULL;
  }

  //row_pointers is for pointing to image_data for reading the png with libpng
  png_bytep *row_pointers = new png_bytep[pImageData->sizey];
  if (!row_pointers) 
  {
    //clean up memory and close stuff
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        free(pImageData->data);
        delete pImageData;
    Log("Unable to allocate row_pointer while loading %s ", strFileName);
               fclose(loadpngfp);
    return NULL;
  }
    
  // set the individual row_pointers to point at the correct offsets of image_data
  for (int i = 0; i < pImageData->sizey; ++i)
        row_pointers[i] = pImageData->data + i * row_bytes;

  //read the png into image_data through row_pointers
  png_read_image(png_ptr, row_pointers);

  //clean up memory and close stuff
  png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
  delete[] row_pointers;
               fclose(loadpngfp);

  return pImageData;
}

LoadedTex *LoadPNG(const char *fullpath)
{
	LoadedTex *pImageData = NULL;

	png_structp png_ptr;
	png_infop info_ptr;
	unsigned int sig_read = 0;
	//int color_type, interlace_type;
	FILE *fp;

	/*
	if ((fp = fopen(relative, "rb")) == NULL)
		return NULL;
	png_byte header[8];
	fread(header, sizeof(png_byte), 8, fp);
	fclose(fp);

	Log("PNG header "<<relative<<" "
		<<(int)header[0]<<","<<(int)header[1]<<","<<(int)header[2]<<","<<(int)header[3]<<","
		<<(int)header[4]<<","<<(int)header[5]<<","<<(int)header[6]<<","<<(int)header[7]);
	*/
	if ((fp = fopen(fullpath, "rb")) == NULL)
		return NULL;

	/* Create and initialize the png_struct
	 * with the desired error handler
	 * functions.  If you want to use the
	 * default stderr and longjump method,
	 * you can supply NULL for the last
	 * three parameters.  We also supply the
	 * the compiler header file version, so
	 * that we know if the application
	 * was compiled with a compatible version
	 * of the library.  REQUIRED
	 */

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (png_ptr == NULL)
	{
		ErrMess("png_ptr error", "png_ptr error");
		fclose(fp);
		return NULL;
	}

	/* allocate/initialize the memory
	 * for image information.  REQUIRED. */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		ErrMess("info_ptr error", "info_ptr error");
		fclose(fp);
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		return NULL;
	}

	/* Set error handling if you are
	 * using the setjmp/longjmp method
	 * (this is the normal method of
	 * doing things with libpng).
	 * REQUIRED unless you  set up
	 * your own error handlers in
	 * the png_create_read_struct()
	 * earlier.
	 */
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		/* Free all of the memory associated
		 * with the png_ptr and info_ptr */
		png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
		fclose(fp);
		/* If we get here, we had a
		 * problem reading the file */
		return NULL;
	}

	struct fake_file
	{
		unsigned int *buf;
		unsigned int size;
		unsigned int cur;
	};

	fake_file ff;
	ff.cur = 0;
	ff.buf = 0;

	//fseek(fp, 0, SEEK_END);
	//ff.size = ftell(fp);
	//fseek(fp, 0, SEEK_SET);
	//ff.buf = (unsigned int*)malloc(ff.size);
	//fread(ff.buf, 1, ff.size, fp);

	/* Set up the output control if
	 * you are using standard C streams */
	png_init_io(png_ptr, fp);
	//png_init_io(png_ptr, (FILE*)&ff);
	//png_set_read_fn(png_ptr, fp, read_data_fn);

	/* If we have already
	 * read some of the signature */
	png_set_sig_bytes(png_ptr, sig_read);

	/*
	 * If you have enough memory to read
	 * in the entire image at once, and
	 * you need to specify only
	 * transforms that can be controlled
	 * with one of the PNG_TRANSFORM_*
	 * bits (this presently excludes
	 * dithering, filling, setting
	 * background, and doing gamma
	 * adjustment), then you can read the
	 * entire image (including pixels)
	 * into the info structure with this
	 * call
	 *
	 * PNG_TRANSFORM_STRIP_16 |
	 * PNG_TRANSFORM_PACKING  forces 8 bit
	 * PNG_TRANSFORM_EXPAND forces to
	 *  expand a palette into RGB
	 */
	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND, png_voidp_NULL);

	return NULL;/////////////moddd
	pImageData = new LoadedTex;

	if(!pImageData)
		OutOfMem(__FILE__, __LINE__);

	pImageData->sizex = png_get_image_width(png_ptr, info_ptr); //info_ptr->width;
	pImageData->sizey = png_get_image_height(png_ptr, info_ptr); //info_ptr->height;
	//switch (info_ptr->color_type)
	switch( png_get_color_type(png_ptr, info_ptr) )
	{
	case PNG_COLOR_TYPE_RGBA:
		pImageData->channels = 4;
		break;
	case PNG_COLOR_TYPE_RGB:
		pImageData->channels = 3;
		break;
	case PNG_COLOR_TYPE_GRAY:
		pImageData->channels = 1;
		break;
	default:
		Log("png %s color type %d not supported\r\n", fullpath, (int)png_get_color_type(png_ptr, info_ptr));
		//std::cout << "Color type " << info_ptr->color_type << " not supported" << std::endl;
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(fp);
		free(ff.buf);
		delete pImageData;
		return NULL;
	}

	unsigned int row_bytes = png_get_rowbytes(png_ptr, info_ptr);
	pImageData->data = (unsigned char*) malloc(row_bytes * pImageData->sizey);

	if(!pImageData->data)
	{
		OutOfMem(__FILE__, __LINE__);
		fclose(fp);
		delete pImageData;
		free(ff.buf);
		return NULL;
	}

	png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);

	for (int i = 0; i < pImageData->sizey; i++)
	{
		// note that png is ordered top to
		// bottom, but OpenGL expect it bottom to top
		// so the order or swapped

		memcpy((void*)(pImageData->data+(row_bytes * i)), row_pointers[i], row_bytes);
		//memcpy((void*)(pImageData->data+(row_bytes * (pImageData->sizey-1-i))), row_pointers[i], row_bytes);
	}

	/* Clean up after the read,
	 * and free any memory allocated */
	png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
	fclose(fp);
	free(ff.buf);

	return pImageData;
}

bool FindTexture(unsigned int &textureidx, const char* relative)
{
	char corrected[1024];
	strcpy(corrected, relative);
	CorrectSlashes(corrected);

	for(int i=0; i<TEXTURES; i++)
	{
		Texture* t = &g_texture[i];

		if(t->loaded && stricmp(t->fullpath.c_str(), corrected) == 0)
		{
			//g_texindex = i;
			//texture = t->texname;
			textureidx = i;
			g_texwidth = t->width;
			g_texheight = t->height;
			return true;
		}
	}

	return false;
}

void FreeTexture(const char* relative)
{
	char corrected[SPE_MAX_PATH+1];
	strcpy(corrected, relative);
	CorrectSlashes(corrected);

	for(int i=0; i<TEXTURES; i++)
	{
		Texture* t = &g_texture[i];

		if(t->loaded && stricmp(t->fullpath.c_str(), corrected) == 0)
		{
			t->loaded = false;
			glDeleteTextures(1, &t->texname);
			delete t->pixels;
			t->pixels = NULL;
			//Log("Found texture "<<filepath<<" ("<<texture<<")");
			return;
		}
	}
}

void FreeTexture(int i)
{
	Texture* t = &g_texture[i];

	if(t->loaded)
	{
		t->loaded = false;
		glDeleteTextures(1, &t->texname);
		delete t->pixels;
		t->pixels = NULL;
	}
}

int NewTexture()
{
	for(int i=0; i<TEXTURES; i++)
		if(!g_texture[i].loaded)
			return i;

	return -1;
}

bool TextureLoaded(unsigned int texture, const char* relative, bool transp, bool clamp, bool mipmaps, unsigned int& texindex, bool reload)
{
	char corrected[1024];
	strcpy(corrected, relative);
	CorrectSlashes(corrected);

	if(!reload)
	{
		texindex = NewTexture();

		if((int)texindex < 0)
		{
			texindex = 0;	// Give a harmless texture index
			return false;
		}
	}

	//g_texindex = texindex;
	Texture* t = &g_texture[texindex];
	t->loaded = true;
	t->fullpath = corrected;
	t->texname = texture;
	t->width = g_texwidth;
	t->height = g_texheight;
	t->transp = transp;
	t->clamp = clamp;
	t->mipmaps = mipmaps;

	t->sky = false;
	t->breakable = false;
	t->passthru = false;
	t->grate = false;
	t->ladder = false;
	t->water = false;
	t->fabric = false;

	if(strstr(corrected, "^"))
		t->sky = true;
	if(strstr(corrected, "!"))
		t->breakable = true;
	if(strstr(corrected, "~"))
		t->passthru = true;
	if(strstr(corrected, "`"))
		t->grate = true;
	if(strstr(corrected, "#"))
		t->ladder = true;
	if(strstr(corrected, "$"))
		t->water = true;
	if(strstr(corrected, "@"))
		t->fabric = true;

	return true;
}

void FreeTextures()
{
	for(int i=0; i<TEXTURES; i++)
	{
		if(!g_texture[i].loaded)
			continue;

		glDeleteTextures(1, &g_texture[i].texname);
		g_texture[i].loaded = false;	// Needed to reload textures. EDIT: not.
		delete g_texture[i].pixels;
		g_texture[i].pixels = NULL;
	}
}

void FindTextureExtension(char *relative)
{
	char strJPGPath[SPE_MAX_PATH] = {0};
	char strPNGPath[SPE_MAX_PATH] = {0};
	char strTGAPath[SPE_MAX_PATH]    = {0};
	char strBMPPath[SPE_MAX_PATH]    = {0};
	FILE *fp = NULL;

	//GetCurrentDirectory(SPE_MAX_PATH, strJPGPath);

	//strcat(strJPGPath, "\\");
	FullPath("", strJPGPath);

	strcat(strJPGPath, relative);
	strcpy(strTGAPath, strJPGPath);
	strcpy(strBMPPath, strTGAPath);
	strcpy(strPNGPath, strBMPPath);

	strcat(strJPGPath, ".jpg");
	strcat(strTGAPath, ".tga");
	strcat(strBMPPath, ".bmp");
	strcat(strPNGPath, ".png");

	if((fp = fopen(strJPGPath, "rb")) != NULL)
	{
		fclose(fp);
		strcat(relative, ".jpg");
		return;
	}

	if((fp = fopen(strPNGPath, "rb")) != NULL)
	{
		fclose(fp);
		strcat(relative, ".png");
		return;
	}

	if((fp = fopen(strTGAPath, "rb")) != NULL)
	{
		fclose(fp);
		strcat(relative, ".tga");
		return;
	}

	if((fp = fopen(strBMPPath, "rb")) != NULL)
	{
		fclose(fp);
		strcat(relative, ".bmp");
		return;
	}
}

bool Load1Texture()
{
	if(g_lastLTex+1 < g_texload.size())
		SetStatus(g_texload[g_lastLTex+1].relative);

	CHECKGLERROR();

	if(g_lastLTex >= 0)
	{
		TextureToLoad* t = &g_texload[g_lastLTex];
		if(t->reload)
			CreateTex(t->texindex, t->relative, t->clamp, t->mipmaps, t->reload);
		else
			CreateTex(*t->ptexindex, t->relative, t->clamp, t->mipmaps, t->reload);
	}

	g_lastLTex ++;

	if(g_lastLTex >= g_texload.size())
	{
		return false;	// Done loading all textures
	}

	return true;	// Not finished loading textures
}

void QueueTexture(unsigned int* texindex, const char* relative, bool clamp, bool mipmaps)
{
	TextureToLoad toLoad;
	toLoad.ptexindex = texindex;
	strcpy(toLoad.relative, relative);
	toLoad.clamp = clamp;
	toLoad.reload = false;
	toLoad.mipmaps = mipmaps;

	g_texload.push_back(toLoad);
}

void RequeueTexture(unsigned int texindex, const char* relative, bool clamp, bool mipmaps)
{
	TextureToLoad toLoad;
	toLoad.texindex = texindex;
	strcpy(toLoad.relative, relative);
	toLoad.clamp = clamp;
	toLoad.reload = true;
	toLoad.mipmaps = mipmaps;

	g_texload.push_back(toLoad);
}

void ReloadTextures()
{
	for(int i=0; i<TEXTURES; i++)
	{
		Texture* t = &g_texture[i];

		if(!t->loaded)
			continue;

		std::string rel;
		rel = MakeRelative(t->fullpath.c_str());
		RequeueTexture(i, rel.c_str(), t->clamp, t->mipmaps);
	}
}

LoadedTex* LoadTexture(const char* full)
{
	if(strstr(full, ".jpg") || strstr(full, ".JPG") || strstr(full, ".jpeg") || strstr(full, ".JPEG") || strstr(full, ".jpe") || strstr(full, ".JPE"))
	{
		//return LoadJPG(full);
		return LoadJPG2(full);
	}
	else if(strstr(full, ".png") || strstr(full, ".PNG"))
	{
		return LoadPNG2(full);
	}
	else if(strstr(full, ".tga") || strstr(full, ".TGA"))
	{
		return LoadTGA(full);
	}
	else if(strstr(full, ".bmp") || strstr(full, ".BMP"))
	{
		return LoadBMP(full);
	}

	Log("Unrecognized texture file extension: %s. Only .jpg .png .tga and .bmp are accepted.", full);

	return NULL;
}

bool CreateTex(LoadedTex* pImage, unsigned int* texname, bool clamp, bool mipmaps)
{
	if(!pImage)
		return false;
	if(!pImage->data)
		return false;

	// Generate a texture with the associative texture ID stored in the array
	glGenTextures(1, texname);
	
	CHECKGLERROR();
	// This sets the alignment requirements for the start of each pixel row in memory.
	glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
	CHECKGLERROR();
	
	// Bind the texture to the texture arrays index and init the texture
	glBindTexture(GL_TEXTURE_2D, *texname);
	
	CHECKGLERROR();
	// Assume that the texture is a 24 bit RGB texture (We convert 16-bit ones to 24-bit)
	int internalFormat = GL_RGB;
	int textureType = GL_RGB;
	bool transp = false;
	
	// If the image is 32-bit (4 channels), then we need to specify GL_RGBA for an alpha
	if(pImage->channels == 4)
	{
		internalFormat = GL_RGBA;
		textureType = GL_RGBA;
		transp = true;
	}
	//grayscale
	else if(pImage->channels == 1)
	{
		internalFormat = GL_ALPHA;
		textureType = GL_ALPHA;
	}
	
	CHECKGLERROR();
	
#if 0	//old
	
	if(mipmaps)
	{
#if 0
		glEnable(GL_TEXTURE_2D);	// ATI fix
		// Option 1: with mipmaps
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
		if(clamp)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}
		
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
		//glTexImage2D(GL_TEXTURE_2D, 0, textureType, pImage->sizex, pImage->sizey, 0, textureType, GL_UNSIGNED_BYTE, pImage->data);
		//glGenerateMipmap(GL_TEXTURE_2D);
		gluBuild2DMipmaps(GL_TEXTURE_2D, textureType, pImage->sizex, pImage->sizey, textureType, GL_UNSIGNED_BYTE, pImage->data);
		
		CheckGLError(__FILE__, __LINE__);
#else
		//if(transp)
		if(true)
		{
			//linear filter, better for thin details with transparency
			
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			
			if(clamp)
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			}
			else
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			}
			
			glTexImage2D(GL_TEXTURE_2D, 0, textureType, pImage->sizex, pImage->sizey, 0, textureType, GL_UNSIGNED_BYTE, pImage->data);
			
			CheckGLError(__FILE__, __LINE__);
		}
		else
		{
			glEnable(GL_TEXTURE_2D);	// ATI fix
			// Option 1: with mipmaps
			
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			
			if(clamp)
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			}
			else
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			}
			
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
			//glTexImage2D(GL_TEXTURE_2D, 0, textureType, pImage->sizex, pImage->sizey, 0, textureType, GL_UNSIGNED_BYTE, pImage->data);
			//glGenerateMipmap(GL_TEXTURE_2D);
			gluBuild2DMipmaps(GL_TEXTURE_2D, textureType, pImage->sizex, pImage->sizey, textureType, GL_UNSIGNED_BYTE, pImage->data);
			
			CheckGLError(__FILE__, __LINE__);
		}
#endif
	}
	else
	{
		// Option 2: without mipmaps
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		
		if(clamp)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}
		
		glTexImage2D(GL_TEXTURE_2D, 0, textureType, pImage->sizex, pImage->sizey, 0, textureType, GL_UNSIGNED_BYTE, pImage->data);
		
		CheckGLError(__FILE__, __LINE__);
	}
	
	CheckGLError(__FILE__, __LINE__);
#endif
	
	
#if 1
	
	//Log("mipmaps:"<<(int)mipmaps<<" :"<<relative);
	
	if(mipmaps)
	{
		glEnable(GL_TEXTURE_2D);	// ATI fix
		// Option 1: with mipmaps
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
#ifdef PLATFORM_GL14
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
#endif
		
		if(clamp)
		{
#ifdef PLATFORM_GL14
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
#endif
			
#ifdef PLATFORM_GLES20
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#endif
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}
		
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, pImage->sizex, pImage->sizey, 0, textureType, GL_UNSIGNED_BYTE, pImage->data);
		
		glGenerateMipmap(GL_TEXTURE_2D);  //undeclared identifier in xcode!?
		
		//gluBuild2DMipmaps(GL_TEXTURE_2D, internalFormat, pImage->sizex, pImage->sizey, textureType, GL_UNSIGNED_BYTE, pImage->data);  //undeclared identifier in xcode!?
		
		CHECKGLERROR();
	}
	else
	{
		// Option 2: without mipmaps
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		
		if(clamp)
		{
#ifdef PLATFORM_GL14
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
#endif
			
#ifdef PLATFORM_GLES20
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#endif
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}
		
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, pImage->sizex, pImage->sizey, 0, textureType, GL_UNSIGNED_BYTE, pImage->data);
		
		CHECKGLERROR();
	}
	
	CHECKGLERROR();
#else
	// Option 3: without mipmaps linear
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, textureType, pImage->sizex, pImage->sizey, 0, textureType, GL_UNSIGNED_BYTE, pImage->data);
#endif

	return true;
}

bool CreateTex(unsigned int &texindex, const char* relative, bool clamp, bool mipmaps, bool reload)
{
	CHECKGLERROR();

	if(!relative)
		return false;

	if(!reload)
		if(FindTexture(texindex, relative))
			return true;

	// Define a pointer to a LoadedTex
	LoadedTex *pImage = NULL;

	char full[1024];
	FullPath(relative, full);

	pImage = LoadTexture(full);

	// Make sure valid image data was given to pImage, otherwise return false
	if(pImage == NULL)
	{
		Log("Failed to load %s\r\n", relative);
		
		
		if(!reload)
			texindex = 0;	// Give a harmless texture index
		
		return false;
	}
	
	unsigned int texname;
	
	if(!CreateTex(pImage, &texname, clamp, mipmaps))
	{
		delete pImage;								// Free the image structure
		return false;
	}

	bool transp = false;
	
	if(pImage->channels == 4)
	{
		transp = true;
	}
	
	g_texwidth = pImage->sizex;
	g_texheight = pImage->sizey;

	TextureLoaded(texname, relative, transp, clamp, mipmaps, texindex, reload);
	
	if(pImage)
	{
		g_texture[texindex].pixels = pImage;
		//delete pImage;								// Free the image structure

		Log(relative);
	}
	
	// Return a success
	return true;
}

void RequeueTextures()
{
	FreeTextures();

	for(int i=0; i<TEXTURES; i++)
	{
		if(g_texture[i].loaded)
			RequeueTexture(i, g_texture[i].fullpath.c_str(), g_texture[i].clamp, g_texture[i].mipmaps);
	}

	//LoadParticles();
	//LoadProjectiles();
	//LoadTerrainTextures();
	//LoadUnitSprites();
	//BSprites();
}

void DiffPath(const char* basepath, char* diffpath)
{
	strcpy(diffpath, basepath);
	//StripExt(diffpath);
	strcat(diffpath, ".jpg");
}

void DiffPathPNG(const char* basepath, char* diffpath)
{
	strcpy(diffpath, basepath);
	//StripExt(diffpath);
	strcat(diffpath, ".png");
}

void SpecPath(const char* basepath, char* specpath)
{
	strcpy(specpath, basepath);
	//StripExt(specpath);
	strcat(specpath, ".spec.jpg");
}

void NormPath(const char* basepath, char* normpath)
{
	strcpy(normpath, basepath);
	//StripExt(normpath);
	strcat(normpath, ".norm.jpg");
}

void OwnPath(const char* basepath, char* ownpath)
{
	strcpy(ownpath, basepath);
	//StripExt(ownpath);
	strcat(ownpath, ".team.png");
}

void AllocTex(LoadedTex* empty, int width, int height, int channels)
{
	empty->data = (unsigned char*)malloc(width * height * channels * sizeof(unsigned char));
	empty->sizex = width;
	empty->sizey = height;
	empty->channels = channels;

	if(!empty->data)
	{
		OutOfMem(__FILE__, __LINE__);
	}
}

void Blit(LoadedTex* src, LoadedTex* dest, Vec2i pos)
{
	if(src == NULL || src->data == NULL)
		return;

	for(int x=0; x<src->sizex; x++)
	{
		if(x+pos.x < 0)
			continue;

		if(x+pos.x >= dest->sizex)
			continue;

		for(int y=0; y<src->sizey; y++)
		{
			if(y+pos.y < 0)
				continue;

			if(y+pos.y >= dest->sizey)
				continue;

			int srcpixel = x*src->channels + y*src->channels*src->sizex;
			int destpixel = (x+pos.x)*dest->channels + (y+pos.y)*dest->channels*dest->sizex;

			dest->data[destpixel + 0] = src->data[srcpixel + 0];
			dest->data[destpixel + 1] = src->data[srcpixel + 1];
			dest->data[destpixel + 2] = src->data[srcpixel + 2];

			if(dest->channels > 3)
			{
				if(src->channels <= 3)
					dest->data[destpixel + 3] = 255;
				else
					dest->data[destpixel + 3] = src->data[srcpixel + 3];
			}
		}
	}
}

static std::vector<JOCTET> my_buffer;
#define BLOCK_SIZE 16384

void my_init_destination(j_compress_ptr cinfo)
{
    my_buffer.resize(BLOCK_SIZE);
    cinfo->dest->next_output_byte = &my_buffer[0];
    cinfo->dest->free_in_buffer = my_buffer.size();
}

boolean my_empty_output_buffer(j_compress_ptr cinfo)
{
    size_t oldsize = my_buffer.size();
    my_buffer.resize(oldsize + BLOCK_SIZE);
    cinfo->dest->next_output_byte = &my_buffer[oldsize];
    cinfo->dest->free_in_buffer = my_buffer.size() - oldsize;
    return true;
}

void my_term_destination(j_compress_ptr cinfo)
{
    my_buffer.resize(my_buffer.size() - cinfo->dest->free_in_buffer);
}


void SaveJPEG2(const char* fullpath, LoadedTex* image, float quality)
{
	FILE *outfile;
	if ((outfile = fopen(fullpath, "wb")) == NULL)
	{
		return;
	}

	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr       jerr;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, NULL);
	cinfo.dest->init_destination = &my_init_destination;
	cinfo.dest->empty_output_buffer = &my_empty_output_buffer;
	cinfo.dest->term_destination = &my_term_destination;

	cinfo.image_width      = image->sizex;
	cinfo.image_height     = image->sizey;
	cinfo.input_components = 3;
	cinfo.in_color_space   = JCS_RGB;

	jpeg_set_defaults(&cinfo);
	
	cinfo.dest->init_destination = &my_init_destination;
	cinfo.dest->empty_output_buffer = &my_empty_output_buffer;
	cinfo.dest->term_destination = &my_term_destination;

	/*set the quality [0..100]  */
    //have to use TRUE instead of true or else doesn't work
	//in xcode which requires conversion to custom type boolean
	//jpeg_set_quality (&cinfo, 100*quality, TRUE);
	//jpeg_start_compress(&cinfo, TRUE);
	jpeg_set_quality (&cinfo, 100*quality, (boolean)TRUE);
	jpeg_start_compress(&cinfo, (boolean)TRUE);

	JSAMPROW row_pointer;
	int row_stride = image->sizex * 3;

	while (cinfo.next_scanline < cinfo.image_height)
	{
		row_pointer = (JSAMPROW) &image->data[cinfo.next_scanline*row_stride];
		jpeg_write_scanlines(&cinfo, &row_pointer, 1);
	}

	jpeg_finish_compress(&cinfo);

	fwrite(&my_buffer[0], my_buffer.size(), 1, outfile);
	fclose(outfile);

	jpeg_destroy_compress(&cinfo);
}

void SaveJPEG(const char* fullpath, LoadedTex* image, float quality)
{
#if 1
	FILE *outfile;
	if ((outfile = fopen(fullpath, "wb")) == NULL)
	{
		return;
	}

	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr       jerr;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, outfile);

	cinfo.image_width      = image->sizex;
	cinfo.image_height     = image->sizey;
	cinfo.input_components = 3;
	cinfo.in_color_space   = JCS_RGB;

	jpeg_set_defaults(&cinfo);
	/*set the quality [0..100]  */
    //have to use TRUE instead of true or else doesn't work
	//in xcode which requires conversion to custom type boolean
	//jpeg_set_quality (&cinfo, 100*quality, TRUE);
	//jpeg_start_compress(&cinfo, TRUE);
	jpeg_set_quality (&cinfo, 100*quality, (boolean)TRUE);
	jpeg_start_compress(&cinfo, (boolean)TRUE);

	JSAMPROW row_pointer;
	int row_stride = image->sizex * 3;

	while (cinfo.next_scanline < cinfo.image_height)
	{
		row_pointer = (JSAMPROW) &image->data[cinfo.next_scanline*row_stride];
		jpeg_write_scanlines(&cinfo, &row_pointer, 1);
	}

	jpeg_finish_compress(&cinfo);

	fclose(outfile);

	jpeg_destroy_compress(&cinfo);
#endif
}

static FILE* savepngfp = NULL;

void png_file_write(png_structp png_ptr, png_bytep data, png_size_t length) 
{
	fwrite(data, 1, length, savepngfp);
}

void png_file_flush(png_structp png_ptr)
{
	fflush(savepngfp);
}

//only for palette+alpha
void Palletize(png_color_16 Colors[PNG_MAX_PALETTE_LENGTH],
		png_byte Trans[PNG_MAX_PALETTE_LENGTH],
		int* npal,
		LoadedTex* lt,
		png_colorp palette)
{
	*npal = 0;

	//1	
	int* closest = new int [lt->sizex * lt->sizey];
	int mentions[PNG_MAX_PALETTE_LENGTH];
	memset(closest, 0, lt->sizex * lt->sizey * sizeof(int));
	memset(mentions, 0, PNG_MAX_PALETTE_LENGTH * sizeof(int));

	for(int x=0; x<lt->sizex; ++x)
	{
tryout:
		for(int y=0; y<lt->sizey; ++y)
		{
			int nearpi = -1;
			int neard = -1;


			for(int pi=0; pi<PNG_MAX_PALETTE_LENGTH; ++pi)
			{
				int d = iabs((int)(palette[pi].red-lt->data[lt->channels*(x+y*lt->sizex)+0])) * 
					iabs((int)(palette[pi].green-lt->data[lt->channels*(x+y*lt->sizex)+1])) * 
					iabs((int)(palette[pi].blue-lt->data[lt->channels*(x+y*lt->sizex)+2]));
				
				if(nearpi < 0)
					nearpi = pi;
				if(neard < 0)
					neard = d;

				//if(mentions[pi] <= mentions[nearpi] &&
				//	d >= neard)
				if(d < neard)
				{
					nearpi = pi;
					neard = d;
				}
			}

			for(int pi=0; pi<PNG_MAX_PALETTE_LENGTH; ++pi)
			{
				int d = iabs((int)(palette[pi].red-lt->data[lt->channels*(x+y*lt->sizex)+0])) * 
					iabs((int)(palette[pi].green-lt->data[lt->channels*(x+y*lt->sizex)+1])) * 
					iabs((int)(palette[pi].blue-lt->data[lt->channels*(x+y*lt->sizex)+2]));
				
				if(nearpi < 0)
					nearpi = pi;
				if(neard < 0)
					neard = d;

				if(mentions[pi] <= mentions[nearpi] &&
					d >= neard)
				{
					nearpi = pi;
					neard = d;
					if(d > 0)
						mentions[pi] = 0;
					//x=0;
					//y=0;
					//goto tryout;
				}
			}

			closest[(x+y*lt->sizex)] = nearpi;
			mentions[nearpi]++;
			palette[nearpi].red = lt->data[lt->channels*(x+y*lt->sizex)+0];
			palette[nearpi].green = lt->data[lt->channels*(x+y*lt->sizex)+1];
			palette[nearpi].blue = lt->data[lt->channels*(x+y*lt->sizex)+2];
		}
	}

	//2
	//LoadedTex closest;
	//AllocTex(&closest, lt->sizex, lt->sizey, lt->channels);
	//memset(closest.data, 0, lt->sizex * lt->sizey * lt->channels);
	memset(closest, 0, lt->sizex * lt->sizey * sizeof(int));
	memset(mentions, 0, PNG_MAX_PALETTE_LENGTH * sizeof(int));

	for(int x=0; x<lt->sizex; ++x)
	{
		for(int y=0; y<lt->sizey; ++y)
		{
			int nearpi = -1;
			int neard = -1;
			for(int pi=0; pi<PNG_MAX_PALETTE_LENGTH; ++pi)
			{
				int d = iabs((int)(palette[pi].red-lt->data[lt->channels*(x+y*lt->sizex)+0])) * 
					iabs((int)(palette[pi].green-lt->data[lt->channels*(x+y*lt->sizex)+1])) * 
					iabs((int)(palette[pi].blue-lt->data[lt->channels*(x+y*lt->sizex)+2]));

				if(nearpi < 0 || neard > d)
				{
					nearpi = pi;
					neard = d;
				}
			}

			closest[(x+y*lt->sizex)] = nearpi;
			mentions[nearpi]++;
			//lt->data[lt->channels*(x+y*lt->sizex)+0] = palette[pi].red;
			//lt->data[lt->channels*(x+y*lt->sizex)+1] = palette[pi].green;
			//lt->data[lt->channels*(x+y*lt->sizex)+2] = palette[pi].blue;
		}
	}

	int avga[PNG_MAX_PALETTE_LENGTH];
	memset(avga, 0, sizeof(int)*PNG_MAX_PALETTE_LENGTH);

	for(int x=0; x<lt->sizex; ++x)
	{
		for(int y=0; y<lt->sizey; ++y)
		{
			int nearpi = closest[(x+y*lt->sizex)];
			avga[nearpi] += lt->data[(x+y*lt->sizex)*lt->channels] *2000/255;
		}
	}

	for(int pi=0; pi<PNG_MAX_PALETTE_LENGTH; ++pi)
		avga[pi] /= imax(1, 2000 * mentions[pi] / 255);

	for(int pi=0; pi<PNG_MAX_PALETTE_LENGTH; ++pi)
		Trans[pi] = imax(0, imin(255, avga[pi]));

	delete [] closest;

	*npal = PNG_MAX_PALETTE_LENGTH;
}

int SavePNG2(const char* fullpath, LoadedTex* image)
{
	//FILE *fp;
	png_structp png_ptr;
	png_infop info_ptr;
	//png_colorp palette;

	/* Open the file */
	savepngfp = fopen(fullpath, "wb");
	if (savepngfp == NULL)
		return (ERROR);

	/* Create and initialize the png_struct with the desired error handler
	 * functions.  If you want to use the default stderr and longjump method,
	 * you can supply NULL for the last three parameters.  We also check that
	 * the library version is compatible with the one used at compile time,
	 * in case we are using dynamically linked libraries.  REQUIRED.
	 */
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
	                                  (png_voidp) NULL, NULL, NULL);

	if (png_ptr == NULL)
	{
		fclose(savepngfp);
		return (ERROR);
	}

	/* Allocate/initialize the image information data.  REQUIRED */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		fclose(savepngfp);
		png_destroy_write_struct(&png_ptr,  NULL);
		return (ERROR);
	}

	int color_type = PNG_COLOR_TYPE_RGB;

	if(image->channels == 4)
		color_type = PNG_COLOR_TYPE_RGBA;

	if(g_usepalette)
	{
		color_type = PNG_COLOR_TYPE_PALETTE;
	}

	int bit_depth = 8;

	if(g_savebitdepth == 2 ||
		g_savebitdepth == 4 ||
		g_savebitdepth == 8 ||
		g_savebitdepth == 16)
	{
		bit_depth = g_savebitdepth;
	}
	else if(!g_hidetexerr)
	{
		ErrMess("PNG write_bit_depth", "write_bit_depth must be either 2, 4, 8, or 16. Defaulting to 8.");
	}

	png_set_IHDR(png_ptr, info_ptr, image->sizex, image->sizey, bit_depth, color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	/* Set error handling.  REQUIRED if you aren't supplying your own
	 * error handling functions in the png_create_write_struct() call.
	 */
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		/* If we get here, we had a problem writing the file */
		fclose(savepngfp);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return (ERROR);
	}

	if(g_usepalette)
	{
		png_colorp palette = (png_colorp)png_malloc(png_ptr, PNG_MAX_PALETTE_LENGTH * sizeof(png_color));//4
		if (!palette) {
			ErrMess("PNG error", "Error allocating palette");
			OUTOFMEM();
			fclose(savepngfp);
			png_destroy_write_struct(&png_ptr, &info_ptr);//
			return false;
		}
		
		png_color_16 Colors[PNG_MAX_PALETTE_LENGTH];
		png_byte Trans[PNG_MAX_PALETTE_LENGTH];
		int npal = 0;

		Palletize(Colors, Trans, &npal, image, palette);
		//BGColor[0].index = 0;
		//TrnsColor[0] = 100; //0 = transparent , 255 = opaque
		png_set_PLTE(png_ptr, info_ptr, palette, PNG_MAX_PALETTE_LENGTH);//12
		//png_set_tRNS(png_ptr, info_ptr, Trans, npal, NULL);//Colors);
	}

	/* One of the following I/O initialization functions is REQUIRED */

	/* Set up the output control if you are using standard C streams */
	//png_init_io(png_ptr, savepngfp);
	png_set_write_fn(png_ptr, NULL, png_file_write, png_file_flush);

	png_bytep* row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * image->sizey);

	if(!row_pointers)
	{
		OutOfMem(__FILE__, __LINE__);
		return NULL;
	}

	for (int y=0; y<image->sizey; y++)
		row_pointers[y] = (png_byte*)&image->data[y*image->sizex*image->channels];

	png_text text_ptr[1];

	char srcstr[123];
	sprintf(srcstr, "Rendered using %s Version %d", TITLE, APPVERSION);
#ifdef USESTEAM
	strcat(srcstr, " Authorized Steam Build");
#endif
	text_ptr[0].key = "Source";
	text_ptr[0].text = srcstr;
	text_ptr[0].compression = PNG_TEXT_COMPRESSION_NONE;
	// text_ptr[2].compression = PNG_TEXT_COMPRESSION_zTXt;
#ifdef PNG_iTXt_SUPPORTED
	text_ptr[0].lang = NULL;
#endif
	text_ptr[0].lang_key = NULL;

	png_set_text(png_ptr, info_ptr, text_ptr, 1);

	png_write_info(png_ptr, info_ptr);

	png_write_image(png_ptr, row_pointers);
	png_write_end(png_ptr, NULL);

	//for (y=0; y<image->sizey; y++)
	//   free(row_pointers[y]);
	free(row_pointers);


	/* This is the easy way.  Use it if you already have all the
	 * image info living in the structure.  You could "|" many
	 * PNG_TRANSFORM flags into the png_transforms integer here.
	 */
	//png_write_png(png_ptr, info_ptr, NULL, NULL);

	/* If you png_malloced a palette, free it here (don't free info_ptr->palette,
	 * as recommended in versions 1.0.5m and earlier of this example; if
	 * libpng mallocs info_ptr->palette, libpng will free it).  If you
	 * allocated it with malloc() instead of png_malloc(), use free() instead
	 * of png_free().
	 */
	//png_free(png_ptr, palette);
	//palette = NULL;

	/* Similarly, if you png_malloced any data that you passed in with
	 * png_set_something(), such as a hist or trans array, free it here,
	 * when you can be sure that libpng is through with it.
	 */
	//png_free(png_ptr, trans);
	//trans = NULL;
	/* Whenever you use png_free() it is a good idea to set the pointer to
	 * NULL in case your application inadvertently tries to png_free() it
	 * again.  When png_free() sees a NULL it returns without action, thus
	 * avoiding the double-free security problem.
	 */

	/* Clean up after the write, and free any memory allocated */
	png_destroy_write_struct(&png_ptr, &info_ptr);

	/* Close the file */
	fclose(savepngfp);

	/* That's it */
	return (1);
}

int SavePNG(const char* fullpath, LoadedTex* image)
{
	FILE *fp;
	png_structp png_ptr;
	png_infop info_ptr;
	//png_colorp palette;

	/* Open the file */
	fp = fopen(fullpath, "wb");
	if (fp == NULL)
		return (ERROR);

	/* Create and initialize the png_struct with the desired error handler
	 * functions.  If you want to use the default stderr and longjump method,
	 * you can supply NULL for the last three parameters.  We also check that
	 * the library version is compatible with the one used at compile time,
	 * in case we are using dynamically linked libraries.  REQUIRED.
	 */
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
	                                  (png_voidp) NULL, NULL, NULL);

	if (png_ptr == NULL)
	{
		fclose(fp);
		return (ERROR);
	}

	/* Allocate/initialize the image information data.  REQUIRED */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		fclose(fp);
		png_destroy_write_struct(&png_ptr,  NULL);
		return (ERROR);
	}

	int color_type = PNG_COLOR_TYPE_RGB;

	if(image->channels == 4)
		color_type = PNG_COLOR_TYPE_RGBA;

	if(g_usepalette)
	{
		color_type = PNG_COLOR_TYPE_PALETTE;
	}

	int bit_depth = 8;

	if(g_savebitdepth == 2 ||
		g_savebitdepth == 4 ||
		g_savebitdepth == 8 ||
		g_savebitdepth == 16)
	{
		bit_depth = g_savebitdepth;
	}
	else if(!g_hidetexerr)
	{
		ErrMess("PNG write_bit_depth", "write_bit_depth must be either 2, 4, 8, or 16. Defaulting to 8.");
	}

	png_set_IHDR(png_ptr, info_ptr, image->sizex, image->sizey, bit_depth, color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	/* Set error handling.  REQUIRED if you aren't supplying your own
	 * error handling functions in the png_create_write_struct() call.
	 */
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		/* If we get here, we had a problem writing the file */
		fclose(fp);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return (ERROR);
	}

	if(g_usepalette)
	{
		png_colorp palette = (png_colorp)png_malloc(png_ptr, PNG_MAX_PALETTE_LENGTH * sizeof(png_color));//4
		if (!palette) {
			ErrMess("PNG error", "Error allocating palette");
			OUTOFMEM();
			fclose(savepngfp);
			png_destroy_write_struct(&png_ptr, &info_ptr);//
			return false;
		}
		png_set_PLTE(png_ptr, info_ptr, palette, PNG_MAX_PALETTE_LENGTH);//12
		
		png_color_16 Colors[PNG_MAX_PALETTE_LENGTH];
		png_byte Trans[PNG_MAX_PALETTE_LENGTH];
		int npal = 0;

		Palletize(Colors, Trans, &npal, image, palette);
		//BGColor[0].index = 0;
		//TrnsColor[0] = 100; //0 = transparent , 255 = opaque
		png_set_tRNS(png_ptr, info_ptr, Trans, npal, NULL);//Colors);
	}

	/* One of the following I/O initialization functions is REQUIRED */

	/* Set up the output control if you are using standard C streams */
	png_init_io(png_ptr, fp);
	//png_set_write_fn(png_ptr, NULL, png_file_write, png_file_flush);

	png_bytep* row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * image->sizey);

	if(!row_pointers)
	{
		OutOfMem(__FILE__, __LINE__);
		return NULL;
	}

	//if(bit_depth == 8)
	{
		for (int y=0; y<image->sizey; y++)
			row_pointers[y] = (png_byte*)&image->data[y*image->sizex*image->channels];
	}
	//else
	{
		for(int y=0; y<image->sizey; y++)
		{
			for(int x=0; x<image->sizex; x++)
			{
	//			image->data[y*image->sizex*image->channels]
	//			 = image->data[y*image->sizex*image->channels];
			}
		}
	}

	png_text text_ptr[1];

	char srcstr[123];
	sprintf(srcstr, "Rendered using %s Version %d", TITLE, APPVERSION);
#ifdef USESTEAM
	strcat(srcstr, " Authorized Steam Build");
#endif
	text_ptr[0].key = "Source";
	text_ptr[0].text = srcstr;
	text_ptr[0].compression = PNG_TEXT_COMPRESSION_NONE;
	// text_ptr[2].compression = PNG_TEXT_COMPRESSION_zTXt;
#ifdef PNG_iTXt_SUPPORTED
	text_ptr[0].lang = NULL;
#endif
	text_ptr[0].lang_key = NULL;

	png_set_text(png_ptr, info_ptr, text_ptr, 1);

	png_write_info(png_ptr, info_ptr);
	png_write_image(png_ptr, row_pointers);
	png_write_end(png_ptr, NULL);

	//for (y=0; y<image->sizey; y++)
	//   free(row_pointers[y]);
	free(row_pointers);


	/* This is the easy way.  Use it if you already have all the
	 * image info living in the structure.  You could "|" many
	 * PNG_TRANSFORM flags into the png_transforms integer here.
	 */
	//png_write_png(png_ptr, info_ptr, NULL, NULL);

	/* If you png_malloced a palette, free it here (don't free info_ptr->palette,
	 * as recommended in versions 1.0.5m and earlier of this example; if
	 * libpng mallocs info_ptr->palette, libpng will free it).  If you
	 * allocated it with malloc() instead of png_malloc(), use free() instead
	 * of png_free().
	 */
	//png_free(png_ptr, palette);
	//palette = NULL;

	/* Similarly, if you png_malloced any data that you passed in with
	 * png_set_something(), such as a hist or trans array, free it here,
	 * when you can be sure that libpng is through with it.
	 */
	//png_free(png_ptr, trans);
	//trans = NULL;
	/* Whenever you use png_free() it is a good idea to set the pointer to
	 * NULL in case your application inadvertently tries to png_free() it
	 * again.  When png_free() sees a NULL it returns without action, thus
	 * avoiding the double-free security problem.
	 */

	/* Clean up after the write, and free any memory allocated */
	png_destroy_write_struct(&png_ptr, &info_ptr);

	/* Close the file */
	fclose(fp);

	/* That's it */
	return (1);
}

void FlipImage(LoadedTex* image)
{
	int x;
	int y2;
	byte temp[4];
	int stride = image->sizex * image->channels;

	for(int y=0; y<image->sizey/2; y++)
	{
		y2 = image->sizey - y - 1;

		unsigned char *pLine = &(image->data[stride * y]);
		unsigned char *pLine2 = &(image->data[stride * y2]);

		for(x = 0; x < image->sizex * image->channels; x += image->channels)
		{
			temp[0] = pLine[x + 0];
			temp[1] = pLine[x + 1];
			temp[2] = pLine[x + 2];
			if(image->channels == 4)
				temp[3] = pLine[x + 3];

			pLine[x + 0] = pLine2[x + 0];
			pLine[x + 1] = pLine2[x + 1];
			pLine[x + 2] = pLine2[x + 2];
			if(image->channels == 4)
				pLine[x + 3] = pLine2[x + 3];

			pLine2[x + 0] = temp[0];
			pLine2[x + 1] = temp[1];
			pLine2[x + 2] = temp[2];
			if(image->channels == 4)
				pLine2[x + 3] = temp[3];
		}
	}
}

// Warning: this function modifies the image data to switch RGB to BGR
int SaveBMP(const char* fullpath, LoadedTex* image)
{
#ifdef PLATFORM_WIN
	FILE* filePtr;
	BITMAPFILEHEADER bitmapFileHeader;
	BITMAPINFOHEADER bitmapInfoHeader;
	int imageIdx;
	unsigned char tempRGB;

	filePtr = fopen(fullpath, "wb");
	if(!filePtr)
		return 0;

	bitmapFileHeader.bfSize = sizeof(BITMAPFILEHEADER);
	bitmapFileHeader.bfType = 0x4D42;
	bitmapFileHeader.bfReserved1 = 0;
	bitmapFileHeader.bfReserved2 = 0;
	bitmapFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	bitmapInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapInfoHeader.biPlanes = 1;
	bitmapInfoHeader.biBitCount = 24;
	bitmapInfoHeader.biCompression = BI_RGB;
	bitmapInfoHeader.biSizeImage = image->sizex * abs(image->sizey) * 3;
	bitmapInfoHeader.biXPelsPerMeter = 0;
	bitmapInfoHeader.biYPelsPerMeter = 0;
	bitmapInfoHeader.biClrUsed = 0;
	bitmapInfoHeader.biClrImportant = 0;
	bitmapInfoHeader.biWidth = image->sizex;
	bitmapInfoHeader.biHeight = image->sizey;

	for(imageIdx = 0; imageIdx < bitmapInfoHeader.biSizeImage; imageIdx += 3)
	{
		tempRGB = image->data[imageIdx];
		image->data[imageIdx] = image->data[imageIdx + 2];
		image->data[imageIdx + 2] = tempRGB;
	}

#if 0
	for(int imageIdx = image->channels * (image->sizex - 1);
	                imageIdx < image->sizex * image->sizey * image->channels;
	                imageIdx += image->channels * image->sizex)
	{
		image->data[imageIdx] = image->data[imageIdx-3];
		image->data[imageIdx+1] = image->data[imageIdx-2];
		image->data[imageIdx+2] = image->data[imageIdx-1];
	}
#endif

	fwrite(&bitmapFileHeader, 1, sizeof(BITMAPFILEHEADER), filePtr);

	fwrite(&bitmapInfoHeader, 1, sizeof(BITMAPINFOHEADER), filePtr);

	fwrite(image->data, 1, bitmapInfoHeader.biSizeImage, filePtr);

	fclose(filePtr);

#endif // PLATFORM_WIN
	return 1;
}


bool SaveRAW(const char* fullpath, LoadedTex* image)
{
	FILE* fp = fopen(fullpath, "wb");

	fwrite(image->data, image->sizex*image->sizey*image->channels, 1, fp);

	fclose(fp);

	return true;
}

void StreamRaw(FILE* fp, unsigned int* texname, Vec2i fullsz, Vec2i srcpos, Vec2i srcsz, Vec2i destsz)
{
	if(srcpos.x < 0)
		srcpos.x = 0;
	if(srcpos.y < 0)
		srcpos.y = 0;
	if(srcpos.x + srcsz.x > fullsz.x)
		srcsz.x = fullsz.x - srcpos.x;
	if(srcpos.y + srcsz.y > fullsz.y)
		srcsz.y = fullsz.y - srcpos.y;

	int stridex = srcsz.x / destsz.x * 3;
	int stridey = srcsz.y / destsz.y * 3 * fullsz.x;

	LoadedTex newtex;
	AllocTex(&newtex, destsz.x, destsz.y, 3);

	int i = 0;

	for(int y=srcpos.y; srcpos.y < srcpos.y+srcsz.y; y+=stridey)
		for(int x=srcpos.x; srcpos.x < srcpos.x+srcsz.x; x+=stridex)
		{
			fseek(fp, y * 3 * fullsz.x + x * 3, SEEK_SET);
			fread(&newtex.data[i], 3, 1, fp);
			i+=3;
		}

	newtex.destroy();
}

void Extract(LoadedTex* original, LoadedTex* empty, int x1, int y1, int x2, int y2)
{
	AllocTex(empty, x2-x1, y2-y1, original->channels);
	
	for(int x=x1; x<x2; x++)
		for(int y=y1; y<y2; y++)
		{
			int i1 = (x + y * original->sizex) * original->channels;
			int i2 = ((x-x1) + (y-y1) * empty->sizex) * empty->channels;
			
			for(int c=0; c<original->channels; c++)
				empty->data[i2] = original->data[i1];
		}
}

void Resample(LoadedTex* original, LoadedTex* empty, Vec2i newdim)
{
#ifdef COMPILEB_DEBUG
	Log("resample...?");
	
#endif

	if(original == NULL || original->data == NULL || original->sizex <= 0 || original->sizey <= 0)
	{
#ifdef COMPILEB_DEBUG
		Log("resample NULL 1");
		
#endif

		empty->data = NULL;

#ifdef COMPILEB_DEBUG
		Log("resample NULL 2");
		
#endif

		empty->sizex = 0;
		empty->sizey = 0;

		if(original != NULL)
			empty->channels = original->channels;

		return;
	}

#ifdef COMPILEB_DEBUG
	Log("resample "<<original->sizex<<","<<original->sizey<<" to "<<newdim.x<<","<<newdim.y);
	
#endif

	AllocTex(empty, newdim.x, newdim.y, original->channels);

	double scaleW =  (double)newdim.x / (double)original->sizex;
	double scaleH = (double)newdim.y / (double)original->sizey;

	for(int cy = 0; cy < newdim.y; cy++)
	{
		for(int cx = 0; cx < newdim.x; cx++)
		{
			int pixel = cy * (newdim.x * original->channels) + cx*original->channels;
			int nearestMatch =  (int)(cy / scaleH) * original->sizex * original->channels + (int)(cx / scaleW) * original->channels;

			empty->data[pixel    ] =  original->data[nearestMatch    ];
			empty->data[pixel + 1] =  original->data[nearestMatch + 1];
			empty->data[pixel + 2] =  original->data[nearestMatch + 2];

			if(original->channels > 3)
				empty->data[pixel + 3] =  original->data[nearestMatch + 3];
		}
	}

#ifdef COMPILEB_DEBUG
	Log("\t done resample");
	
#endif
}
