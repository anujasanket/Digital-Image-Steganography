#include <stdio.h>
#include "encode.h"
#include "common.h"
#include<string.h>
#include "types.h"
#include "decode.h"


Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    
    // Check for correct command format
    if (argv[1] == NULL || strcmp(argv[1], "-d") != 0 || argv[2] == NULL)
    {
        printf("Usage: %s -d <stego_image.bmp> [output_file]\n", argv[0]);
        return e_failure;
    }

    // Validate stego image file (must be .bmp)
    if (strstr(argv[2], ".bmp") == NULL)
    {
        printf("Error: Stego image must be a .bmp file\n");
        return e_failure;
    }

    // Store stego image file name
    decInfo->stego_image_fname = argv[2];
    printf("%s\n",decInfo->stego_image_fname);
    // check output file pass or not
    if (argv[3] != NULL)
    {
        strcpy(decInfo->output_fname, argv[3]);//if passed strore in structure member
    }
    else
    {
        strcpy(decInfo->output_fname, "output"); // default base name; extension added later
    }

    return e_success;
}
Status open_src_file(DecodeInfo *decInfo)
{
    
    // Open the source (stego) image file in read mode
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "r");
    printf("%s\n",decInfo->stego_image_fname);

    if (decInfo->fptr_stego_image == NULL)
    {
        fprintf(stderr, "Error: Unable to open stego image file: %s\n", decInfo->stego_image_fname);
        return e_failure;
    }
    else{
        printf("Open files were successfull\n");
    }

    return e_success;
}



Status decode_byte_from_lsb(char *data, char *image_buffer)
{
    *data=0;
    for(int i=0;i<8;i++)
    {
       int bit=image_buffer[i]&1;  //get lsb bit
       *data = *data | (bit<<(7-i)); //set on proper position

    }
    return e_success;
}
Status decode_int_from_lsb(int*data,char*image_buffer)

{
    *data=0;
    for(int i=0;i<32 ;i++)
    {
       int bit=image_buffer[i]&1;  //get lsb bit
       if(bit==1)
       *data= *data | (1<<(31 - i)); //set on proper position

    }

    printf("from decode_int -->%d\n",*data);
    return e_success;
}
Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo)
{
    int i;
    //read 8 byte from image
    char dec_char[3];
    fseek(decInfo->fptr_stego_image,54,SEEK_SET);

    char data[8]={0};
    
    for(int i=0;i<2;i++)
    {
        printf("reading %d times",i+1);
        fread(data,1,8,decInfo->fptr_stego_image);
        decode_byte_from_lsb(&dec_char[i],data);
        printf("%c\n",dec_char[i]);
    }

    dec_char[2]='\0';

    if(strcmp(dec_char,magic_string))
    {
        fprintf(stderr,"Error-> magic strig not matched\n");
        return e_failure;
    }
    return e_success;
}


 Status decode_extn_size(DecodeInfo *decInfo)
 {
     int extn_size = 0;

    // Read 4 bytes (i.e., 32 bits)
    printf("offset position=%lu\n",ftell(decInfo->fptr_stego_image));
    char buf[32]; 
    fread(buf,1,32,decInfo->fptr_stego_image);
    decode_int_from_lsb(&extn_size,buf);

    printf("Extention size=%d\n",extn_size);
    decInfo -> extn_size = extn_size;
    return e_success;
}

Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    
    char ch, i;

    // Decode each character of the extension string
    for (i = 0; i<decInfo -> extn_size; i++)
    {
        // Read 8 bytes from stego image to decode one byte
        if (fread(decInfo->image_data, 1, 8, decInfo->fptr_stego_image) != 8)
        {
            fprintf(stderr, "Error: Unable to read image data for extension char %d\n", i);
            return e_failure;
        }

        // Decode one byte from 8 bytes of image data
        decode_byte_from_lsb(&ch, decInfo->image_data);

        // Store decoded char into extension buffer
       decInfo->extn_secret_file[i] = ch;
        printf("%c\n",ch);

    }
    decInfo -> extn_secret_file[i] = '\0';

    // Null-terminate the extension string
    
    return e_success;

}
//open output file in write mode
// Open the file in write mode
Status open_output_file(DecodeInfo *decInfo)
 {
    

    // add entention and then open file
    
    strcat(decInfo->output_fname, decInfo -> extn_secret_file);
    printf("file name: %s\n", decInfo->output_fname);
    decInfo->output_fptr = fopen(decInfo->output_fname, "w");
    if (decInfo->output_fptr == NULL)
    {
        fprintf(stderr, "Error: Unable to open file %s for writing\n", decInfo->output_fname);
        return e_failure;
    }
    else
    {

    }

    return e_success;
}
Status decode_secret_file_size(long int file_size, DecodeInfo *decInfo)


{
//int extn_size = 0;
int image_size=0;

    // Read 4 bytes (i.e., 32 bits)
    printf("offset position=%lu\n",ftell(decInfo->fptr_stego_image));
    char buf[32]; 
    fread(buf,1,32,decInfo->fptr_stego_image);
    decode_int_from_lsb(&image_size,buf);//extn_size

    printf("image_file size=%d\n",image_size);
    decInfo -> image_datasize = image_size;
    return e_success;
}


Status decode_secret_file_data(DecodeInfo *decInfo)
{
    
    char data;
    int decInfo_size;
    
    // Loop through each byte of secret data
    for (long i = 0; i < decInfo->image_datasize; i++)
    {
        // Read 8 bytes from stego image and decode 1 byte of secret data
        if (fread(decInfo->image_data, 1, 8, decInfo->fptr_stego_image) != 8)
        {
            fprintf(stderr, "Error: Unable to read stego image data\n");
            return e_failure;
        }

        decode_byte_from_lsb(&data, decInfo->image_data);

        // Write the decoded byte to the output file
        if (fputc(data, decInfo->output_fptr) == EOF)
        {
            fprintf(stderr, "Error: Unable to write to output file\n");
            return e_failure;
        }
    }

    return e_success;
}

Status decode_data_from_image(char *data, int size, FILE *fptr_src_image)

{
    
    char image_buffer[8];

    for (int i = 0; i < size; i++)
    {
        // Read 8 bytes from the stego image
        if (fread(image_buffer, 1, 8, fptr_src_image) != 8)
        {
            fprintf(stderr, "Error: Unable to read from stego image while decoding data\n");
            return e_failure;
        }

        // Decode a single byte from the 8 LSBs
        data[i] = 0;
        for (int j = 0; j < 8; j++)
        {
            data[i] = (data[i] << 1) | (image_buffer[j] & 1);
        }
    }

    return e_success;
}
 Status do_decoding(DecodeInfo *decInfo)
{
    // decInfo->image_capacity = get_image_size_for_bmp(decInfo->fptr_src_image);
    // decInfo->extn_secret_file = get_file_size(decInfo->fptr_secret);
    
    if(decode_magic_string(MAGIC_STRING,decInfo)==e_failure)
    {
        printf("decode magic string is failure\n");
        return e_failure;
    }
    else{
        printf("Magic string is decoded successfull\n");
    }
    
    
        if(decode_extn_size(decInfo) == e_failure)
    {
        return e_failure;
    }

    if(decode_secret_file_extn(decInfo)== e_failure)
    {
        return e_failure;
        
    }
    if(decode_secret_file_size(decInfo->size,decInfo)== e_failure)
    {
        return e_failure;
    }

    if(open_output_file(decInfo)== e_failure)
    {
        return e_failure;
    }

    if (decode_secret_file_data(decInfo) == e_failure)
    {
        return e_failure;
    }
    
    
    
    return e_success;


}



