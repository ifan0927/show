// Copies a BMP file

#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"

int main(int argc, char *argv[])
{
    // ensure proper usage
    if (argc != 4)
    {
        fprintf(stderr, "Usage: copy infile outfile\n");
        return 1;
    }

    // remember filenames
    char *infile = argv[2];
    char *outfile = argv[3];

    // open input file
    FILE *inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", infile);
        return 2;
    }

    // open output file
    FILE *outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        fprintf(stderr, "Could not create %s.\n", outfile);
        return 3;
    }

    //confirm user input and check input is ok
    float muti = atof(argv[1]);
    if (muti < 0 || muti > 100)
    {
        fprintf(stderr,"Usage: ./resize f infile outfile\n");
        return 1;
    }

    // read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf;
    BITMAPFILEHEADER bf_new;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi;
    BITMAPINFOHEADER bi_new;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 ||
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(outptr);
        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return 4;
    }

    bf_new = bf;
    bi_new = bi;
    int factor = 1 / muti;


    //resize
    if (muti >= 1)
    {
        bi_new.biWidth = bi.biWidth * muti;
        bi_new.biHeight = bi.biHeight * muti;
    }
    if (muti < 1)
    {
        bi_new.biWidth = bi.biWidth / factor;
        bi_new.biHeight = bi.biHeight / factor;
    }

    // determine padding for scanlines
    int padding = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    int padding_new = (4 - (bi_new.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

    bi_new.biSizeImage = (bi_new.biWidth * sizeof(RGBTRIPLE) + padding_new) * abs(bi_new.biHeight);
    bf_new.bfSize = bi_new.biSizeImage + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    // write outfile's BITMAPFILEHEADER
    fwrite(&bf_new, sizeof(BITMAPFILEHEADER), 1, outptr);
    // write outfile's BITMAPINFOHEADER
    fwrite(&bi_new, sizeof(BITMAPINFOHEADER), 1, outptr);

    // iterate over infile's scanlines while muti >= 1
    if (muti >= 1)
    {
        for (int i = 0, biHeight = abs(bi.biHeight); i < biHeight; i++)
        {
            //set counter
            int counter = 0;
            while (counter < muti)
            {

                //iterate over pixels in scanline
                for (int j = 0; j < bi.biWidth ; j++)
                {

                    //temporary storage
                    RGBTRIPLE triple;

                    //read RGB triple from infile
                    fread(&triple , sizeof(RGBTRIPLE) , 1 , inptr);

                    //write RGB triple to outfile
                    int pixelcounter = 0 ;
                    while (pixelcounter < muti)
                    {

                        fwrite(&triple , sizeof(RGBTRIPLE) , 1 , outptr);
                        pixelcounter ++ ;
                    }
                }
                for (int k = 0 ; k < padding_new ; k++)
                {
                    fputc(0x00 , outptr);
                }
                if (counter < muti - 1 )
                {

                    fseek(inptr , -(sizeof(RGBTRIPLE) * bi.biWidth)  , SEEK_CUR);
                }
                counter ++ ;

            }
            if (counter == muti)
            {
                fseek(inptr , padding , SEEK_CUR);
            }
        }
    }

    // iterate over infile's scanlines while muti < 1 (small)
    int ptr_counter = 0 ;
    if (muti < 1)
    {
        for (int i = 0, biHeight = abs(bi_new.biHeight) ; i < biHeight ; i++)
        {
            for(int j = 0 ; j < bi_new.biWidth ; j++)
            {
                //set ptrcounter to 0
                ptr_counter = 0 ;

                //temporary memory
                RGBTRIPLE triple;

                //read RGB triple from infile
                fread(&triple , sizeof(RGBTRIPLE) , 1 , inptr);

                //write RGB triple to outfile
                fwrite(&triple , sizeof(RGBTRIPLE) , 1 , outptr);

                //move pointer
                while (ptr_counter < (factor - 1))
                {
                    fseek(inptr , sizeof(RGBTRIPLE) , SEEK_CUR);
                    ptr_counter ++ ;
                }
            }

            //add padding
            for (int k = 0 ; k < padding_new ; k++)
            {
                fputc(0x00 , outptr);
            }

            fseek(inptr , padding , SEEK_CUR);
            fseek(inptr , (sizeof(RGBTRIPLE) * sizeof(bi.biWidth)) + padding , SEEK_CUR);

        }


    }



    // close infile
    fclose(inptr);

    // close outfile
    fclose(outptr);

    // success
    return 0;
}

