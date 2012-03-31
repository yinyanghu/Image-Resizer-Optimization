/*

  代码优化实验——如何让你的代码“跑得更快”

  
 	图像/视频是现代计算机进行数据处理的重要对象之一。
	图像/视频包含的数据（像素）量庞大，而各种图像/视频处理算法都常需要针对大量的像素进行重复、相似的计算操作，因此效率是一个十分重要的考虑因素。

例如自动的改变图像分辨率以实现图像放大或缩小的效果。这个操作在平时计算机上浏览图片/视频时常常用到（比如缩放窗口或专门的缩放图片等）

   而正因为这类算法中运算步骤的重复性、相似性和显而易见的并行性，现代CPU或GPU会用硬件来实现部分基础性甚至进阶的图像处理算法。当然，这需要硬件的支持。

但是，除了硬件支持之外，我们依然可以从软件编码的角度去考虑如何改善这类算法的性能。仔细的编码，从小处着眼，去发现什么样的改变会对算法效率的提高带来令人满意的效果。

希望通过这个实验帮助大家理解并掌握从计算机组织的角度去进行代码优化的技能。


我们提供的代码包含
   - 读取一个测试图片.ppm file
   - 一个已经实现的函数slow_rescale()，对图片进行双线性插值以完成图片放大
   - 已经实现的readPPMimage(),getPixel(),setPixel(),可供大家使用
   - 一个空的函数fast_rescale()
   - main()函数负责调用slow_rescale和fast_rescale并测试它们各自每秒可以执行放大的次数（也就是算法的帧率FPS-Frame per Second）
   - 将缩放后的图像文件写入磁盘以供检查.
   - 不对视频进行操作，所以只是通过对同一个测试图片进行反复调用缩放函数来完成。这样也有助于避免视频播放中的视频解码传输等额外操作带来的延迟。

你们需要做的事情：
- 请务必用我们提供的compile.sh来编译你的代码！
- 填充这个空的函数：fast_rescale()，主要目标就是通过代码的优化来达到比slow_rescale()函数更高的效率（帧率）
- 你的函数效率越快，分数越高。
- 在你的代码中，凡是你认为对提高效率有帮助的代码处，都请给出文字说明，解释你为什么觉得这样做可以更快
- 文字说明的越合理，分数越高。


其他建议：
- 如果想了解更多关于图像缩放的原理，可以和我们联系。
- 必须在同一个环境/编译选项下进行编译和运行，否则无法比较。
- 不要修改已有的代码，而只是填充fast_rescale()或增加你自己认为有用的辅助函数
- 可以用getPixel() 和setPixel() ，也可以不用。



  特别提醒:

   必须使用我们给出的compile.sh对本代码进行编译。
  
 
*/

// Image size definition for 1080p
#define HD_Xres 1920
#define HD_Yres 1080

/*
  Standard C libraries
*/
#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#include<time.h>
/*
  Function prototypes
*/

unsigned char *fast_rescale(unsigned char *src, int src_x, int src_y, int dest_x, int dest_y);
unsigned char *slow_rescale(unsigned char *src, int src_x, int src_y, int dest_x, int dest_y);
void getPixel(unsigned char *image, int x, int y, int sx, unsigned char *R, unsigned char *G, unsigned char *B);
void setPixel(unsigned char *image, int x, int y, int sx, unsigned char R, unsigned char G, unsigned char B);
int main(int argc, char *argv[]);
unsigned char *readPPMimage(const char *filename, int *sx, int *sy);
void imageOutput(unsigned char *im, int sx, int sy, const char *name);

/*****************************************************
** 请填充下面这个函数，
** 一定要比slow_rescale()快哦！
*****************************************************/


//bool calced = true;

