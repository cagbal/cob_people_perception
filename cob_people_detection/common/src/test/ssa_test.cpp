
#include<cob_people_detection/subspace_analysis.h>
#include<opencv/cv.h>
#include<opencv/highgui.h>
#include<iostream>


int main(int argc, const char *argv[])
{

  //std::string im1="/share/goa-tz/people_detection/debug/results/3_norm.jpg";
  //std::string im2="/share/goa-tz/people_detection/debug/results/5_norm.jpg";
  //std::string im3="/share/goa-tz/people_detection/debug/results/9_norm.jpg";
  std::string im4="/share/goa-tz/people_detection/debug/results/240.bmp";
  std::string im5="/share/goa-tz/people_detection/debug/results/266.bmp";
  std::string im6="/share/goa-tz/people_detection/debug/results/269.bmp";

  std::vector<std::string> in_vec;
  //in_vec.push_back(im1);
  //in_vec.push_back(im2);
  //in_vec.push_back(im3);
  in_vec.push_back(im4);
  in_vec.push_back(im5);
  in_vec.push_back(im6);

  std::vector<cv::Mat> img_vec;

  for(int i =0;i<in_vec.size();i++)
  {
    cv::Mat img=cv::Mat(120,120,CV_64FC1);
    img =cv::imread(in_vec[i],0);
    
    cv::resize(img,img,cv::Size(120,120));
    img.convertTo(img,CV_64FC1);

    img_vec.push_back(img);
  }


 //for(int i =0;i<3;i++)
 //{
 //  cv::Mat out =img_vec[i];
 //  out.convertTo(out,CV_8U);
 //  cv::imshow("vec",out);
 //  cv::waitKey(500);
 //}
  std::vector<int> label_vec;
  for(int i=0;i<img_vec.size();i++)
  {
  label_vec.push_back(i);
  }
  int ss_dim=img_vec.size()-1;



  //cv::Mat a=img_vec[0];
  //a.clone();
  //std::cout<<a.rows<<" ---- "<<a.cols<<"...."<<a.type()<<std::endl;
  //a=a.reshape(1,0);
  //cv::Mat b=a.reshape(1,0);
  //std::cout<<b.rows<<" ---- "<<b.cols<<"...."<<a.type()<<std::endl;


  SubspaceAnalysis::Eigenfaces EF(img_vec,label_vec,ss_dim);
  return 0;
}
