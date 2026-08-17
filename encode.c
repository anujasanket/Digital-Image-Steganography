#include <stdio.h>
#include "encode.h"
#include "common.h"
#include <string.h>
#include "types.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */

 // check in which mode user want to open file -e for encoading and -d for decoding
OperationType check_operation_type(char *argv[])
{
    if (strcmp(argv[1], "-e") == 0)
    {
        return e_encode;
    }
    else if (strcmp(argv[1], "-d") == 0)
    {
        return e_decode;
    }
    else
        return e_unsupported;
}
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{

    if (argv[1] == NULL)
    {
        printf(" first argument should be '-e'\n");
        return e_failure;
    }
    else if (argv[2] == NULL)
    {
        printf("Source file is missing.\n");
        return e_failure;
    }
    else if (argv[3] == NULL)
    {
        printf("Secret file is missing.\n");
        return e_failure;
    }
    else if (strstr(argv[2], ".bmp") == NULL)
    {
        printf("Source file should be in .BMP format\n"); // This function checks whether beautiful.bmp contains the .bmp extension or not.
        return e_failure;
    }
    else if (strstr(argv[3], ".txt") == NULL)
    {
        printf("Secret file should be in .txt format\n");
        return e_failure;
    }
    else
    {
        encInfo->src_image_fname = argv[2];
        encInfo->secret_fname = argv[3];
        if (argv[4] == NULL)
        {
            printf("Output file not mentioned. Created stego.bmp as default\n");
            encInfo->stego_image_fname = "stego.bmp"; // default name
        }
        else
        {
            encInfo->stego_image_fname = argv[4];
        }
        printf("stego: %s src: %s secret: %s", encInfo->stego_image_fname, encInfo->src_image_fname, encInfo->secret_fname);
        return e_success;
    }
}

Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

        return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

        return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

        return e_failure;
    }

    // No failure return e_success
    return e_success;
}
//check image capacity

Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);
    int capacity_required = (strlen(MAGIC_STRING) * 8) + (strlen(".txt") * 8) + 32 + (encInfo->size_secret_file * 8);
    if (capacity_required > encInfo->image_capacity)
    {
        printf("Src Image doesn't have enough capacity\n");
        return e_failure;
    }
    return e_success;
}
uint get_file_size(FILE *fptr)
{
    fseek(fptr, 0, SEEK_END);
    long size = ftell(fptr);
    rewind(fptr);
    return (uint)size;
}
// copy 54 bytes of header as it is
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    printf("%lu\n", ftell(fptr_src_image));
    rewind(fptr_src_image);
    printf("%lu\n", ftell(fptr_src_image));

    printf("%lu\n", ftell(fptr_dest_image));
    char header[54];
    printf("hello\n");
    if (fread(header, 1, 54, fptr_src_image) != 54)
    {
        return e_failure; 
    }
    if (fwrite(header, 1, 54, fptr_dest_image) != 54)
    {
        return e_failure; 
    }
    return e_success; 
}
//Encode magic string
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    return encode_data_to_image((char *)magic_string, strlen(magic_string), encInfo->fptr_src_image, encInfo->fptr_stego_image);
}
// Encoded last bit of datat
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for (int i = 7; i >= 0; i--)
    {
        int bit = (data >> i) & 1;
        *image_buffer = (*image_buffer & ~1) | bit;
        image_buffer++; // move to next image pointer
    }
    return e_success;
}
//copy remainning data to stego file
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    int ch;
    while ((ch = fgetc(fptr_src)) != EOF)
    {
        if (fputc(ch, fptr_dest) == EOF)
        {
            fprintf(stderr, "Error writing remaining image data\n");
            return e_failure;
        }
    }
    return e_success;
}

uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */

 //Encoading  secret file size 
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char buffer[32];
    int bits[32];
    int k = 0;

    for (int i = 31; i >= 0; i--)
    {
        bits[k++] = (file_size >> i) & 1;
    }

    if (fread(buffer, 1, 32, encInfo->fptr_src_image) != 32)
    {
        fprintf(stderr, "Error: while reading from source image\n");
        return e_failure;
    }

    for (int i = 0; i < 32; i++)
    {
        buffer[i] = (buffer[i] & ~1) | bits[i];
    }

    if (fwrite(buffer, 1, 32, encInfo->fptr_stego_image) != 32)
    {
        fprintf(stderr, "Error: while writing to stego image\n");
        return e_failure;
    }

    return e_success;
}
// Encoading Extention size
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    int exten_size = strlen(file_extn);
    printf("extn_size = %d\n", exten_size);
    char buffer[32];
    int bits[32];
    int k = 0;
    for (int i = 31; i >= 0; i--)
    {
        bits[k++] = (exten_size >> i) & 1;
    }
    if (fread(buffer, 1, 32, encInfo->fptr_src_image) != 32)
    {
        fprintf(stderr, "Error: while reading to a file\n");
        return e_failure;
    }
    for (int i = 0; i < 32; i++)
    {
        buffer[i] = (buffer[i] & ~1) | bits[i];
    }
    if (fwrite(buffer, 1, 32, encInfo->fptr_stego_image) != 32)
    {
        fprintf(stderr, "Error: while reading to a file\n");
        return e_failure;
    }
    // Encode extn size
    for (int i = 0; i < exten_size; i++)
    {
        char ch = file_extn[i];
        for (int bit = 7; bit >= 0; bit--)
        {
            int bit_val = (ch >> bit) & 1;

            // Read 1 byte from source image
            if (fread(buffer, 1, 1, encInfo->fptr_src_image) != 1)
            {
                fprintf(stderr, "Error: while reading source image\n");
                return e_failure;
            }
            buffer[0] = (buffer[0] & ~1) | bit_val;
            if (fwrite(buffer, 1, 1, encInfo->fptr_stego_image) != 1)
            {
                fprintf(stderr, "Error: while writing stego image\n");
                return e_failure;
            }
        }
    }
    return e_success;
}
//Read data from source file and write to stego image

Status encode_data_to_image(char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
    char buffer[8];
    

    for (int i = 0; i < size; i++)
    {
        unsigned char ch = data[i];
        // printf("%c\n",ch);
        if (fread(buffer, 1, 8, fptr_src_image) != 8)
        {
            fprintf(stderr, "Error: Unable to read from source image\n");
            return e_failure;
        }
        for (int j = 0; j < 8; j++)
        {
            buffer[j] = (buffer[j] & ~1) | ((ch >> (7 - j)) & 1);
        }
        if (fwrite(buffer, 1, 8, fptr_stego_image) != 8)
        {
            fprintf(stderr, "Error: Unable to write to stego image\n");
            return e_failure;
        }
    }

    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char ch;
    while (fread(&ch, 1, 1, encInfo->fptr_secret) == 1)
    {
        if (encode_data_to_image(&ch, 1, encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
        {
            fprintf(stderr, "Error: Unable to encode secret file data\n");
            return e_failure;
        }
    }
    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
    
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);
    if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
    {
        printf("Copy bmp header is successful\n");
    }
    else
    {
        return e_failure;
    }
    if (encode_magic_string(MAGIC_STRING, encInfo) == e_success)
    {
        printf("Magic string copied successully\n");
    }
    else
    {
        return e_failure;
    }

    if (encode_secret_file_extn(".txt", encInfo) == e_failure)
    {
        return e_failure;
    }
    else
    {
        printf("Encoidng secret fill extention was a success\n");
    }
    if (encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_failure)
    {
        return e_failure;
    }
    else
    {
        printf("Encode secret file size is a success\n");
    }
    if (encode_secret_file_data(encInfo) == e_failure)
    {
        return e_failure;
    }
    
    else
    {
        printf("Encode secrest file data successfully\n");
    }
    
    
    if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
    {
        return e_failure;
    }
    else
    {
        printf("Remaining datas were copied successful\n");
    }
    return e_success;
}
