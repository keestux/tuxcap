#define XMD_H

#include <Magick++.h>
#include "ImageLib.h"
#include <math.h>
#include "Common.h"
#include "SexyAppBase.h"

#if 0
#include <tchar.h>
#include "..\PakLib\PakInterface.h"
#endif

using namespace ImageLib;
using namespace Magick;

ImageLib::Image::Image()
{
	mWidth = 0;
	mHeight = 0;
	mBits = NULL;
}


ImageLib::Image::Image(int width, int height) {
  mWidth = width;
  mHeight = height;
  mBits = new ulong[mWidth * mHeight];
}


ImageLib::Image::~Image()
{
	delete[] mBits;
}

int	ImageLib::Image::GetWidth()
{
	return mWidth;
}

int	ImageLib::Image::GetHeight()
{
	return mHeight;
}

ulong* ImageLib::Image::GetBits()
{
	return mBits;
}

#if 0
//////////////////////////////////////////////////////////////////////////
// PNG Pak Support

static void png_pak_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
	png_size_t check;

	/* fread() returns 0 on error, so it is OK to store this in a png_size_t
	* instead of an int, which is what fread() actually returns.
	*/
	check = (png_size_t)p_fread(data, (png_size_t)1, length,
		(PFILE*)png_ptr->io_ptr);

	if (check != length)
	{
		png_error(png_ptr, "Read Error");
	}
}

bool ImageLib::WriteJPEGImage(const std::string& theFileName,ImageLib::Image* theImage)
{
	FILE *fp;

	if ((fp = fopen(theFileName.c_str(), "wb")) == NULL)
		return false;

	struct jpeg_compress_struct cinfo;
	struct my_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;

	if (setjmp(jerr.setjmp_buffer))
	{
		/* If we get here, the JPEG code has signaled an error.
		 * We need to clean up the JPEG object, close the input file, and return.
		 */
		jpeg_destroy_compress(&cinfo);
		fclose(fp);
		return false;
	}

	jpeg_create_compress(&cinfo);

	cinfo.image_width = theImage->mWidth;
	cinfo.image_height = theImage->mHeight;
	cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    cinfo.optimize_coding = 1;
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, 80, TRUE);

	jpeg_stdio_dest(&cinfo, fp);

	jpeg_start_compress(&cinfo, true);

	int row_stride = theImage->GetWidth() * 3;

	unsigned char* aTempBuffer = new unsigned char[row_stride];

	unsigned long* aSrcPtr = theImage->mBits;

	for (int aRow = 0; aRow < theImage->mHeight; aRow++)
	{
		unsigned char* aDest = aTempBuffer;

		for (int aCol = 0; aCol < theImage->mWidth; aCol++)
		{
			unsigned long src = *(aSrcPtr++);

			*aDest++ = (src >> 16) & 0xFF;
			*aDest++ = (src >>  8) & 0xFF;
			*aDest++ = (src      ) & 0xFF;
		}

		jpeg_write_scanlines(&cinfo, &aTempBuffer, 1);
	}

	delete [] aTempBuffer;

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);

	fclose(fp);

	return true;
}

