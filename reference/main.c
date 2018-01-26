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
#include <time.h>
#include <pthread.h>
#include <termios.h>
#define MAX_NUM 1 << 8
#define MAX_KAL 20
#define PIXEL 800
#define EPS 1e-7
#define nArrKalimat 14


#define height 500
#define width 833.33

typedef struct {
	int x, y;
} Point;
char kalimat[MAX_NUM][MAX_NUM];
int RK[MAX_KAL][PIXEL][PIXEL];
int GK[MAX_KAL][PIXEL][PIXEL];
int BK[MAX_KAL][PIXEL][PIXEL];
int HK[MAX_KAL];
int WK[MAX_KAL];
#define initH 100
#define initW 100

void init();
void clearScr();
void swap(int *a, int *b);
void gambarGaris(int x1, int y1, int x2, int y2, int tebal);
void gambarPoint(int x,  int y);
void gambarGarisPutus(int x1, int y1, int x2, int y2, int tebal);
void gambarTank(int x1, int y1, int x2, int y2, int position, int sign_for_change_direction);
void gambarPolygon(int x, int y, int scale);
void gambarLingkaran(int a, int b, int r);
void gambarPesawat(int x, int y);
int collision(Point pesawat, Point T1, Point T2);

int fbfd = 0;
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
long int screensize = 0;
char *fbp = 0;
long int location = 0;

int nidxlooping = 150, idxloop = 0;
int idxplane_x, idxtank_sign, idxtank_pos;

//meledak
int meledak = 0;
Point T_1_now, T_2_now;
typedef struct {
	char jenis; //jenis garis yang digambar, s for straight, d for dashed, and c for curve/circle
	int x1;
	int y1;
	int x2;
	int y2;
} LINES;

void *detect_key(void *param){
	while(!meledak){
		char c = getchar();
		Point Pesawat;
		Pesawat.x = idxplane_x;
		Pesawat.y = 50;
		meledak = collision(Pesawat, T_1_now, T_2_now);
	}
}
int main()
{
	init();
	T_1_now.x = T_1_now.y = T_2_now.x = T_2_now.y = -1;
	pthread_t new_thread;
	if(pthread_create(&new_thread, NULL, detect_key, NULL)) {
		fprintf(stderr, "Error creating thread\n");
		return 1;
	}
	// INISIASI
	idxplane_x = -50;
	idxtank_pos = 8; idxtank_sign = 1;

	while (!meledak) {
		gambarPesawat(idxplane_x,50);
		gambarTank(300,450,500,450,idxtank_pos,idxtank_sign);

		gambarLingkaran(400,430,19);
		gambarLingkaran(360,430,19);
		gambarLingkaran(440,430,19);
		gambarLingkaran(320,430,19);
		gambarLingkaran(480,430,19);
		
		// ========== MOVEMENT 
		idxloop++;
		// Pesawat
		idxplane_x += 10;
		if (idxplane_x >= 900){
			idxplane_x = -100;
		}
		// Tank
		// idxtank_sign atau sig_for_change_directioon ga dipake karna tidak ada perubahan arah dari peluru
		if(idxloop % 10 == 0){
			if ((idxtank_pos == 9) && (idxtank_sign == 1)) {
				idxtank_pos = 10;
			}
			else if ((idxtank_pos == 9) && (idxtank_sign == 0)) {
				idxtank_pos = 8;
			}
			else if (idxtank_pos == 8) {
				idxtank_pos = 9;
				idxtank_sign = 1;
			}
			else if (idxtank_pos == 10) {
				idxtank_pos = 9;
				idxtank_sign = 0;
			}
		}
		nanosleep((const struct timespec[]){{0,100000000L}},NULL);
		//============== END MOVEMENT
		
		if(!meledak){
			clearScr();
		}

	}
    nanosleep((const struct timespec[]){{0,100000000L}},NULL);
    clearScr();
    gambarTank(300,450,500,450,idxtank_pos,idxtank_sign);
    gambarPolygon(idxplane_x + 50,50,10);
    
		gambarLingkaran(400,430,19);
		gambarLingkaran(360,430,19);
		gambarLingkaran(440,430,19);
		gambarLingkaran(320,430,19);
		gambarLingkaran(480,430,19);

	munmap(fbp, screensize);
	close(fbfd);
	if(pthread_join(new_thread, NULL)) {
		fprintf(stderr, "Error joining thread\n");
		return 2;
	}

	return 0;
}

