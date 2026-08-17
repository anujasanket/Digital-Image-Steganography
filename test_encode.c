/*Name of projet: Image Steganography
Name: Anuja
Date of submission: 11-6-2025
Discription: Hide message secretly in the image */ 
#include<stdio.h>
#include "encode.h"
#include "decode.h"
#include "types.h"
#include<string.h>
//#include "decode.h"
int main(int argc, char*argv[])
{
    uint img_size;
    EncodeInfo encInfo;
    DecodeInfo decInfo;
    // check for enough arguments are present or not
    if(argc<3)
    {
        printf("Not Enough arguments present command for encoding ->./a.out -e beautiful.bmp secret.txt stego_img.bmp and for decding ./a.out -d stego_img.bmp (enter any file neme...eg-output)\n");
        return e_failure;
    }
    // if user select encoding part

    if(check_operation_type(argv) == e_encode)
    {
        printf("----------------------------------------------\n");
        printf(" You have selected Encoding!\n");
        
        if(read_and_validate_encode_args(argv,&encInfo) == e_success)
        {
            printf("Read and validate encode arguments successfull!\n");
    
            if (open_files(&encInfo) == e_success)
            {
                printf("Done\n");
                if(do_encoding(&encInfo) == e_success)
                {
                    printf(" Encoding Done Successfully \n");
                    printf("------------------------------------\n");
                }
            }
        }
        else
        {
            printf("Unsucessfull\n");
        }
    }
    //if user select Decoading part

            
    if(check_operation_type(argv) == e_decode)
    {
        DecodeInfo decInfo;
        printf("You have selected Decoading\n");
        if(read_and_validate_decode_args(argv,&decInfo) == e_success)
        {
            
            printf("Read and validate decode arguments successfull");
            if(open_src_file(&decInfo)==e_success)
            {
                printf("decoding successfull\n");
                do_decoding(&decInfo);
                return 0;

            }
            
        }
        
    }
    else
    {
        // printf("unsuccessfull\n");
        // return 0;
    }
}
   
    
   