unsigned char *fast_rescale(unsigned char *src, int src_x, int src_y, int dest_x, int dest_y)
{
	
	static int f[101][1024];
	
//	if (calced)
//	{
//			printf("*************");
//		asm("halt");
//		calced = false;
	for (int i = 0; i <= 100; ++ i)
		for (int j = -510; j <= 510; ++ j)
			f[i][j + 510] = ((i * j) / 100);
//		asm("halt");
//	}
//	register unsigned char R1, G1, B1, R2, G2, B2, empty1, empty2;
//	register unsigned char R3, G3, B3, R4, G4, B4, empty3, empty4;

	register unsigned char A[16];
	register int B[9];
//	unsigned char B[8];

//	register int RT1, GT1, BT1, RT3, GT3, BT3, RT2, GT2, BT2;

	
	unsigned char *dst;			// Destination image - must be allocated here! 
	dst = (unsigned char *)calloc(dest_x * dest_y * 3, sizeof(unsigned char));   // Allocate and clear destination image
	if (!dst) return(NULL);					       // Unable to allocate image
	
	register double step_x, step_y;			// Step increase as per instructions above
	//unsigned char R1, G1, B1, R2, G2, B2, empty1, empty2;		// Colours at the four neighbours
	//unsigned char R3, G3, B3, R4, G4, B4, empty3, empty4;
//	long long T1, T2;
//	long long Color;
//	register unsigned char R, G, B;//, RR, GG, BB;			// Final colour at a destination pixel
	register int x, y, xx, xxx;			// Coordinates on destination image
	double fx, fy;				// Corresponding coordinates on source image
	register int dy;				// Fractional component of source image coordinates
	register unsigned char *offset = dst;

	step_x = (double)(src_x - 1)/(double)(dest_x - 1);
	step_y = (double)(src_y - 1)/(double)(dest_y - 1);
//	asm("kkk");
//	R1 = 1; G1 = 1; B1 = 1; R2 = 1; G2 = 1; B2 = 1; RT1 = 1; GT1 = 1; BT1 = 1;
//	R3 = 1; G3 = 1; B3 = 1; R4 = 1; G4 = 1; B4 = 1; RT2 = 1; GT2 = 1; BT2 = 1;
//	&R1; &G1; &B1; &R2; &G2; &B2; &empty1; &empty2;
//	&R3; &G3; &B3; &R4; &G4; &B4; &empty3; &empty4;
//	asm("kkk");
	
//	printf("0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n", &R1, &G1, &B1, &R2, &G2, &B2, &RT1, &GT1, &BT1);
//	printf("0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n", &R3, &G3, &B3, &R4, &G4, &B4, &RT2, &GT2, &BT2);
	

		
	
	//printf("%.5lf\n", step_x);
	//printf("%.5lf\n", step_y);
	
	static int fl_x[HD_Xres], three_fl_x[HD_Xres];
	static int dx[HD_Xres];
	
	int fl_y;
//	int cl_x, cl_y;
//	register int k;
	register const int delta = 3 * src_x;
	// Loop over destination image
	register unsigned char *p;
	register unsigned char *q;
	register unsigned char *pp = A;
	register unsigned char *qq = A + 8;
	register int *left, *right;
	register int *k;
	
//	printf("0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n", &R1, &G1, &B1, &R2, &G2, &B2, &RT1, &GT1);
//	printf("0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n", &R3, &G3, &B3, &R4, &G4, &B4, &RT2, &GT2);
	
	fx = 0;

	for (int i = 0; i != dest_x; ++ i)
	{
		fl_x[i] = int(fx);
		dx[i] = int((fx - fl_x[i]) * 100);
		three_fl_x[i] = 3 * fl_x[i];
		fx += step_x;
	}
	fy = 0;
	for (y = 0; y != dest_y; ++ y)
	{
		fl_y = int(fy);
		dy = int((fy - fl_y) * 100);
		q = src + 3 * src_x * fl_y;
		
		
		//right = f[dy];
		
		p = q + three_fl_x[0];
		*((long long*)pp) = *((long long*)p);
		*((long long*)qq) = *((long long*)(p + delta));
		
		B[0] = 510 + A[0] + A[11] - A[3] - A[8];
		B[1] = 510 + A[3] - A[0];
		B[2] = 510 + A[8] - A[0];
		
		B[3] = 510 + A[1] + A[12] - A[4] - A[9];
		B[4] = 510 + A[4] - A[1];
		B[5] = 510 + A[9] - A[1];
		
		
		B[6] = 510 + A[2] + A[13] - A[5] - A[10];
		B[7] = 510 + A[5] - A[2];
		B[8] = 510 + A[10] - A[2];
		
		//left = f[dx[0]];
		
	//	*offset ++ = *(left + *(right + B[0]) + B[1]) + *(right + B[2]) + A[0];
	//	*offset ++ = *(left + *(right + B[3]) + B[4]) + *(right + B[5]) + A[1];
	//	*offset ++ = *(left + *(right + B[6]) + B[7]) + *(right + B[8]) + A[2];
		
		*offset ++ = f[dx[0]][f[dy][B[0]] + B[1]] + f[dy][B[2]] + A[0];
		*offset ++ = f[dx[0]][f[dy][B[3]] + B[4]] + f[dy][B[5]] + A[1];
		*offset ++ = f[dx[0]][f[dy][B[6]] + B[7]] + f[dy][B[8]] + A[2];

//		*offset ++ = (R1 + R4 - R2 - R3) * dy * dx[0] + (R2 - R1) * dx[0] + (R3 - R1) * dy + R1;
//		*offset ++ = (G1 + G4 - G2 - G3) * dy * dx[0] + (G2 - G1) * dx[0] + (G3 - G1) * dy + G1;
//		*offset ++ = (B1 + B4 - B2 - B3) * dy * dx[0] + (B2 - B1) * dx[0] + (B3 - B1) * dy + B1;
		
		for (x = 1; x != dest_x; ++ x)
		{
			if (three_fl_x[x] != three_fl_x[x - 1])
			{
				p = p + 3;
				*((long long*)pp) = *((long long*)p);
				*((long long*)qq) = *((long long*)(p + delta));
				B[0] = 510 + A[0] + A[11] - A[3] - A[8];
				B[1] = 510 + A[3] - A[0];
				B[2] = 510 + A[8] - A[0];
		
				B[3] = 510 + A[1] + A[12] - A[4] - A[9];
				B[4] = 510 + A[4] - A[1];
				B[5] = 510 + A[9] - A[1];
		
		
				B[6] = 510 + A[2] + A[13] - A[5] - A[10];
				B[7] = 510 + A[5] - A[2];
				B[8] = 510 + A[10] - A[2];
			}
				//left = f[dx[x]];
		/*
				*offset ++ = *(left + *(right + B[0]) + B[1]) + *(right + B[2]) + A[0];
				*offset ++ = *(left + *(right + B[3]) + B[4]) + *(right + B[5]) + A[1];
				*offset ++ = *(left + *(right + B[6]) + B[7]) + *(right + B[8]) + A[2];
		*/	
				*offset ++ = f[dx[x]][f[dy][B[0]] + B[1]] + f[dy][B[2]] + A[0];
				*offset ++ = f[dx[x]][f[dy][B[3]] + B[4]] + f[dy][B[5]] + A[1];
				*offset ++ = f[dx[x]][f[dy][B[6]] + B[7]] + f[dy][B[8]] + A[2];
			/*
			*offset ++ = (R1 + R4 - R2 - R3) * dx[x] * dy + (R2 - R1) * dx[x] + (R3 - R1) * dy + R1;
			*offset ++ = (G1 + G4 - G2 - G3) * dx[x] * dy + (G2 - G1) * dx[x] + (G3 - G1) * dy + G1;
			*offset ++ = (B1 + B4 - B2 - B3) * dx[x] * dy + (B2 - B1) * dx[x] + (B3 - B1) * dy + B1;
			*/
			//k = &dx[x];
		//	left = f[dx[x]];
		//	*offset ++ = *(left + *(right + RT1) + RT2) + *(right + RT3) + R1;
		//	*offset ++ = *(left + *(right + GT1) + GT2) + *(right + GT3) + G1;
		//	*offset ++ = *(left + *(right + BT1) + BT2) + *(right + BT3) + B1;
			//*offset ++ = f[dx[x]][f[dy][RT1] + RT2] + f[dy][RT3] + R1;
			//*offset ++ = f[dx[x]][f[dy][GT1] + GT2] + f[dy][GT3] + G1;
			//*offset ++ = f[dx[x]][f[dy][BT1] + BT2] + f[dy][BT3] + B1;
			
		}
		
		fy += step_y;
	}
	return(dst);		
}


