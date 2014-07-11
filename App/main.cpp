#include <iostream>
#include<fstream>
#include <opencv2/opencv.hpp>
#include <darkimagedehazor.h>
#include <medianfliterdehazor.h>
#include <ctime>
#include"filter.h"
using namespace std;
using namespace cv;

int main()
{
    time_t start = clock();
//    DarkImageDehazor darkChannalDehaze(std::string("C:\\hr\\experiment\\tempimage\\images\\nonuniform.JPG"),1,0.01,220);
    MedianFliterDehazor medianDehaze(std::string("C:\\hr\\experiment\\tempimage\\images\\nonuniform.JPG"),220,1.3);
//    darkChannalDehaze.Process();
    medianDehaze.Process();
//    cv::imshow("img",img);
    std::cout <<"cost: "<<(clock() - start)<<"ms"<<std::endl;
    waitKey();
    return 0;
}