boo ImageLib::WritePNGImage(const std::string& theFileName,ImageLib::Image* theImage)
{
	png_structp png_ptr;
	png_infop info_ptr;

	FILE *fp;

	if ((fp = fopen(theFileName.c_str(), "wb")) == NULL)
		return false;

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
	  NULL, NULL, NULL);

	if (png_ptr == NULL)
	{
		fclose(fp);
		return false;
	}

	// Allocate/initialize the memory for image information.  REQUIRED.
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		fclose(fp);
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		return false;
	}

   // Set error handling if you are using the setjmp/longjmp method (this is
   // the normal method of doing things with libpng).  REQUIRED unless you
   // set up your own error handlers in the png_create_write_struct() earlier.

	if (setjmp(png_ptr->jmpbuf))
	{
		// Free all of the memory associated with the png_ptr and info_ptr
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);
		// If we get here, we had a problem writeing the file
		return NULL;
	}

	png_init_io(png_ptr, fp);

	png_color_8 sig_bit;
	sig_bit.red = 8;
	sig_bit.green = 8;
	sig_bit.blue = 8;
	/* if the image has an alpha channel then */
	sig_bit.alpha = 8;
	png_set_sBIT(png_ptr, info_ptr, &sig_bit);
	png_set_bgr(png_ptr);

	png_set_IHDR(png_ptr, info_ptr, theImage->mWidth, theImage->mHeight, 8, PNG_COLOR_TYPE_RGB_ALPHA,
       PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	// Add filler (or alpha) byte (before/after each RGB triplet)
	//png_set_expand(png_ptr);
	//png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
	//png_set_gray_1_2_4_to_8(png_ptr);
	//png_set_palette_to_rgb(png_ptr);
	//png_set_gray_to_rgb(png_ptr);

	png_write_info(png_ptr, info_ptr);

	for (int i = 0; i < theImage->mHeight; i++)
	{
		png_bytep aRowPtr = (png_bytep) (theImage->mBits + i*theImage->mWidth);
		png_write_rows(png_ptr, &aRowPtr, 1);
	}

	// write rest of file, and get additional chunks in info_ptr - REQUIRED
	png_write_end(png_ptr, info_ptr);

	// clean up after the write, and free any memory allocated - REQUIRED
	png_destroy_write_struct(&png_ptr, &info_ptr);

	// close the file
	fclose(fp);

	return true;
}

bool ImageLib::WriteTGAImage(const std::string& theFileName,ImageLib::Image* theImage)
{
	FILE* aTGAFile = fopen(theFileName.c_str(), "wb");
	if (aTGAFile == NULL)
		return false;

	BYTE aHeaderIDLen = 0;
	fwrite(&aHeaderIDLen, sizeof(BYTE), 1, aTGAFile);

	BYTE aColorMapType = 0;
	fwrite(&aColorMapType, sizeof(BYTE), 1, aTGAFile);
	
	BYTE anImageType = 2;
	fwrite(&anImageType, sizeof(BYTE), 1, aTGAFile);

	WORD aFirstEntryIdx = 0;
	fwrite(&aFirstEntryIdx, sizeof(WORD), 1, aTGAFile);

	WORD aColorMapLen = 0;
	fwrite(&aColorMapLen, sizeof(WORD), 1, aTGAFile);

	BYTE aColorMapEntrySize = 0;
	fwrite(&aColorMapEntrySize, sizeof(BYTE), 1, aTGAFile);	

	WORD anXOrigin = 0;
	fwrite(&anXOrigin, sizeof(WORD), 1, aTGAFile);

	WORD aYOrigin = 0;
	fwrite(&aYOrigin, sizeof(WORD), 1, aTGAFile);

	WORD anImageWidth = theImage->mWidth;
	fwrite(&anImageWidth, sizeof(WORD), 1, aTGAFile);	

	WORD anImageHeight = theImage->mHeight;
	fwrite(&anImageHeight, sizeof(WORD), 1, aTGAFile);	

	BYTE aBitCount = 32;
	fwrite(&aBitCount, sizeof(BYTE), 1, aTGAFile);	

	BYTE anImageDescriptor = 8 | (1<<5);
	fwrite(&anImageDescriptor, sizeof(BYTE), 1, aTGAFile);

	fwrite(theImage->mBits, 4, theImage->mWidth*theImage->mHeight, aTGAFile);

	fclose(aTGAFile);

	return true;
}