void init(){
	//============ Init FrameBuffer
	// Open the file for reading and writing
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
	if (*fbp == -1) {
		perror("Error: failed to map framebuffer device to memory");
		exit(4);
	}
	printf("The framebuffer device was mapped to memory successfully.\n");
	//============ EndOF Init FrameBuffer
}

void clearScr(){
	for (int h = 0; h < height; h++)
		for (int w = 0; w < width; w++) {

			location = (w + initW + vinfo.xoffset) * (vinfo.bits_per_pixel/8) +
				(h + initH + vinfo.yoffset) * finfo.line_length;

			if (vinfo.bits_per_pixel == 32) {
				*(fbp + location) = 255;        // Some blue
				*(fbp + location + 1) = 255; //15+(x-100)/2;     // A little green
				*(fbp + location + 2) = 255; //200-(y-100)/5;    // A lot of red
				*(fbp + location + 3) = 0;      // No transparency
			}
		}
}

void swap(int *a, int *b){
	int temp = *a;
	*a = *b;
	*b = temp;
}

void gambarGaris(int x1, int y1, int x2, int y2, int tebal){
	for(int t1 = -tebal; t1 <= tebal; ++t1){
		for(int t2 = -tebal; t2 <= tebal; ++t2){
			int xawal = x1 < x2 ? x1 : x2;
			int xakhir = x2 > x1 ? x2 : x1;

			int yawal = y1, yakhir = y2;
			if(xawal != x1){
				swap(&yawal, &yakhir);
			}

			xawal += t1;
			xakhir += t1;
			yawal += t2;
			yakhir += t2;

			int selisihAbsX = xawal - xakhir >= 0 ? xawal - xakhir : xakhir - xawal;
			int selisihAbsY = yawal - yakhir >= 0 ? yawal - yakhir : yakhir - yawal;

			//bagi kasus kalo selisihnya 0
			if(selisihAbsX == 0){
				if(yawal > yakhir) swap(&yawal, &yakhir);
				for(int i = yawal; i <= yakhir; ++i){
					gambarPoint(xawal, i);
				}
			}
			else if (selisihAbsY == 0){
				if(xawal > xakhir) swap(&xawal, &xakhir);
				for(int i = xawal; i <= xakhir; ++i){
					gambarPoint(i, yawal);
				}
			}
			else if(selisihAbsY < selisihAbsX){
				if(xawal > xakhir) swap(&xawal, &xakhir), swap(&yawal, &yakhir);
				int deltaY = yakhir - yawal;
				int deltaX = xakhir - xawal;
				for(int i = 0; i <= deltaX; ++i){
					int modY = (deltaY * i) % deltaX;
					int ynow = yawal + (deltaY * i) / deltaX + (modY * 2) / deltaX;
					gambarPoint(xawal + i, ynow);
				}
			}
			else{
				if(yawal > yakhir) swap(&xawal, &xakhir), swap(&yawal, &yakhir);
				int deltaY = yakhir - yawal;
				int deltaX = xakhir - xawal;
				for(int i = 0; i <= deltaY; ++i){
					int modX = (deltaX * i) % deltaY;
					int xnow = xawal + (deltaX * i) / deltaY + (modX * 2) / deltaY;
					gambarPoint(xnow, yawal + i);
				}
			}

		}
	}
}

