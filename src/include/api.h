#ifndef API_H
#define API_H

#include <string>
#include <opencv2/opencv.hpp>
#include "api.h"

#define NUM_PROCESSORS 6

using namespace cv;
using namespace std;

typedef enum Direction {
	NO_DIRECTION = -2,
	HORIZONTAL = 0,
	VERTICAL = 1,
	BOTH = -1,
} Direction;

const Direction directions[] = { NO_DIRECTION, HORIZONTAL, VERTICAL, BOTH };

const char* DirStr(Direction d);

typedef enum Rotation {
	NO_ROTATION = 0,
	NINETY = 1,
	ONEEIGHTY = 2,
	TWOSEVENTY = 3,
} Rotation;

const Rotation rotations[] = { NO_ROTATION, NINETY, ONEEIGHTY, TWOSEVENTY };

const char* RotStr(Rotation r);

class TransformArtifact {
	public:
	int i, j;
	Direction direction;
	Rotation rotation;
	float contrast;
	float brightness;
};

class Transform {
	public:
	int i, j;
	Direction direction;
	Rotation rotation;
	float contrast;
	float brightness;
	Mat data;
	Mat jpeg;
	operator float () const {
		float sum = 0;
		for (int rx=0; rx<jpeg.rows; ++rx) {
			for (int cx=0; cx<jpeg.cols; ++cx) {
				float adjustor = ((8-rx) + (8-cx)) * 0.01;
				sum += adjustor * jpeg.at<unsigned char>(rx, cx);
			}
		}
		return sum;
	}
	bool operator < (Transform t) {
		return float(*this) < float(t);
	}
};

void reduce(const Mat &m, float factor, Mat &result);
void rotate(const Mat &m, Rotation rotation, Mat &result);
void flip(const Mat &m, Direction direction, Mat &result);
void apply_transform(const Mat &m, Transform &t, Mat &result);

float find_contrast(Mat m);
float find_brightness(Mat m);

vector<Transform> generate_transforms(const Mat &m, int src_sz, int dst_sz, int step); 

float distortion(const Mat &domain_block, const Mat &range_block, Transform &t);

vector<vector<Transform>> compress(const Mat &image,  int src_sz, int dst_sz, int step, int &rows_added, int &cols_added, int num_cores, bool silent);
vector<vector<Transform>> compress_slow(const Mat &image,  int src_sz, int dst_sz, int step, int &rows_added, int &cols_added, bool silent);
vector<vector<Transform>> compress(string input_file, int src_sz, int dst_sz, int step, int &rows_added, int &cols_added, int num_cores, bool silent=false);
void compress(string input_file, string output_file, int src_sz, int dst_sz, int step, int num_cores, bool silent=false);

vector<Mat> decompress(const vector<vector<Transform>> &transforms, int source_size=16, int destination_size=8, int step=4, int num_iters=8);
Mat decompress(string input_file, int num_iters);
void decompress(string input_file, string output_file, int num_iters);

int get_filesize(string path);
float MSE(const Mat &M, const Mat &N);
float MSE(string source_file, string decoded_file);

#endif /* API_H */