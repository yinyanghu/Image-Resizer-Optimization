Image_Rescale:	Image_Rescale.cpp
		g++ Image_Rescale.cpp -o Image_Rescale

Image_Rescale_HD:	Image_Rescale_HD.cpp
			g++ Image_Rescale_HD.cpp -o Image_Rescale_HD

debug:		Image_Rescale.cpp Image_Rescale_HD.cpp
		g++ Image_Rescale.cpp -o Image_Rescale -g
		g++ Image_Rescale_HD.cpp -o Image_Rescale_HD -g

clean:
		rm Image_Rescale
		rm Image_Rescale_HD
		rm vanilla_rescaled.ppm
		rm fast_rescaled.ppm
