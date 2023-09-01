#include <iostream>
#include <stdio.h>
#include <png.h>

using namespace std;



int write_to_file(png_bytep *rows, int height, int width, int bit_depth, png_structp png, png_infop info)
{
    FILE* output = fopen("OUTPUT.PNG", "w");
    if (!output) {
        cerr << "Error opening output file: OUTPUT.PNG" << endl;
        return 6;
    }
    if (!png)
    {
        cerr << "Error creating read struct" << endl;
        return 7;
    }
    png_init_io(png, output);

    // Set up IHDR, write info, and write image data
    png_set_IHDR(png, info, width, height, bit_depth, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);


    
    cout << "Writing info to OUTPUT.PNG" << endl;
    png_write_info(png, info); //this line errors

    cout << "Writing image to OUTPUT.PNG" << endl;
    png_write_image(png, rows);

    cout << "Writing EOF to OUTPUT.PNG" << endl;
    png_write_end(png, NULL);

    fclose(output);

    return 0;
}

double f(double x)
{
    return pow(x, .25);
}

void set_transparent_gradient(png_bytep *rows, int height, int width, bool left_to_right, int starting_alpha, int ending_alpha)
{
    int s = 0;
    int e = width - 1;
    //switch 
    if(!left_to_right)
    {
        s += e;
        e = s - e;
        s -= e;
    }

    for (int y = 0; y < height; ++y) 
    {   
        int i = 0;
        for (int x = s; x < width && x >= 0; ) 
        {
            png_bytep pixel = &(rows[y][x*4]); // RGBA format
            int a = (int)(2.55*(double)starting_alpha-(2.55 * f((double)i/ (double)(width)) * (double)(starting_alpha-ending_alpha) ));
            png_byte alpha = static_cast<png_byte>(a);
            pixel[3] = alpha; // Set the alpha value
            if (left_to_right)
                ++x;
            else if (!left_to_right)
                --x;
            i++;
        }
    }
}


int main(int argc, char **argv) 
{
    if (argc != 3) {
        cout << "Usage: " << argv[0] << " <filename.png> <left || right>" << endl;
        return 1; // Return an appropriate error code
    }
    if (strcmp("left", argv[2])!=0 && strcmp("right", argv[2])!=0)
    {
        cout << "Usage: " << argv[0] << " <filename.png> <left || right>" << endl;
        return 8;
    }
    bool side = strcmp("left", argv[2]) ? true : false;


    printf("Opening %s...\n", argv[1]);
    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
        cerr << "Error opening file: " << argv[1] << endl;
        return 2; // Return an appropriate error code
    }
    
    // Read the PNG signature (8 bytes) from the file
    png_byte header[8];
    fread(header, 1, 8, fp);

    
    // Check if the file is a PNG by comparing its signature with PNG signature
    int is_png = !png_sig_cmp(header, 0, 8);
    if (!is_png) {
        cerr << "File is not a PNG image: " << argv[1] << endl;
        fclose(fp);
        return 3; // Return an appropriate error code
    }

    cout << "File is a valid PNG image: " << argv[1] << endl;
    
    cout << "Reading file data..." << endl;

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
        png_destroy_read_struct(&png, NULL, NULL); 
        cerr << "Error creating read struct" << endl;
        return 4; // Return an appropriate error code
    }


    png_infop info = png_create_info_struct(png);
    if (!info) {
        cerr << "Error creating info struct" << endl;
        png_destroy_read_struct(&png, NULL, NULL);
        return 5; //Return an appropriate error code
    }    

    png_init_io(png, fp);
    rewind(fp);
    png_read_info(png, info);

    cout << "Getting image properties..." << endl;
    int height = png_get_image_height(png, info);
    int width = png_get_image_width(png, info);
    png_byte bit_depth = png_get_bit_depth(png, info);

    cout << "Converting image to pixel array..." << endl;
    png_bytep *rows = new png_bytep[height];
    for (int y = 0; y < height; y++) {
        rows[y] = new png_byte[png_get_rowbytes(png, info)];
    }
    png_read_image(png, rows);

    set_transparent_gradient(rows, height, width, side, 0, 100);

    fclose(fp);
    png_destroy_read_struct(&png, &info, NULL); 

    
    png_structp png_write = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_write) {
        cerr << "Error creating write struct" << endl;
        return 4; // Return an appropriate error code
    }

    png_infop info_write = png_create_info_struct(png_write);
    if (!info_write) {
        cerr << "Error creating write info struct" << endl;
        png_destroy_write_struct(&png_write, NULL);
        return 5; // Return an appropriate error code
    }    

    int status = write_to_file(rows, height, width, bit_depth, png_write, info_write);
    if (!status)
    {
        return status;
    }

    //garbage collection
    for (int y = 0; y < height; y++) {
        delete[] rows[y];
    }
    delete[] rows;
    return 0;
}
