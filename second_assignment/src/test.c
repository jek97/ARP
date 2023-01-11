#include "./../include/processA_utilities.h"
#include <bmpfile.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <errno.h>
#include <semaphore.h>
#include <time.h>
#include <string.h>

int main() {
    // declare some internal variable:
    int xc = 100; // declare the x coordinate of the center of the circle in the ncurse window
    int yc = 100; // declare the y coordinate of the center of the circle in the ncurse window
    int r = 30; // initialize the radius of the circle in the shared memory
    rgb_pixel_t c_color = {0, 0, 255, 255}; // initialize the circle color (BRGA) to red
    const char * shm_snapshot = "./out/snapshot.bmp";
    int arr[600]={0};
    arr[300] = 1;

    bmpfile_t *image; // Data structure for storing the bitmap file
    int width = 1600; // width of the image (in pixels)
    int height = 600; // Height of the image (in pixels)
    int depth = 4; // Depth of the image (1 for greyscale images, 4 for colored images)
    image = bmp_create(width, height, depth); // create the image

    int i_max = bmp_get_width(image); // number of row of the picture
    int j_max = bmp_get_height(image); // number of column of the picture

    rgb_pixel_t color = {255, 0, 0, 255}; // define the color white
    rgb_pixel_t color1 = {0, 255, 0, 255}; // define the color white
    for (int i = 1; i <= i_max; i++) { // for all the image points
        for (int j = 1; j <= j_max; j++) {
            bmp_set_pixel(image, i, j, color); // color the pixels of white color
        }
    }
    for (int i = 1; i <= i_max; i++) { // for all the image points
        for (int j = 1; j <= j_max; j++) {
            if (arr[j] == 1) {
                bmp_set_pixel(image, i, j, color1); // color the pixels of white color
            }
            
        }
    }
    
    

    
    bmp_save(image, shm_snapshot);
}