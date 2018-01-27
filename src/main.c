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
#include <string.h>

#define HEIGHT 500
#define WIDTH 800
#define INIT_HEIGHT 100
#define INIT_WIDTH 100

struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
char *fbp = 0;
int fbfd = 0;
long int screensize = 0;
long int location = 0;

void init();
void clearScreen();
void printpixelBG(int x, int y, int colorR, int colorG, int colorB);
void printpixel(int x, int y, int color);
void bres_line(int x1, int y1, int x2, int y2, int thickness);
void printPesawat(int kolom, int baris, int banyakBaris, int banyakKolom);
void printTank(int kolom, int baris, int banyakBaris, int banyakKolom);

char a[1000][1000];
char bg[1000][1000];
char tank[1000][1000];

int main() {
    // Open the file for reading and writing

    init();   
    FILE *fp, *fbg, *ftank;
    int i,j,k;
    
    fbg = fopen("../background.txt","r");
    if (fbg == NULL){
		printf("No data in bg.txt\n");
		return 0;
	}
	else{
		i = 0;
		while (fscanf(fbg, "%s",bg[i]) != EOF){
			i++;
		}
	}

    fp = fopen("../outputJet.txt","r");
    if(fp==NULL){
    	printf("File tidak ada\n");
    	return 0;
    }
    i = 0;
    while(fscanf(fp, "%s", a[i])!=EOF){
    	i++;
    }

    ftank = fopen("../tank.txt","r");
    if(fp==NULL){
        printf("File tidak ada\n");
        return 0;
    }
    k = 0;
    while(fscanf(ftank, "%s", tank[k])!=EOF){
        k++;
    }

    int idx = i;
    int ukuranBaris = idx, ukuranKolom = strlen(a[0]);
    int barisTank = k;
    int kolomTank = strlen(tank[0]);
    clearScreen();
    int kolom = 700;
    while(1){
    	int x = WIDTH/2;
    	int xTengah = WIDTH/2;
    	int xKanan = WIDTH/2;
	    for (int y = HEIGHT; y >100; y--) {
	    	clearScreen();
            printTank(xTengah-(kolomTank/2),HEIGHT-barisTank-10,barisTank,kolomTank);
            printf("%d \n",kolomTank);///////////////////////////////////////////////////////////
	    	printPesawat(kolom,2*ukuranBaris,ukuranBaris,ukuranKolom);
	    	kolom--;
	    	if(kolom<0){kolom=700;}

	    	//printing ke kiri atas
	    	if(x<20){break;}
		    bres_line(x,y,x-20,y-10,3);
		    x -= 2;

		    //printing ke atas
		    bres_line(xTengah,y,xTengah,y-10,10);

		    //printing ke kanan atas
		    bres_line(xKanan,y,xKanan+20,y-10, 5);
		    xKanan += 2;
		    
		    usleep(1000);
	    }

		
    }
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

//clearScreen
void clearScreen() {	//BackGround Screen
    for (int h = 0; h < HEIGHT; h++){
        for (int w = 0; w < WIDTH; w++) {
			switch (bg[h][w]){
				case '0': printpixelBG(w,h,0,0,0); break;
				case '1': printpixelBG(w,h,255,255,255); break;
				case '2': printpixelBG(w,h,255,255,0); break;
				case '3': printpixelBG(w,h,0,255,255); break;
				case '4': printpixelBG(w,h,0,255,0); break;
				case '5': printpixelBG(w,h,220,220,220); break;
			}
        }
    }
}

void printpixelBG(int x, int y, int colorR, int colorG, int colorB){	//Print Pixel Color using RGB
    location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) +
                       (y+vinfo.yoffset) * finfo.line_length;

    if (vinfo.bits_per_pixel == 32) {
        *(fbp + location) = colorB;			//Blue Color
        *(fbp + location + 1) = colorG;		//Green Color
        *(fbp + location + 2) = colorR;		//Red Color
        *(fbp + location + 3) = 0;			//Transparancy
    }
}

void printpixel(int x, int y, int color){
    location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) +
                       (y+vinfo.yoffset) * finfo.line_length;

    if (vinfo.bits_per_pixel == 32) {
        *(fbp + location) = color;
        *(fbp + location + 1) = color;
        *(fbp + location + 2) = color;
        *(fbp + location + 3) = 0;
    }
}

void bres_line(int x1, int y1, int x2, int y2, int thickness){
    int dx, dy, x, y, x_end, p, const1, const2, i;
    for(i = 0; i < thickness; i++){
        dx = abs(x1-x2);
        dy = abs(y1-y2);
    
        p = 2 * dy - dx;
        const1 = 2 * dy;
        const2 = 2 * (dy-dx);

        if(x1 > x2){
            x = x2 + i;
            y = y2;
            x_end = x1 + i;
        }else{
            x = x1 + i;
            y = y1;
            x_end = x2 + i;
        }
    
        printpixel(x,y,0);
        while(x < x_end){
            x++;
            if(p < 0){
                p = p + const1;
            }else{
                y++;
                p = p + const2;
            }
        
            printpixel(x,y,0);
        }
    }
}

void printPesawat(int kolom, int baris, int banyakBaris, int banyakKolom){
	int i,j;
	int y = baris, x = kolom;
	for(i=0;i<banyakBaris;i++){
		for(j=0;j<banyakKolom;j++){
			char kar = a[i][j];
			if(kar=='0'){printpixel(x,y,0);}
			x++;
		}
		y++;
		x = kolom;
	}
}
////////////////////////////////////////////////////////////////////
void printTank(int kolom, int baris, int banyakBaris, int banyakKolom){
    int i,j;
    int y = baris, x = kolom;
    for(i=0;i<banyakBaris;i++){
        for(j=0;j<banyakKolom;j++){
            char kar = tank[i][j];
            if(kar=='0'){printpixelBG(x,y,20,80,20);}
            x++;
        }
        y++;
        x = kolom;
    }
}
