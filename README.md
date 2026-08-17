
# Digital Image Steganography

## Description
Digital Image Steganography is a project used to hide secret information inside a digital image without noticeably changing the image.

The project uses image processing techniques to embed and extract hidden data from an image.

## Features
- Hide secret text inside an image
- Extract hidden text from the image
- Preserve the visual quality of the image
- Simple command-line based implementation

## Technologies Used
- C
- Image Processing
- File Handling
- Bitwise Operations

## How It Works
1. Select an image and secret message.
2. Encode the secret message into the image.
3. Generate a stego-image containing the hidden message.
4. Decode the stego-image to retrieve the original message.

## Project Structure
```text
Digital-Image-Steganography/
├── encode.c
├── decode.c
├── main.c
├── header.h
└── README.md
