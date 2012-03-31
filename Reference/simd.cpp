struct Pixel
{
 Pixel() :r(0), g(0), b(0), a(0) {}
 Pixel(int rgba) { *((int*)this) = rgba; }
 Pixel(unsigned char r, unsigned char g, unsigned char b, unsigned char a) : r(r), g(g), b(b), a(a) {}

 unsigned char r, g, b, a;
};

struct Image
{
 int width;
 int height; 
 Pixel* data;
};



inline Pixel GetPixel(const Image* img, float x, float y)
{
 int px = (int)x; // floor of x
 int py = (int)y; // floor of y
 const int stride = img->width;
 const Pixel* p0 = img->data + px + py * stride; // pointer to first pixel

 // load the four neighboring pixels
 const Pixel& p1 = p0[0 + 0 * stride];
 const Pixel& p2 = p0[1 + 0 * stride];
 const Pixel& p3 = p0[0 + 1 * stride];
 const Pixel& p4 = p0[1 + 1 * stride];

 // Calculate the weights for each pixel
 float fx = x - px;
 float fy = y - py;
 float fx1 = 1.0f - fx;
 float fy1 = 1.0f - fy;
 
 int w1 = fx1 * fy1 * 256.0f;
 int w2 = fx  * fy1 * 256.0f;
 int w3 = fx1 * fy  * 256.0f;
 int w4 = fx  * fy  * 256.0f;

 // Calculate the weighted sum of pixels (for each color channel)
 int outr = p1.r * w1 + p2.r * w2 + p3.r * w3 + p4.r * w4;
 int outg = p1.g * w1 + p2.g * w2 + p3.g * w3 + p4.g * w4;
 int outb = p1.b * w1 + p2.b * w2 + p3.b * w3 + p4.b * w4;
 int outa = p1.a * w1 + p2.a * w2 + p3.a * w3 + p4.a * w4;

 return Pixel(outr >> 8, outg >> 8, outb >> 8, outa >> 8);
}

Now, let's take a look at the SSE2 version of the same code:
// constant values that will be needed
static const __m128 CONST_1111 = _mm_set1_ps(1);
static const __m128 CONST_256 = _mm_set1_ps(256);

inline __m128 CalcWeights(float x, float y)
{
 __m128 ssx = _mm_set_ss(x);
 __m128 ssy = _mm_set_ss(y);
 __m128 psXY = _mm_unpacklo_ps(ssx, ssy);      // 0 0 y x

 //__m128 psXYfloor = _mm_floor_ps(psXY); // use this line for if you have SSE4
 __m128 psXYfloor = _mm_cvtepi32_ps(_mm_cvtps_epi32(psXY));
 __m128 psXYfrac = _mm_sub_ps(psXY, psXYfloor); // = frac(psXY)
 
 __m128 psXYfrac1 = _mm_sub_ps(CONST_1111, psXYfrac); // ? ? (1-y) (1-x)
 __m128 w_x = _mm_unpacklo_ps(psXYfrac1, psXYfrac);   // ? ?     x (1-x)
        w_x = _mm_movelh_ps(w_x, w_x);      // x (1-x) x (1-x)
 __m128 w_y = _mm_shuffle_ps(psXYfrac1, psXYfrac, _MM_SHUFFLE(1, 1, 1, 1)); // y y (1-y) (1-y)

 // complete weight vector
 return _mm_mul_ps(w_x, w_y);
}