/*****************************************************
** 不要修改下面的任何函数和代码！！！
**
**
**  如果需要，你可以阅读这些代码，或调用它们，或copy其中的部分。
** （注意不要让它们过多的局限你的修改思路）
** 
**  但是，请仔细阅读下面函数中的注释.
**  便于你理解 slow_rescale()是如何实现图片放大的
*****************************************************/
unsigned char *slow_rescale(unsigned char *src, int src_x, int src_y, int dest_x, int dest_y)
{
 /*
   图像数据类似于一个矩阵，矩阵的宽和高分别就是图像的宽（x方向长度）和高（y方向长度）
   每个矩阵的元素就是图像中的一个像素点，记录的是这个点的颜色
   但一个颜色需要用3个8位的元素分别记录构成这个颜色的RGB值。

   因此，假设图像的宽为sx，高为sy
   那么存储图像中所有像素的颜色信息需要sx * sy * 3个字节
   
   在本实验中，PPM图像将所有信息按行顺序存放在一维数组中 
   [(0,0) (1,0) (2,0) ... (sx-1,0) (0,1) (1,1) ... (sx-1,1) ... ... ... (sx-1,sy-1)]
   注意：这里的每个（x，y）代表一个像素，而真正存放这个像素的地址空间是连续的3个字节。.

   本文件中提供的 getPixel() 和 setPixel() 可以用来存取图像的像素颜色.
   你也可以写你字节的存取函数或甚至不使用存取函数以达到更快的速度

   ----------------------
   双线性插值放大算法思路：
   
   我们提供分辨率较小（？？？）的实验图片和一个大的目标分辨率（1920x1080）.

   因此需要用插值的方法从小图像的颜色计算出大图像的颜色。

   （1）计算目标图像和源图像的宽高比值.
  
   例如源图像的宽高为 (src_x, src_y)
   目标图像的宽高为   (dst_x, dst_y):
	
   则： 
   step_x=src_x/dst_x;
   step_y=src_y/dst_y;

   例如： 源(10 x 10) 和目标 (25 x 25) 可计算出step_x=step_y=0.4

   （2）源图像中的0.4个像素将要“变”为目标图像中的一个像素！

   也就是说，我们需要进行下列赋值：
   dst(0,0) <- src(0,0)
   dst(0,1) <- src(0,0.4)
   dst(0,2) <- src(0,0.8)
   dst(0,3) <- src(0,1.2)
   .
   .
   .
   dst(1,0) <- src(0.4,0)
   dst(1,1) <- src(0.4,0.4)
   .
   .
   等等
   
    
   但是, src(0.4,0.4) 这样以小数为下标的像素点并不是真实存在的。
   所以， 需要插值。

   （3） 插值的方法就是找到这个小数下标的上下左右共四个最近的整数下标像素点。如下图:
   
   对于 src(0.4,0.4)找到的4个点就是N1-N4:
                          
                                 N1               T1                       N2
                               src(0,0) -----------*----------------------src(1,0)
                                                   |
                                                   |
                                                   |
                                                src(0.4,0.4)
                                                   |
                                                   |
                                                   |
                                                   |
                                                   |
                                                   |
                                                   |
                              src(0,1)-------------*----------------------src(1,1)
                                 N3                T2                       N4

   
   利用N1和N2的颜色，以及根据0.4和“0”“1”之间各自的距离，可以插值计算得到T1的颜色,
   （即0.6*N1 + 0.4*N2 = T1)
   类似的，
   利用N3和N4的颜色，可以插值计算得到T2的颜色,
   利用T1和T2的颜色，可以插值计算得到最终所需要的src(0.4,0.4)的颜色,
   
   （4）注意这里的“颜色”都需要分RGB三个字节分别进行计算。

   而当源或目标分辨率很大的时候，计算次数十分庞大。所以，优化是十分必要的。

   下面是一个十分“不优化”的函数。你可以将之作为“反例”进行参考。
   重要的是只要你理解上面的插值过程，你就可以完成你自己的fast函数了！

 */

 double step_x,step_y;			// Step increase as per instructions above
 unsigned char R1,R2,R3,R4;		// Colours at the four neighbours
 unsigned char G1,G2,G3,G4;
 unsigned char B1,B2,B3,B4;
 double RT1, GT1, BT1;			// Interpolated colours at T1 and T2
 double RT2, GT2, BT2;
 unsigned char R,G,B;			// Final colour at a destination pixel
 unsigned char *dst;			// Destination image - must be allocated here! 
 int x,y;				// Coordinates on destination image
 double fx,fy;				// Corresponding coordinates on source image
 double dx,dy;			// Fractional component of source image coordinates

 dst=(unsigned char *)calloc(dest_x*dest_y*3,sizeof(unsigned char));   // Allocate and clear destination image
 if (!dst) return(NULL);					       // Unable to allocate image

 step_x=(double)(src_x-1)/(double)(dest_x-1);
 step_y=(double)(src_y-1)/(double)(dest_y-1);

 for (x=0;x<dest_x;x++)			// Loop over destination image
  for (y=0;y<dest_y;y++)
  {
   fx=x*step_x;
   fy=y*step_y;
   dx=fx-(int)fx;
   dy=fy-(int)fy;   
   getPixel(src,floor(fx),floor(fy),src_x,&R1,&G1,&B1);	// get N1 colours
   getPixel(src,ceil(fx),floor(fy),src_x,&R2,&G2,&B2);	// get N2 colours
   getPixel(src,floor(fx),ceil(fy),src_x,&R3,&G3,&B3);	// get N3 colours
   getPixel(src,ceil(fx),ceil(fy),src_x,&R4,&G4,&B4);	// get N4 colours
   // Interpolate to get T1 and T2 colours
   RT1=(dx*R2)+(1-dx)*R1;
   GT1=(dx*G2)+(1-dx)*G1;
   BT1=(dx*B2)+(1-dx)*B1;
   RT2=(dx*R4)+(1-dx)*R3;
   GT2=(dx*G4)+(1-dx)*G3;
   BT2=(dx*B4)+(1-dx)*B3;
   // Obtain final colour by interpolating between T1 and T2
   R=(unsigned char)((dy*RT2)+((1-dy)*RT1));
   G=(unsigned char)((dy*GT2)+((1-dy)*GT1));
   B=(unsigned char)((dy*BT2)+((1-dy)*BT1));
   // Store the final colour
   setPixel(dst,x,y,dest_x,R,G,B);
  }
 return(dst);
}

