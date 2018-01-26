/*
To test that the Linux framebuffer is set up correctly, and that the device permissions
are correct, use the program below which opens the frame buffer and draws a gradient-
filled red square:

retrieved from:
Testing the Linux Framebuffer for Qtopia Core (qt4-x11-4.2.2)

http://cep.xor.aps.anl.gov/software/qt4-x11-4.2.2/qtopiacore-testingframebuffer.html
*/

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
char *fbp = 0;
int fbfd = 0;
long int screensize = 0;
long int location = 0;

void init();
void printpixel(int x, int y);
void bres_line(int x1, int y1, int x2, int y2);

int main()
{
    // Open the file for reading and writing
    init();   
    bres_line(100,100,500,700);
    munmap(fbp, screensize);
    close(fbfd);
    return 0;
}

void init(){
    fbfd = open("/dev/fb0", O_RDWR);
    if (fbfd == -1) {
        perror("Error: cannot open framebuffer device");
        exit(1);
    }
    printf("The framebuffer device was opened successfully.\n");

    // Get fixed screen information
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1) {
        perror("Error reading fixed information");
        exit(2);
    }

    // Get variable screen information
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
        perror("Error reading variable information");
        exit(3);
    }

    printf("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);

    // Figure out the size of the screen in bytes
    screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;

    // Map the device to memory
    fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if ((int)fbp == -1) {
        perror("Error: failed to map framebuffer device to memory");
        exit(4);
    }
    printf("The framebuffer device was mapped to memory successfully.\n");

}

void printpixel(int x, int y){
    location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) +
                       (y+vinfo.yoffset) * finfo.line_length;

    if (vinfo.bits_per_pixel == 32) {
        *(fbp + location) = 255;
        *(fbp + location + 1) = 255;
        *(fbp + location + 2) = 255;
        *(fbp + location + 3) = 0;
    }
}

void bres_line(int x1, int y1, int x2, int y2){
    int dx, dy, x, y, x_end, p, const1, const2;
    
    dx = abs(x1-x2);
    dy = abs(y1-y2);
    
    p = 2 * dy - dx;
    const1 = 2 * dy;
    const2 = 2 * (dy-dx);

    if(x1 > x2){
        x = x2;
        y = y2;
        x_end = x1;
    }else{
        x = x1;
        y = y1;
        x_end = x2;
    }
    
    printpixel(x,y);
    while(x < x_end){
        x++;
        if(p < 0){
            p = p + const1;
        }else{
            y++;
            p = p + const2;
        }
        
        printpixel(x,y);
    }
}