void gambarPoint(int x, int y){
	if (y < height && x < width && y >= 0 && x >= 0){
		int location = (x + initW + vinfo.xoffset) * (vinfo.bits_per_pixel/8) +
			(y + initH + vinfo.yoffset) * finfo.line_length;

		if (vinfo.bits_per_pixel == 32) {
			*(fbp + location) = 200;        // Some blue
			*(fbp + location + 1) = 0; //15+(x-100)/2;     // A little green
			*(fbp + location + 2) = 0; //200-(y-100)/5;    // A lot of red
			*(fbp + location + 3) = 0;      // No transparency
		}
	}
}

void gambarGarisPutus(int x1, int y1, int x2, int y2, int tebal){
	int spasi = (tebal+1) * 5;
	int putus = (tebal+1) * 20;
	for(int t1 = -tebal; t1 <= tebal; ++t1){
		for(int t2 = -tebal; t2 <= tebal; ++t2){
			int xawal = x1 < x2 ? x1 : x2;
			int xakhir = x2 > x1 ? x2 : x1;

			int yawal = y1, yakhir = y2;
			if(xawal != x1){
				swap(&yawal, &yakhir);
			}

			xawal += t1;
			xakhir += t1;
			yawal += t2;
			yakhir += t2;

			int selisihAbsX = xawal - xakhir >= 0 ? xawal - xakhir : xakhir - xawal;
			int selisihAbsY = yawal - yakhir >= 0 ? yawal - yakhir : yakhir - yawal;

			//bagi kasus kalo selisihnya 0
			if(selisihAbsX == 0){
				if(yawal > yakhir) swap(&yawal, &yakhir);
				for(int i = yawal; i <= yakhir;){
					int j = 0;
					while (j<putus && i<=yakhir){
						gambarPoint(xawal, i);
						++i;
						++j;
					}
					i+=spasi;
				}
			}
			else if (selisihAbsY == 0){
				if(xawal > xakhir) swap(&xawal, &xakhir);
				for(int i = xawal; i <= xakhir;){
					int j = 0;
					while (j<putus && i<=xakhir){
						gambarPoint(i, yawal);
						++i;
						++j;
					}
					i+=spasi;
				}
			}
			else if(selisihAbsY < selisihAbsX){
				if(xawal > xakhir) swap(&xawal, &xakhir), swap(&yawal, &yakhir);
				int deltaY = yakhir - yawal;
				int deltaX = xakhir - xawal;
				for(int i = 0; i <= deltaX;){
					int j = 0;
					while (j<putus && i<=deltaX){
						int modY = (deltaY * i) % deltaX;
						int ynow = yawal + (deltaY * i) / deltaX + (modY * 2) / deltaX;
						gambarPoint(xawal + i, ynow);
						++i;
						++j;
					}
					i+=spasi;
				}
			}
			else{
				if(yawal > yakhir) swap(&xawal, &xakhir), swap(&yawal, &yakhir);
				int deltaY = yakhir - yawal;
				int deltaX = xakhir - xawal;
				for(int i = 0; i <= deltaY;){
					int j = 0;
					while (j<putus && i<=deltaY){
						int modX = (deltaX * i) % deltaY;
						int xnow = xawal + (deltaX * i) / deltaY + (modX * 2) / deltaY;
						gambarPoint(xnow, yawal + i);
						++j;
						++i;
					}
					i+=spasi;
				}
			}
		}
	}
}

