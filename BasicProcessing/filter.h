#ifndef FILTER_H
#define FILTER_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <deque>
#include"time.h"

struct point
{
    point(int a,int b):x(a),y(b){}
    int x;
    int y;
};

class Filter
{
public:
    Filter();
    static void DarkImageFilter(cv::Mat &src, int windowsize, cv::Mat &dst);
//    void guideFilter(cv::Mat &guideI, cv::Mat &inputI, cv::Mat &dst, int radius, float eps);
    static void GuideFilter_Single(cv::Mat& guidedImage, cv::Mat& source,cv::Mat& output,int radius, float epsilon);
    static void GuideFilter_Multi(std::vector< cv::Mat >& guidedImages, cv::Mat& sourse, cv::Mat& output, int radius, float epsilon);
    template<typename T>
    static void MaxFilter_1D(std::vector<T> &vec, std::vector<T> &dst, int windowsize, std::deque<int> &maxs=std::deque<int>());
    static void MaxFilter_2D(cv::Mat &src,cv::Mat &dst,int windowsize);
    static void MinFilter_2D(cv::Mat &src,cv::Mat &dst,int windowsize);
private:
    template<typename T>
    static int max(const T& a,const T& b){
        if(a>b)
            return a;
        return b;
    }

    template<typename T>
    static int min(const T& a,const T& b){
        if(a>b)
            return b;
        return a;
    }
    template<typename T>
    static void MinFilter_1D(std::vector<T> &vec, std::vector<T>&dst,int windowsize,std::deque<int> &mins=std::deque<int>());
//    static void MinFilter_2D(cv::Mat &src,cv::Mat &dst,int windowsize);
    static void MakeDepth32f(cv::Mat& source, cv::Mat& output);

};

#endif // FILTER_H