inline Pixel GetPixelSSE(const Image* img, float x, float y)
{
 const int stride = img->width;
 const Pixel* p0 = img->data + (int)x + (int)y * stride; // pointer to first pixel

 // Load the data (2 pixels in one load)
 __m128i p12 = _mm_loadl_epi64((const __m128i*)&p0[0 * stride]); 
 __m128i p34 = _mm_loadl_epi64((const __m128i*)&p0[1 * stride]); 

 __m128 weight = CalcWeights(x, y);

 // extend to 16bit
 p12 = _mm_unpacklo_epi8(p12, _mm_setzero_si128());
 p34 = _mm_unpacklo_epi8(p34, _mm_setzero_si128());

 // convert floating point weights to 16bit integer
 weight = _mm_mul_ps(weight, CONST_256); 
 __m128i weighti = _mm_cvtps_epi32(weight); // w4 w3 w2 w1
         weighti = _mm_packs_epi32(weighti, _mm_setzero_si128()); // 32->16bit

 // prepare the weights
 __m128i w12 = _mm_shufflelo_epi16(weighti, _MM_SHUFFLE(1, 1, 0, 0));
 __m128i w34 = _mm_shufflelo_epi16(weighti, _MM_SHUFFLE(3, 3, 2, 2));
 w12 = _mm_unpacklo_epi16(w12, w12); // w2 w2 w2 w2 w1 w1 w1 w1
 w34 = _mm_unpacklo_epi16(w34, w34); // w4 w4 w4 w4 w3 w3 w3 w3
 
 // multiply each pixel with its weight (2 pixel per SSE mul)
 __m128i L12 = _mm_mullo_epi16(p12, w12);
 __m128i L34 = _mm_mullo_epi16(p34, w34);

 // sum the results
 __m128i L1234 = _mm_add_epi16(L12, L34); 
 __m128i Lhi = _mm_shuffle_epi32(L1234, _MM_SHUFFLE(3, 2, 3, 2));
 __m128i L = _mm_add_epi16(L1234, Lhi);
  
 // convert back to 8bit
 __m128i L8 = _mm_srli_epi16(L, 8); // divide by 256
 L8 = _mm_packus_epi16(L8, _mm_setzero_si128());
 
 // return
 return _mm_cvtsi128_si32(L8);
}





inline Pixel GetPixelSSE3(const Image<Pixel>* img, float x, float y)
{
 const int stride = img->width;
 const Pixel* p0 = img->data + (int)x + (int)y * stride; // pointer to first pixel

 // Load the data (2 pixels in one load)
 __m128i p12 = _mm_loadl_epi64((const __m128i*)&p0[0 * stride]); 
 __m128i p34 = _mm_loadl_epi64((const __m128i*)&p0[1 * stride]); 

 __m128 weight = CalcWeights(x, y);

 // convert RGBA RGBA RGBA RGAB to RRRR GGGG BBBB AAAA (AoS to SoA)
 __m128i p1234 = _mm_unpacklo_epi8(p12, p34);
 __m128i p34xx = _mm_unpackhi_epi64(p1234, _mm_setzero_si128());
 __m128i p1234_8bit = _mm_unpacklo_epi8(p1234, p34xx);

 // extend to 16bit 
 __m128i pRG = _mm_unpacklo_epi8(p1234_8bit, _mm_setzero_si128());
 __m128i pBA = _mm_unpackhi_epi8(p1234_8bit, _mm_setzero_si128());
 
 // convert weights to integer
 weight = _mm_mul_ps(weight, CONST_256); 
 __m128i weighti = _mm_cvtps_epi32(weight); // w4 w3 w2 w1
         weighti = _mm_packs_epi32(weighti, weighti); // 32->2x16bit

 //outRG = [w1*R1 + w2*R2 | w3*R3 + w4*R4 | w1*G1 + w2*G2 | w3*G3 + w4*G4]
 __m128i outRG = _mm_madd_epi16(pRG, weighti);
 //outBA = [w1*B1 + w2*B2 | w3*B3 + w4*B4 | w1*A1 + w2*A2 | w3*A3 + w4*A4]
 __m128i outBA = _mm_madd_epi16(pBA, weighti);

 // horizontal add that will produce the output values (in 32bit)
 __m128i out = _mm_hadd_epi32(outRG, outBA);
 out = _mm_srli_epi32(out, 8); // divide by 256
 
 // convert 32bit->8bit
 out = _mm_packus_epi32(out, _mm_setzero_si128());
 out = _mm_packus_epi16(out, _mm_setzero_si128());

 // return
 return _mm_cvtsi128_si32(out);
}

