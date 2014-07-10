#include "filter.h"


Filter::Filter()
{}

//minFilter
void Filter::DarkImageFilter(cv::Mat& src, int windowsize, cv::Mat& dst)
{
    std::vector<cv::Mat> channels;
    cv::split(src,channels);
    int row =src.rows;
    int col =src.cols;
    cv::Mat img = cv::Mat(row,col,CV_8UC1);
    if(channels[0].isContinuous() && channels[1].isContinuous()&&channels[2].isContinuous())
    {
        col = row*col;
        row = 1;
    }


    for(int i = 0;i < row; ++i)
    {
        for(int j =0;j <col;++j)
        {
            uchar p0 = channels[0].at<uchar>(i,j);
            uchar p1 = channels[1].at<uchar>(i,j);
            uchar p2 = channels[2].at<uchar>(i,j);

            uchar temp = ((p0<p1)?p0:p1);
            img.at<uchar>(i,j) = ((temp<p2)?temp:p2);
        }
    }
    std::cout <<"MINFILTER START"<<std::endl;
    time_t startDark = clock();

    MinFilter_2D(img,dst,windowsize);
    std::cout <<"cost: "<<clock()- startDark<<"ms"<<std::endl;

}

void Filter::MinFilter_2D(cv::Mat &src, cv::Mat &dst, int windowsize)
{
    CV_Assert(src.rows == dst.rows && src.cols ==dst.cols);
    int r = windowsize/2;
    cv::Mat src_border;
    cv::copyMakeBorder(src,src_border,r,r,r,r,src.type(),cv::Scalar(255));
    std::deque< std::deque<point> > col_mins;
    int row =src_border.rows;
    int col =src_border.cols;

    /*********init col_mins************/
    for(int j =0;j <col;++j)
    {
        std::vector<uchar> vec,dst;
        std::deque<int> tmins;
        std::deque<point> cmins;
        for(int i=0;i <=2*r;++i)
            vec.push_back(src_border.at<uchar>(i,j));
        Filter::MinFilter_1D(vec,dst,vec.size(),tmins);
        if(tmins.size() ==0)
            cmins.push_back((point(2*r,j)));
        else{
            while(tmins.size()>0)
            {
//                std::cout <<"ha"<<std::endl;
                int i =tmins.front();
                cmins.push_back(point(i,j));
                tmins.pop_front();
            }
        }
        col_mins.push_back(cmins);
    }

    /***************compute mins *******************/
    for(int i=r;i<row-r;++i)
    {
        int i_max =i+r;
        int i_min =i-r;
        std::vector<uchar> vec,col_dst;

        for(int a=0;a< col_mins.size();++a)
        {
            std::deque<point> v =col_mins[a];
            point p =v[0];
            vec.push_back(src_border.at<uchar>(p.x,p.y));
        }

        std::deque<int> tmins;
        Filter::MinFilter_1D(vec,col_dst,windowsize,tmins);
        for(int j=r, a=0;j<col-r, a<col_dst.size();++j, ++a)
        {
            dst.at<uchar>(i-r,j-r) =col_dst[a];
        }
        //move col_mins to next line
        if(i ==row-r-1)
            break;

        for(int j =0;j< col;++j)
        {
            if(src_border.at<uchar>(i_max+1,j) >src_border.at<uchar>(i_max,j))
            {
                col_mins[j].push_back(point(i_max,j));
                if(col_mins[j].front().x <i_min)
                    col_mins[j].pop_front();
                if(col_mins[j].size() ==0)
                {
                    col_mins[j].push_back(point(i_max+1,j));
                }
            }
            else
            {
                while(col_mins[j].size()>0)
                {
                    if(src_border.at<uchar>(i_max+1,j)>=
                            src_border.at<uchar>(col_mins[j].back().x,col_mins[j].back().y))
                    {
                        if(col_mins[j].front().x <i_min)
                            col_mins[j].pop_front();
                        break;
                    }
                    col_mins[j].pop_back();
                }
                if(col_mins[j].size() ==0)
                {
                    col_mins[j].push_back(point(i_max+1,j));
                }
            }
        }
    }
}






template<typename T>
void Filter::MinFilter_1D(std::vector<T> &vec, std::vector<T>&dst,int windowsize,std::deque<int> &mins)
{
    int anchor=0;
    dst.resize(vec.size()-windowsize+1);
    mins.clear();
    //    std::cout <<dst.size()<<std::endl;
    for(int i = 1;i < vec.size();++i)
    {
        if(i >=windowsize)
            dst[i-windowsize] =vec[mins.size()>0 ? mins.front():(i-1)];
        if(vec[i] > vec[i-1])
        {
            mins.push_back(i-1);
        }
        else
        {
            while( mins.size() >0)
            {
                if(vec[i] >= vec[mins.back()])
                    break;
                mins.pop_back();
            }
        }
        while(mins.size() >0){
            if( i == windowsize +mins.front()) mins.pop_front();
            else break;
        }
    }
    anchor =(mins.size() >0? mins.front(): vec.size() -1);
    dst[vec.size() -windowsize] = vec[anchor];

}

