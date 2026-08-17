#ifndef DECODE_H
#define DECODE_H

#include "types.h" // Contains user defined types

/* 
 * Structure to store information required for
 * encoding secret file to source Image
 * Info about output and intermediate data is
 * also stored
 */

#define MAX_SECRET_BUF_SIZE 1
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)
#define MAX_FILE_SUFFIX 4

typedef struct _DecodeInfo
{
    /* Source Image info */
    char *stego_image_fname;      //To store the argv[2]
    FILE *fptr_stego_image;      //To store the file pointer of argv[2]
    char image_data[MAX_IMAGE_BUF_SIZE];        //To store the data

    int extn_size;
    uint image_datasize;
    char extn[20];
    char extn_secret_file[MAX_FILE_SUFFIX];
    char secret_data[MAX_SECRET_BUF_SIZE];
    long size;
    int image_size;

    /* Output Image Info */
    char output_fname[20];       //To store the output file name  
    FILE *output_fptr;          //To store the file pointer of output file

} DecodeInfo;



/* Decoding function prototype */

/* Read and validate Encode args from argv */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Perform the encoding */
Status do_decoding(DecodeInfo *decInfo);

/* Get File pointers for i/p and o/p files */
Status open_src_file(DecodeInfo *decInfo);

/* Store Magic String */
Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo);

/*Extn size*/
Status decode_extn_size(DecodeInfo *decInfo);

/* Encode secret file extenstion */
Status decode_secret_file_extn(DecodeInfo *decInfo);

/*Opening the output file*/
Status open_output_file( DecodeInfo *decInfo);

/* Encode secret file size */
Status decode_secret_file_size(long file_size, DecodeInfo *decInfo);

/* Encode secret file data*/
Status decode_secret_file_data(DecodeInfo *decInfo);

/* Encode function, which does the real encoding */
Status decode_data_from_image(char *data, int size, FILE *fptr_stego_image);

/* Encode a byte into LSB of image data array */
Status decode_byte_from_lsb(char *data, char *image_buffer);
Status decode_int_from_lsb(int *data, char *image_buffer);

#endif