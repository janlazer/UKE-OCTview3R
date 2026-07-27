#ifndef ZSF_DEFINITIONS_H
#define ZSF_DEFINITIONS_H


#define MAXCHANNELS	4096
#define PLOT_SIZE 201
#define MM_PER_PIXEL 0.018003358
#define NUM_FILENAME_SEPARATORS 11	//this is the number of components of a single saved image file name (LabView-RayTracing-Software legacy)
#define VALUE_STRING_SIZE 5			//this is needed to generate equal sized strings from values within spot image file name. "natural sort" is performed with these equal sized strings.
#define MAX_MESSAGES 32				//this is the number of max displayed info messages 
#define CSV_NUMBER_PRECISION 13		//this is the precision of the numbers that are exported to csv files (zernike coeffs, position and slope values)
#define CSV_NUMBER_FORMAT QLocale::German	//this is the format of the numbers that are exported to csv files (zernike coeffs, position and slope values)

struct point2d {
	double x, y;
};

struct zernikeTuple {
	int nollIndex;
	double coefficient;
};

struct slopeData {
	double x, y;
	double m_x, m_y;
};

struct surfaceData {
	double x, y;
	double value;
};

enum zernikeClass {
	ZHARALD,
	ZJAN,
	ZMIRO
};

enum zsfAlgorithm {
	LEASTSQUARE
};

enum algoID{
	LEASTSQUARE_SDV,
	LEASTSQUARE_QR,
	LEASTSQUARE_EQUATION
};

enum measurement{
	MEASURE,
	EVALUATE,
	MEASURE_AND_EVALUATE,
	MEASURE_AND_EVALUATE_LIVE
};

#endif // ZSF_DEFINITIONS_H