void getPixel(unsigned char *image, int x, int y, int sx, unsigned char *R, unsigned char *G, unsigned char *B)
{
 // Get the colour at pixel x,y in the image and return it using the provided RGB pointers
 // Requires the image size along the x direction!
 *(R)=*(image+((x+(y*sx))*3)+0);
 *(G)=*(image+((x+(y*sx))*3)+1);
 *(B)=*(image+((x+(y*sx))*3)+2);
}

void setPixel(unsigned char *image, int x, int y, int sx, unsigned char R, unsigned char G, unsigned char B)
{
 // Set the colour of the pixel at x,y in the image to the specified R,G,B
 // Requires the image size along the x direction!
 *(image+((x+(y*sx))*3)+0)=R;
 *(image+((x+(y*sx))*3)+1)=G;
 *(image+((x+(y*sx))*3)+2)=B;
}

int main(int argc, char *argv[])
{
 /*
    Main routine:
    - Load the test image specified in the command line
    - Run both the slow and fast image scaling routines for a few seconds
    - Compute FPS for both
    - Save output images to disk
    - Print out FPS ratio of fast routine to slow routine (should be > 1!)
 */

 unsigned char *src;		// Used to store the source image
 unsigned char *dst;		// Will be used to hold the rescaled image
 int sx, sy;			// Resolution of the source image (sx * sy pixels)
 time_t t0, t1, t2, t3;
 int c_a,c_b;
 double FPS_a;
 double FPS_b;
 if (argc!=2)
 {
  fprintf(stderr,"Usage: Image_Rescale src_name\n");
  fprintf(stderr," src_name is the name of the test image (must be in .ppm format)\n");
  exit(1);
 }
 src=readPPMimage(argv[1], &sx, &sy);
 if (!src)
 {
  fprintf(stderr,"Unable to open test image\n");
  exit(1);
 }




 fprintf(stderr,"Starting tests...\n");
 // Time plain slow routine
 t1=t0=time(NULL);
 c_a=0;
 while(difftime(t1,t0)<3.0)
 {
  dst=slow_rescale(src,sx,sy,HD_Xres,HD_Yres);
  if (dst) {c_a++; free(dst);} else break;
  t1=time(NULL);
 }
 if (c_a>0)
 {
  FPS_a=c_a/(double)(t1-t0);
  fprintf(stderr,"slow image rescaling FPS=%f\n",FPS_a);
 }
 else
 {
  fprintf(stderr,"Something went wrong!\n");
 }


	

 // Time your fast routine
 t3=t2=time(NULL);
 c_b=0;
 while(difftime(t3,t2)<3.0)
 {
  dst=fast_rescale(src,sx,sy,HD_Xres,HD_Yres);
  if (dst) {c_b++; free(dst);} else break;
  t3=time(NULL);
 }
 if (c_b>0)
 {
  FPS_b=c_b/(double)(t3-t2);
  fprintf(stderr,"Fast image rescaling FPS=%f\n",FPS_b);
  fprintf(stderr,"Ratio: %f\n",FPS_b/FPS_a);
 }
 else
 {
  fprintf(stderr,"Fast routine not implemented\n");
 }







 // Output rescaled images for inspection
 dst=slow_rescale(src,sx,sy,HD_Xres,HD_Yres);
 if (dst) {imageOutput(dst,HD_Xres,HD_Yres,"vanilla_rescaled.ppm"); free(dst);}
 dst=fast_rescale(src,sx,sy,HD_Xres,HD_Yres);
 if (dst) {imageOutput(dst,HD_Xres,HD_Yres,"fast_rescaled.ppm"); free(dst);}

 fprintf(stderr,"Done!\n");
 free(src);
 exit(0);
}