void gambarTank(int x1, int y1, int x2, int y2, int position, int sign_for_change_direction) {
	int line_to_draw = 9;
	//int position = 9; //use 8,9,10 instead of 0,1,2
	//int sign_for_change_direction = 1; //1 is positive, 0 is negative
	int tebal_default = 1;
	int i = 0; //for transition debug or nothing

	int peluru_x[11];
	peluru_x[8] = 240; peluru_x[9] = 399; peluru_x[10] = 550;
	int peluru_y = 75;

	LINES tank[line_to_draw+2]; //there are 3 states for turret position

	//tank itu sendiri
	tank[0].jenis = 's'; tank[0].x1 = x1; tank[0].y1 = y1; tank[0].x2 = x2; tank[0].y2 = y2;
	tank[1].jenis = 's'; tank[1].x1 = x1; tank[1].y1 = y1; tank[1].x2 = x1; tank[1].y2 = y1-40;
	tank[2].jenis = 's'; tank[2].x1 = x2; tank[2].y1 = y2; tank[2].x2 = x2; tank[2].y2 = y2-40;
	tank[3].jenis = 's'; tank[3].x1 = x1; tank[3].y1 = y1-40; tank[3].x2 = x2; tank[3].y2 = y2-40;
	tank[4].jenis = 's'; tank[4].x1 = x1+10; tank[4].y1 = y1-40; tank[4].x2 = x1+30; tank[4].y2 = y1-70;
	tank[5].jenis = 's'; tank[5].x1 = x2-10; tank[5].y1 = y2-40; tank[5].x2 = x2-30; tank[5].y2 = y2-70;
	tank[6].jenis = 's'; tank[6].x1 = x1+30; tank[6].y1 = y1-70; tank[6].x2 = x2-30; tank[6].y2 = y2-70;
	tank[7].jenis = 's'; tank[7].x1 = x1+50; tank[7].y1 = y1-72; tank[7].x2 = x2-50; tank[7].y2 = y2-72;

	//turret (meriam)
	tank[8].jenis = 's'; tank[8].x1 = (x1+x2)/2; tank[8].y1 = y1-75; tank[8].x2 = (x1+x2)/2-12; tank[8].y2 = y2-101; //turret left
	tank[9].jenis = 's'; tank[9].x1 = (x1+x2)/2; tank[9].y1 = y1-75; tank[9].x2 = (x1+x2)/2; tank[9].y2 = y2-105; //turret up
	tank[10].jenis = 's'; tank[10].x1 = (x1+x2)/2; tank[10].y1 = y1-75; tank[10].x2 = (x1+x2)/2+12; tank[10].y2 = y2-101; //turret right

	// while(i<3000) {
	for (int j=0;j<line_to_draw+2;j++) {
		if (tank[j].jenis == 's') { //straight
			if (j < 8) { //not turret
				gambarGaris(tank[j].x1,tank[j].y1,tank[j].x2,tank[j].y2,tebal_default);
			}
			else if (j >= 8) { //turret
				gambarGaris(tank[position].x1,tank[position].y1,tank[position].x2,tank[position].y2,tebal_default+2);
				T_1_now.x = tank[position].x1;
				T_1_now.y = tank[position].y1;

				T_2_now.x = tank[position].x2;
				T_2_now.y = tank[position].y2;
				// BUAT PELURU
				gambarGarisPutus(tank[position].x2, tank[position].y2, peluru_x[position], peluru_y, 1);

				j = 20;
			}
		}
		else if (tank[j].jenis == 'd') { //dashed
			gambarGarisPutus(tank[j].x1,tank[j].y1,tank[j].x2,tank[j].y2,tebal_default);
		}
		else if (tank[j].jenis == 'c') {
			//gambarLingkaran(x1,y1,y2);
		}
	}
	//      nanosleep((const struct timespec[]){{0,500000000L}},NULL);
	i++;
	//       clearScr();

	// }
}

void gambarPolygon(int x, int y, int scale){

	Point P[8];

	P[0].x = x + (scale * 4);
	P[0].y = y;

	P[1].x = x;
	P[1].y = y + (scale * 4);

	P[2].x = P[0].x;
	P[2].y = y + (scale * 8);

	P[3].x = x + (scale * 8);
	P[3].y = P[1].y;

	P[4].x = x + scale;
	P[4].y = y + scale;

	P[5].x = x + (scale * 7);
	P[5].y = P[4].y;

	P[6].x = P[4].x;
	P[6].y = y + (scale * 7);

	P[7].x = P[5].x;
	P[7].y = P[6].y;

	int awal[] = {0, 0, 1, 1, 2, 2, 3, 3};
	int akhir[] = {6, 7, 5, 7, 4, 5, 4, 6};

	for(int i = 0;i < 8; ++i){
		gambarGaris(P[awal[i]].x, P[awal[i]].y, P[akhir[i]].x, P[akhir[i]].y, scale / 5);
	}
}