bool ImageLib::WriteBMPImage(const std::string& theFileName,ImageLib::Image* theImage)
{
	FILE* aFile = fopen(theFileName.c_str(), "wb");
	if (aFile == NULL)
		return false;

	BITMAPFILEHEADER aFileHeader;
	BITMAPINFOHEADER aHeader;

	memset(&aFileHeader,0,sizeof(aFileHeader));
	memset(&aHeader,0,sizeof(aHeader));

	int aNumBytes = theImage->mWidth*theImage->mHeight*4;

	aFileHeader.bfType = ('M'<<8) | 'B';
	aFileHeader.bfSize = sizeof(aFileHeader) + sizeof(aHeader) + aNumBytes;
	aFileHeader.bfOffBits = sizeof(aHeader); 

	aHeader.biSize = sizeof(aHeader);
	aHeader.biWidth = theImage->mWidth;
	aHeader.biHeight = theImage->mHeight;
	aHeader.biPlanes = 1;
	aHeader.biBitCount = 32;
	aHeader.biCompression = BI_RGB;

	fwrite(&aFileHeader,sizeof(aFileHeader),1,aFile);
	fwrite(&aHeader,sizeof(aHeader),1,aFile);
	DWORD *aRow = theImage->mBits + (theImage->mHeight-1)*theImage->mWidth;
	int aRowSize = theImage->mWidth*4;
	for (int i=0; i<theImage->mHeight; i++, aRow-=theImage->mWidth)
		fwrite(aRow,4,theImage->mWidth,aFile);

	fclose(aFile);
	return true;
}

#endif

int ImageLib::gAlphaComposeColor = 0xFFFFFF;
bool ImageLib::gAutoLoadAlpha = true;
bool ImageLib::gIgnoreJPEG2000Alpha = true;