unsigned char *readPPMimage(const char *filename, int *sx, int *sy)
{
 // Reads an image from a .ppm file. A .ppm file is a very simple image representation
 // format with a text header followed by the binary RGB data at 24bits per pixel.
 // The header has the following form:
 //
 // P6
 // # Optionally, one or more comment lines preceded by '#'
 // 340 200
 // 255
 //
 // The first line 'P6' is the .ppm format identifier, this is followed by one or more
 // lines with comments, typically used to inidicate which program generated the
 // .ppm file.
 // After the comments, a line with two integer values specifies the image resolution
 // as number of pixels in x and number of pixels in y.
 // The final line of the header stores the maximum value for pixels in the image,
 // usually 255.
 // After this last header line, binary data stores the RGB values for each pixel
 // in row-major order. Each pixel requires 3 bytes ordered R, G, and B.
 //
 // NOTE: Windows file handling is rather crotchetty. You may have to change the
 //       way this file is accessed if the images are being corrupted on read
 //       on Windows.
 //
 // readPPMdata converts the image colour information to doubleing point. This is so that
 // the texture mapping function doesn't have to do the conversion every time
 // it is asked to return the colour at a specific location.
 //
 // On error, the function returns NULL
 //

 FILE *f;
 unsigned char *im;
 char line[1024];
 int sizx,sizy;

 f=fopen(filename,"rb+");
 if (f==NULL)
 {
  fprintf(stderr,"Unable to open file %s for reading, please check name and path\n",filename);
  return(NULL);
 }
 fgets(&line[0],1000,f);
 if (strcmp(&line[0],"P6\n")!=0)
 {
  fprintf(stderr,"Wrong file format, not a .ppm file or header end-of-line characters missing\n");
  fclose(f);
  return(NULL);
 }

 // Skip over comments
 fgets(&line[0],511,f);
 while (line[0]=='#')
 {
  fgets(&line[0],511,f);
 }
 sscanf(&line[0],"%d %d\n",&sizx,&sizy);           // Read file size
 *(sx)=sizx;
 *(sy)=sizy;

 fgets(&line[0],9,f);  	                // Read the remaining header line
 im=(unsigned char *)calloc(sizx*sizy*3,sizeof(unsigned char));
 if (im==NULL)
 {
  fprintf(stderr,"Out of memory allocating space for image\n");
  fclose(f);
  return(NULL);
 }

 fread(im,sizx*sizy*3*sizeof(unsigned char),1,f);
 fclose(f);
 return(im);
}

void imageOutput(unsigned char *im, int sx, int sy, const char *name)
{
 /*
   Outputs the image stored in 'im' to a .ppm file for inspection
 */
 FILE *f;

 f=fopen(name,"w");
 if (f)
 {
  fprintf(f,"P6\n");
  fprintf(f,"# Image_Rescaling output\n");
  fprintf(f,"%d %d\n",sx,sy);
  fprintf(f,"255\n");
  fwrite(im,sx*sy*3*sizeof(unsigned char),1,f);
  fclose(f);
 }
 else
 {
  fprintf(stderr,"Unable to open file for image output!\n");
 }
}
