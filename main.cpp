#include <iostream>
#include<fstream>
#include <opencv2/opencv.hpp>
#include <dehazor.h>
#include <ctime>
#include"filter.h"
using namespace std;
using namespace cv;

int main()
{
    time_t start = clock();
//    Mat img = imread("E:\\tempimage\\4 (2).jpg",CV_LOAD_IMAGE_COLOR);
//    Dehazor dehaze(img,15,0.95,0.01,220);
    Mat img = imread("E:\\tempimage\\IMG_70827.JPG",CV_LOAD_IMAGE_COLOR);
//    Mat img = imread("E:\\tempimage\\4.JPG",CV_LOAD_IMAGE_COLOR);
    Dehazor dehaze(img,120,1,0.01,220);
    dehaze.process();
    cv::imshow("img",img);
    std::cout <<"cost: "<<(clock() - start)<<"ms"<<std::endl;
    waitKey();
    return 0;
}
