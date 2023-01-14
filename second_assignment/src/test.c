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
#include <signal.h>
#include <sys/mman.h>
void main() {
    rgb_pixel_t c_color = {0, 0, 255, 255}; // initialize the circle color (BRGA) to red
    int width = 1600; // width of the image (in pixels)
    int height = 600; // Height of the image (in pixels)
    int depth = 4; // Depth of the image (1 for greyscale images, 4 for colored images)
    bmpfile_t * image = bmp_create(width, height, depth); // create the image
    int arr[width * height];
    int shm_size = sizeof(arr); // set the shared memory dimension
    printf("%i", shm_size);
}