void gambarLingkaran(int a, int b, int r) {
	//persamaan lingkaran: (x-a)^2 + (y-b)^2 - r^2 = 0
	int hasil;
	for (int x = a-r; x <= a+r; x++) {
		for (int y = b-r; y <= b+r; y++) {
			hasil = ((x-a)*(x-a)) + ((y-b)*(y-b)) - (r*r);
			if (hasil < 0) {
				gambarPoint(x, y);
			}
		}
	}
}

void gambarPesawat(int x, int y) {

	Point P[15];
	int tebal = 2;

	// 0,0
	P[0].x = x;	P[0].y = y;

	//100, 0
	P[1].x = P[0].x + 100;
	P[1].y = P[0].y; 

	//25, 30
	P[2].x = P[0].x + 25;
	P[2].y = P[0].y + 30;

	//150, 30
	P[3].x = P[2].x + 125;
	P[3].y = P[2].y;

	//0, -25
	P[4].x = P[0].x;
	P[4].y = P[0].y - 25;

	//20, -25
	P[5].x = P[4].x + 20;
	P[5].y = P[4].y;

	//25, 0
	P[6].x = P[5].x + 5;
	P[6].y = P[0].y;

	//50, 0
	P[7].x = P[0].x + 50;
	P[7].y = P[0].y;

	//45, -20
	P[8].x = P[0].x + 45;
	P[8].y = P[0].y - 20;

	//65, -20
	P[9].x = P[8].x + 20;
	P[9].y = P[8].y;

	//80, 0
	P[10].x = P[0].x + 80;
	P[10].y = P[0].y;

	//50, 15
	P[11].x = P[7].x;
	P[11].y = P[7].y + 15;

	//45, 40
	P[12].x = P[8].x;
	P[12].y = P[8].y + 60;

	//65, 40
	P[13].x = P[9].x;
	P[13].y = P[9].y + 60;

	//80, 15
	P[14].x = P[10].x;
	P[14].y = P[10].y + 15;

	//max : 150, 40
	//min : 0, -25

	int awal[] = {0, 0, 2, 1, 0, 4, 5, 7, 8, 9, 11, 12, 13};
	int akhir[] = {1, 2, 3, 3, 4, 5, 6, 8, 9, 10, 12, 13, 14};
	for(int i = 0;i < 13; ++i){
		gambarGaris(P[awal[i]].x, P[awal[i]].y, P[akhir[i]].x, P[akhir[i]].y, tebal / 2);
	}   
}

int collision(Point pesawat, Point T1, Point T2){
	if(T1.x == -1) return 0;
	int collide = 0;

	Point Pkanan = pesawat;
	Point Pkiri = pesawat;

	Pkanan.x += 150;
	Pkanan.y += 40;

	Pkiri.x += 0;
	Pkiri.y += -25;

	//bagi kasus kalo ternyata mereka satu garis lurus

	if(T1.x == T2.x){
		if(T1.x >= Pkiri.x && T1.x <= Pkanan.x){
			collide = 1;
		}
		else collide = 0;
	}
	else{
		double selisihX = T1.x - T2.x;
		double selisihY = T1.y - T2.y;

		double kali_kiri = (double) (Pkiri.y - T2.y) / selisihY;
		double kali_kanan = (double) (Pkanan.y - T2.y) / selisihY;

		double X_peluru_kiri = T2.x + selisihX * kali_kiri;
		double X_peluru_kanan = T2.x + selisihX * kali_kanan;

		double lo_X = Pkiri.x;
		double hi_X = Pkanan.x;

		if((X_peluru_kiri >= lo_X && X_peluru_kiri <= hi_X) || (X_peluru_kanan >= lo_X && X_peluru_kanan <= hi_X)){
			collide = 1;
		}
		else{
			collide = 0;
		}
	}
	return collide;

}