//minFilter


void Filter::MakeDepth32f(cv::Mat& source, cv::Mat& output)
{
    if ((source.depth() != CV_32F ) > FLT_EPSILON)
        source.convertTo(output, CV_32F);
    else
        output = source;
}


void Filter::GuideFilter_Single( cv::Mat& guidedImage, cv::Mat& source,cv::Mat& output,int radius, float epsilon)
{

    CV_Assert(radius >= 2 && epsilon > 0);
    CV_Assert(source.data != NULL && source.channels() == 1);
    CV_Assert(guidedImage.channels() == 1);
    CV_Assert(source.rows == guidedImage.rows && source.cols == guidedImage.cols);

    cv::Mat guided;
    if (guidedImage.data == source.data)
    {
        //make a copy
        guidedImage.copyTo(guided);
    }
    else
    {
        guided = guidedImage;
    }

    //将输入扩展为32位浮点型，以便以后做乘法
    cv::Mat source_32f, guided_32f;
    MakeDepth32f(source, source_32f);
    MakeDepth32f(guided, guided_32f);

    //计算I*p和I*I
    cv::Mat mat_Ip, mat_I2;
    cv::multiply(guided_32f, source_32f, mat_Ip);
    cv::multiply(guided_32f, guided_32f, mat_I2);

    //计算各种均值
    cv::Mat mean_p, mean_I, mean_Ip, mean_I2;
    cv::Size win_size(2*radius + 1, 2*radius + 1);
    cv::boxFilter(source_32f, mean_p, CV_32F, win_size);
    cv::boxFilter(guided_32f, mean_I, CV_32F, win_size);
    cv::boxFilter(mat_Ip, mean_Ip, CV_32F, win_size);
    cv::boxFilter(mat_I2, mean_I2, CV_32F, win_size);

    //计算Ip的协方差和I的方差
    cv::Mat cov_Ip = mean_Ip - mean_I.mul(mean_p);
    cv::Mat var_I = mean_I2 - mean_I.mul(mean_I);
    var_I += epsilon;

    //求a和b
    cv::Mat a, b;
    divide(cov_Ip, var_I, a);
    b = mean_p - a.mul(mean_I);

    //对包含像素i的所有a、b做平均
    cv::Mat mean_a, mean_b;
    cv::boxFilter(a, mean_a, CV_32F, win_size);
    cv::boxFilter(b, mean_b, CV_32F, win_size);

    //计算输出 (depth == CV_32F)
    output = mean_a.mul(guided_32f) + mean_b;
}



