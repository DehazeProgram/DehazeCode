#ifndef COLORCORRECT_H
#define COLORCORRECT_H

#include<iostream>
#include<vector>
#include<opencv2/opencv.hpp>

class ColorCorrect
{
public:
    ColorCorrect();
    static void AutoColor(cv::Mat &image, float s1, float s2);
    static void ContractEnhancement(cv::Mat &image,float s1,float s2);
    static void GenerateHistogram(std::vector<int> &histo, const cv::Mat &image);
private:
    static void AutoColor_single(cv::Mat &image, float s1, float s2);

};

#endif // COLORCORRECT_H