ImageLib::Image* ImageLib::GetImage(std::string theFilename, bool lookForAlphaImage)
{

	if (!gAutoLoadAlpha)
		lookForAlphaImage = false;

        theFilename = Sexy::ReplaceBackSlashes(theFilename);

	int aLastDotPos = theFilename.rfind('.');
	int aLastSlashPos = (int)theFilename.rfind('/');

	std::string anExt;
	std::string aFilename;

	if (aLastDotPos > aLastSlashPos)
	{
		anExt = theFilename.substr(aLastDotPos, theFilename.length() - aLastDotPos);
		aFilename = theFilename.substr(0, aLastDotPos);
	}
	else {
          aFilename = theFilename;
        
        }

        ImageLib::Image* anImage = NULL;

        Magick::Image mImage;
        bool ok = false;

        if (anExt.length() == 0) 
          {
            std::list<std::string> coderList;

            coderList.push_back("jpg");
            coderList.push_back("png");
            coderList.push_back("gif");
            coderList.push_back("jp2"); 
            coderList.push_back("tga");
            coderList.push_back("tif");
            coderList.push_back("bmp");

            std::list<std::string>::const_iterator entry = coderList.begin();
            while( entry != coderList.end() ) 
              {
                    try {
                        if (Sexy::gSexyAppBase->FileExists(theFilename + "." + *entry))  {
                          // Read a file into image object
                          mImage.read( theFilename + "." + *entry);
                          ok  = true;
                          break;
                        }
                        else if (Sexy::gSexyAppBase->FileExists(theFilename + "." + Sexy::Upper(*entry))) {
                          // Read a file into image object
                          mImage.read( theFilename + "." + Sexy::Upper(*entry));
                          ok  = true;
                          break;
                        }
                    }
                    catch( Magick::Exception &error_ )
                      {
                        return NULL;
                      }
                    catch (std::exception &error) {
                      return NULL;

                    }
                    catch ( ...) {
                      return NULL;
                    }
                ++entry;
              }
          }
        else
          {

            try {
              // Read a file into image object
              if (Sexy::gSexyAppBase->FileExists(theFilename)) {
                mImage.read( theFilename );
                ok = true;
              }
              else if (Sexy::gSexyAppBase->FileExists(aFilename + Sexy::Lower(anExt))) {
                mImage.read( aFilename + Sexy::Lower(anExt));
                ok = true;
              }
              else if (Sexy::gSexyAppBase->FileExists(aFilename + Sexy::Upper(anExt))) {
                mImage.read( aFilename + Sexy::Upper(anExt) );
                ok = true;
              }
            }
            catch( Magick::Exception &error_ )
              {
                return NULL;
              }
            catch (std::exception &error) {
              return NULL;
            }
            catch ( ...) {
              return NULL;
            }
          }
       
	// Check for alpha images
	Image* anAlphaImage = NULL;
	if(lookForAlphaImage)
	{
          // Check _ImageName
          anAlphaImage = GetImage(aFilename + "_", false);

          if(anAlphaImage==NULL) {
            anAlphaImage = GetImage(aFilename.substr(0, aLastSlashPos+1) + "_" +
                                    aFilename.substr(aLastSlashPos+1, aFilename.length() - aLastSlashPos - 1), false);
          }
	}

	// Compose alpha channel with image
	if (anAlphaImage != NULL) 
	{

          if (ok && anImage == NULL) {
            //TODO put this in a function
            anImage = new ImageLib::Image(mImage.baseColumns(), mImage.baseRows());

            const Magick::PixelPacket* pixels = mImage.getConstPixels(0,0,mImage.baseColumns(), mImage.baseRows());

            for(int i = 0; i < mImage.baseColumns() * mImage.baseRows(); ++i) {
              const Magick::PixelPacket* p = pixels + i;
              Magick::Color c(*p);
              Magick::ColorRGB rgb = c;

              *((unsigned char*)anImage->mBits + i * sizeof(ulong) + 2) = (unsigned char)(rgb.red() * 255.0f);          
              *((unsigned char*)anImage->mBits + i * sizeof(ulong) + 1) = (unsigned char)(rgb.green() * 255.0f);          
              *((unsigned char*)anImage->mBits + i * sizeof(ulong) + 0) = (unsigned char)(rgb.blue() * 255.0f);          
              *((unsigned char*)anImage->mBits + i * sizeof(ulong) + 3) = 255 - (unsigned char)(c.alpha() * 255.0f);
            }
          }

          if (anImage != NULL)
            {
              if ((anImage->mWidth == anAlphaImage->mWidth) &&
                  (anImage->mHeight == anAlphaImage->mHeight))
                {
                  unsigned long* aBits1 = anImage->mBits;
                  unsigned long* aBits2 = anAlphaImage->mBits;
                  int aSize = anImage->mWidth*anImage->mHeight;

                  for (int i = 0; i < aSize; i++)
                    {
                      *aBits1 = (*aBits1 & 0x00FFFFFF) | ((*aBits2 & 0xFF) << 24);
                      ++aBits1;
                      ++aBits2;
                    }
                }

              delete anAlphaImage;
            }
          else if (gAlphaComposeColor==0xFFFFFF)
            {
              anImage = anAlphaImage;

              unsigned long* aBits1 = anImage->mBits;

              int aSize = anImage->mWidth*anImage->mHeight;
              for (int i = 0; i < aSize; i++)
                {
                  *aBits1 = (0x00FFFFFF) | ((*aBits1 & 0xFF) << 24);
                  ++aBits1;
                }
            }
          else
            {
              const int aColor = gAlphaComposeColor;
              anImage = anAlphaImage;

              unsigned long* aBits1 = anImage->mBits;

              int aSize = anImage->mWidth*anImage->mHeight;
              for (int i = 0; i < aSize; i++)
                {
                  *aBits1 = aColor | ((*aBits1 & 0xFF) << 24);
                  ++aBits1;
                }
            }
	}

        if (anImage == NULL && ok) {
          anImage = new ImageLib::Image(mImage.baseColumns(), mImage.baseRows());

          const Magick::PixelPacket* pixels = mImage.getConstPixels(0,0,mImage.baseColumns(), mImage.baseRows());

          for(int i = 0; i < mImage.baseColumns() * mImage.baseRows(); ++i) {
            const Magick::PixelPacket* p = pixels + i;
            Magick::Color c(*p);
            Magick::ColorRGB rgb = c;

            *((unsigned char*)anImage->mBits + i * sizeof(ulong) + 2) = (unsigned char)(rgb.red() * 255.0f);          
            *((unsigned char*)anImage->mBits + i * sizeof(ulong) + 1) = (unsigned char)(rgb.green() * 255.0f);          
            *((unsigned char*)anImage->mBits + i * sizeof(ulong) + 0)= (unsigned char)(rgb.blue() * 255.0f);          
            *((unsigned char*)anImage->mBits + i * sizeof(ulong) + 3) = 255 - (unsigned char)(c.alpha() * 255.0f);
          }
        }

	return anImage;
}