void Filter::GuideFilter_Multi(std::vector< cv::Mat > &guidedImages, cv::Mat &source,
                               cv::Mat &output, int radius, float epsilon)
{
    CV_Assert(radius >= 2 && epsilon > 0);
    CV_Assert(source.data != NULL && source.channels() == 1);
    CV_Assert(guidedImages.size() == 3);
    CV_Assert((source.rows == guidedImages[0].rows)&&(source.cols == guidedImages[0].cols));
    CV_Assert((source.rows == guidedImages[1].rows)&&(source.cols == guidedImages[1].cols));
    CV_Assert((source.rows == guidedImages[2].rows)&&(source.cols == guidedImages[2].cols));
    //    std::cout <<"source.rows: "<<source.rows<<std::endl;
    //    std::cout <<"source.cols: "<<source.cols<<std::endl;

    int col =source.cols;
    int row =source.rows;



    //convertTo 32f
    cv::Mat guidedImage_b_32f,guidedImage_g_32f,guidedImage_r_32f,source_32f;
    MakeDepth32f(guidedImages[0],guidedImage_b_32f);
    MakeDepth32f(guidedImages[1],guidedImage_g_32f);
    MakeDepth32f(guidedImages[2],guidedImage_r_32f);
    MakeDepth32f(source,source_32f);

    //compute mat multiply

    cv::Mat I1I1,I2I2,I3I3,I1I2,I1I3,I2I3;
    cv::Mat I1P,I2P,I3P;

    cv::multiply(guidedImage_b_32f,guidedImage_b_32f,I1I1);
    cv::multiply(guidedImage_g_32f,guidedImage_g_32f,I2I2);
    cv::multiply(guidedImage_r_32f,guidedImage_r_32f,I3I3);
    cv::multiply(guidedImage_b_32f,guidedImage_g_32f,I1I2);
    cv::multiply(guidedImage_b_32f,guidedImage_r_32f,I1I3);
    cv::multiply(guidedImage_g_32f,guidedImage_r_32f,I2I3);

    cv::multiply(guidedImage_b_32f,source_32f,I1P);
    cv::multiply(guidedImage_g_32f,source_32f,I2P);
    cv::multiply(guidedImage_r_32f,source_32f,I3P);

    //compute means
    cv::Size win_size(2*radius + 1, 2*radius + 1);

    cv::Mat meanI1,meanI2,meanI3,meanP;
    cv::Mat corrI1,corrI2,corrI3,corrI1I2,corrI1I3,corrI2I3,corrI1P,corrI2P,corrI3P;

    cv::boxFilter(guidedImage_b_32f,meanI1,CV_32F,win_size);
    cv::boxFilter(guidedImage_g_32f,meanI2,CV_32F,win_size);
    cv::boxFilter(guidedImage_r_32f,meanI3,CV_32F,win_size);
    cv::boxFilter(source_32f,meanP,CV_32F,win_size);

    cv::boxFilter(I1I1,corrI1,CV_32F,win_size);
    cv::boxFilter(I2I2,corrI2,CV_32F,win_size);
    cv::boxFilter(I3I3,corrI3,CV_32F,win_size);

    cv::boxFilter(I1I2,corrI1I2,CV_32F,win_size);
    cv::boxFilter(I1I3,corrI1I3,CV_32F,win_size);
    cv::boxFilter(I2I3,corrI2I3,CV_32F,win_size);

    cv::boxFilter(I1P,corrI1P,CV_32F,win_size);
    cv::boxFilter(I2P,corrI2P,CV_32F,win_size);
    cv::boxFilter(I3P,corrI3P,CV_32F,win_size);

    //compute cov var
    cv::Mat varI1,varI2,varI3,covI1I2,covI2I3,covI1I3;

    varI1 = corrI1 - meanI1.mul(meanI1);
    varI2 = corrI2 - meanI2.mul(meanI2);
    varI3 = corrI3 - meanI3.mul(meanI3);

    covI1I2 = corrI1I2 - meanI1.mul(meanI2);
    covI1I3 = corrI1I3 - meanI1.mul(meanI3);
    covI2I3 = corrI2I3 - meanI2.mul(meanI3);

    //compute covIP
    cv::Mat covIP;
    std::vector< cv::Mat > covIPLayers;
    covIPLayers.push_back(corrI1P -meanI1.mul(meanP));
    covIPLayers.push_back(corrI2P -meanI2.mul(meanP));
    covIPLayers.push_back(corrI3P -meanI3.mul(meanP));
    cv::merge(covIPLayers,covIP);
    //    cv::imshow("covIP",covIP);

    //compute varI multiply covIP(compute a and b)
    cv::Mat a(row,col,CV_32FC3);
    cv::Mat b(row,col,CV_32FC1);
    for(int i =0;i< row;++i)
    {
        for(int j =0;j< col;++j)
        {

            cv::Vec3f vec = covIP.at<cv::Vec3f>(i,j);
            cv::Mat varI =(cv::Mat_<float>(3,3) <<varI1.at<float>(i,j)-epsilon,covI1I2.at<float>(i,j),covI1I3.at<float>(i,j),
                           covI1I2.at<float>(i,j),varI2.at<float>(i,j)-epsilon,covI2I3.at<float>(i,j),
                           covI1I3.at<float>(i,j),covI2I3.at<float>(i,j),varI3.at<float>(i,j))-epsilon;
            //            std::cout <<varI<<std::endl;
            cv::invert(varI,varI);
            cv::Vec3f temp;
            cv::gemm(varI,vec,1,NULL,0,temp);
            a.at<cv::Vec3f>(i,j) =temp;
            b.at<float>(i,j) = meanP.at<float>(i,j) -
                    (temp[0]*meanI1.at<float>(i,j)+temp[1]*meanI2.at<float>(i,j)+temp[2]*meanI3.at<float>(i,j));

        }
    }
    // compute output image
    cv::Mat meanB,meanA1,meanA2,meanA3;
    //    cv::Mat tempOutput(row,col,CV_32F);
    std::vector< cv::Mat > a_Layers;
    cv::split(a,a_Layers);
    cv::boxFilter(a_Layers[0],meanA1,CV_32F,win_size);
    cv::boxFilter(a_Layers[1],meanA2,CV_32F,win_size);
    cv::boxFilter(a_Layers[2],meanA3,CV_32F,win_size);
    cv::boxFilter(b,meanB,CV_32F,win_size);
    for(int i =0;i< row;++i)
    {
        for(int j =0;j< col;++j)
        {
            output.at<float>(i,j) = meanA1.at<float>(i,j)*guidedImage_b_32f.at<float>(i,j)+
                    meanA2.at<float>(i,j)*guidedImage_g_32f.at<float>(i,j)+
                    meanA3.at<float>(i,j)*guidedImage_r_32f.at<float>(i,j)+
                    meanB.at<float>(i,j);
        }
    }
}


