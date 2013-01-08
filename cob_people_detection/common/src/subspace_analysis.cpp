#include"cob_people_detection/subspace_analysis.h"
#include<opencv/highgui.h>

void SubspaceAnalysis::DFFS(cv::Mat& orig_mat,cv::Mat& recon_mat,cv::Mat& avg,std::vector<double>& DFFS)
{

  cv::Mat temp=cv::Mat(orig_mat.rows,orig_mat.cols,orig_mat.type());
  orig_mat.copyTo(temp);
  DFFS.resize(recon_mat.rows);
  for(int i=0;i<orig_mat.rows;i++)
  {
    cv::Mat recon_row=recon_mat.row(i);
    cv::Mat temp_row=temp.row(i);
    cv::subtract(temp_row,avg,temp_row);
    DFFS[i]= cv::norm(recon_row,temp_row,cv::NORM_L2);
  }
  return;
}
void SubspaceAnalysis::project(cv::Mat& src_mat,cv::Mat& proj_mat,cv::Mat& avg_mat,cv::Mat& coeff_mat)
{

  // reduce im mat by mean
  for(int i=0;i<src_mat.rows;i++)
  {
    cv::Mat im=src_mat.row(i);
    cv::subtract(im,avg_mat.reshape(1,1),im);
  }

  //calculate coefficients

  cv::gemm(src_mat,proj_mat,1.0,cv::Mat(),0.0,coeff_mat);

}

void SubspaceAnalysis::reconstruct(cv::Mat& coeffs,cv::Mat& proj_mat,cv::Mat& avg,cv::Mat& rec_im)
{
  cv::gemm(coeffs,proj_mat,1.0,Mat(),0.0,rec_im,GEMM_2_T);
  for(int i=0;i<rec_im.rows;i++)
  {
    cv::Mat curr_row=rec_im.row(i);
    cv::add(curr_row,avg.reshape(1,1),curr_row);
  }
}

void SubspaceAnalysis::calcDataMat(std::vector<cv::Mat>& input_data,cv::Mat& data_mat)
{

  // convert input to data matrix,with images as rows


    double* src_ptr;
    double*  dst_ptr;
    cv::Mat src_mat;
    for(int i = 0;i<input_data.size();i++)
    {
      input_data[i].copyTo(src_mat);

      src_ptr = src_mat.ptr<double>(0,0);
      //src_ptr = input_data[i].ptr<double>(0,0);
      dst_ptr = data_mat.ptr<double>(i,0);

      for(int j=0;j<input_data[i].rows;j++)
      {
        src_ptr=src_mat.ptr<double>(j,0);
        //src_ptr=input_data[i].ptr<double>(j,0);
        for(int col=0;col<input_data[i].cols;col++)
        {
        *dst_ptr=*src_ptr;
        src_ptr++;
        dst_ptr++;
        }
      }

    }


    //cv::Mat src_mat=cv::Mat(120,120,CV_64FC1);
    //src_mat= input_data[0].clone();
    ////src_mat.convertTo(src_mat,CV_64F);
    //std::cout<<"src"<<src_mat.rows<<","<<src_mat.cols<<std::endl;
    //src_mat=src_mat.reshape(1,1);
    //src_mat.clone();
    //std::cout<<"src"<<src_mat.rows<<","<<src_mat.cols<<std::endl;
    //cv::Mat dst_mat;
    //dst_mat.push_back(src_mat);

    //std::cout<<"dst"<<dst_mat.rows<<","<<dst_mat.cols<<std::endl;
    //dst_mat=dst_mat.reshape(1,120);
    //dst_mat.convertTo(dst_mat,CV_8UC1);
    //cv::imshow("DST",dst_mat);
    //cv::waitKey(0);
    
    

  return;
}

//---------------------------------------------------------------------------------
// EIGENFACES
//---------------------------------------------------------------------------------
//
//
SubspaceAnalysis::Eigenfaces::Eigenfaces(std::vector<cv::Mat>& img_vec,std::vector<int>& label_vec,int& dim_ss)
{
  //check if input has the same size
  if(img_vec.size()!=label_vec.size())
  {
    //TODO: ROS ERROR
    std::cout<<"EIGFACE: input has to be the same sie\n";
    return;
  }

  //initialize all matrices
  model_data_=cv::Mat(img_vec.size(),img_vec[0].total(),CV_64FC1);
  avg_=cv::Mat(1,img_vec[0].total(),CV_64FC1);
  proj_model_data_=cv::Mat(img_vec.size(),dim_ss,CV_64FC1);
  avg_=cv::Mat(dim_ss,img_vec[0].total(),CV_64FC1);


  SubspaceAnalysis::calcDataMat(img_vec,model_data_);

  cv::Mat dummy;
  model_data_.copyTo(dummy);
  dummy.convertTo(dummy,CV_8UC1);
  dummy =dummy.reshape(1,img_vec.size()*120);
  cv::imshow("model_data",dummy);
  cv::imwrite("/home/goa-tz/Desktop/model_data.jpg",dummy);
  cv::waitKey(0);

  //initiate PCA
  pca_=SubspaceAnalysis::PCA(model_data_,dim_ss);
  pca_.eigenvecs.copyTo(proj_);
  pca_.mean.copyTo(avg_);

  std::vector<double> DFFS;
  projectToSubspace(model_data_,proj_model_data_,DFFS);
}



void SubspaceAnalysis::Eigenfaces::projectToSubspace(cv::Mat& src_mat,cv::Mat& dst_mat,std::vector<double>& DFFS)
{
  SubspaceAnalysis::project(src_mat,proj_,avg_,dst_mat);

  cv::Mat rec_mat;
  SubspaceAnalysis::reconstruct(dst_mat,proj_,avg_,rec_mat);

  cv::Mat dummy;

  std::cout<<"RECMAT"<<rec_mat.rows<<","<<rec_mat.cols<<std::endl;
  rec_mat.copyTo(dummy);
  dummy.convertTo(dummy,CV_8UC1);
  dummy =dummy.reshape(1,rec_mat.rows*120);
  cv::imshow("recon",dummy);
  cv::imwrite("/home/goa-tz/Desktop/recon.jpg",dummy);
  cv::waitKey(0);

  SubspaceAnalysis::DFFS(src_mat,rec_mat,avg_,DFFS);
}
void SubspaceAnalysis::Eigenfaces::meanCoeffs(cv::Mat& coeffs,std::vector<int>& label_vec,cv::Mat& mean_coeffs)
{
  std::vector<int> distinct_vec;
  bool unique=true;
  for(int i=0;i<label_vec.size();++i)
  {
    if(i==0)distinct_vec.push_back(label_vec[i]);continue;

    unique=true;
    for(int j=0;j<distinct_vec.size();j++)
    {
      if(label_vec[i]==distinct_vec[j]) unique =false;
    }
    if(unique==true)distinct_vec.push_back(label_vec[i]);
  }
  int num_classes=distinct_vec.size();



  std::vector<cv::Mat> mean_coeffs_mat(num_classes);
  std::vector<int>     samples_per_class(num_classes);

  //initialize vectors with zeros
  for( int i =0;i<num_classes;i++)
  {
   mean_coeffs_mat[i]=cv::Mat::zeros(1,coeffs.cols,CV_64FC1);
   samples_per_class[i]=0;
  }

  int class_index;
  for(int i =0;i<coeffs.rows;i++)
  {
    cv::Mat curr_row=coeffs.row(i);
    class_index=label_vec[i];

    add(mean_coeffs_mat[class_index],curr_row,mean_coeffs_mat[class_index]);
    samples_per_class[class_index]++;
  }

  for (int i = 0; i < num_classes; i++) {
  cv::Mat curr_row=mean_coeffs.row(i);
  mean_coeffs_mat[i].convertTo(curr_row,CV_64FC1,1.0/static_cast<double>(samples_per_class[i]));


  }

}
//TODO:
//provide interface for recosntruction and projection
//with autmatic generation of DFFS and DIFS rspectively
//
//
//SubspaceAnalysis::Fisherfaces::Fisherfaces()
//{
//  //TODO:
//  //initiate PCA
//  //forward truncated data to LDA
//  //get projection matrixw
//}
////TODO:
////provide interface for recosntruction and projection
////with autmatic generation of DFFS and DIFS rspectively
////
////MAKE Sure every matrix is transformed to one image per row matrix"!!!!!!
////



//---------------------------------------------------------------------------------
// SSA
//---------------------------------------------------------------------------------


SubspaceAnalysis::SSA::SSA(cv::Mat& data_mat,int& ss_dim):dimension(ss_dim)
{
  calcDataMatMean(data_mat,mean);
}


void SubspaceAnalysis::SSA::calcDataMatMean(cv::Mat& data,cv::Mat& mean)
{
  cv::Mat row_cum=cv::Mat::zeros(1,data.cols,CV_64FC1);
  for(int i=0;i<data.rows;++i)
  {
    cv::Mat data_row=data.row(i);
    //calculate mean
    cv::add(row_cum,data_row,row_cum);
  }
  row_cum.convertTo(mean,CV_64F,1.0/static_cast<double>(data.rows));

  //mean.convertTo(mean,CV_8UC1);
  //mean=mean.reshape(1,120);
  //cv::imshow("mean",mean);
  //cv::waitKey(0);
  //return;
}


void SubspaceAnalysis::SSA::decompose(cv::Mat& data_mat)
{

  EigenvalueDecomposition evd(data_mat);

  cv::Mat eigenvals_unsorted,eigenvecs_unsorted;
  eigenvals_unsorted=evd.eigenvalues();
  eigenvals.clone().reshape(1,1);
  eigenvecs_unsorted=evd.eigenvectors();

  //sort eigenvals and eigenvecs

  cv::Mat sort_indices;
  cv::sortIdx(eigenvals_unsorted,sort_indices,CV_SORT_DESCENDING);//TODO: are flags just right

  cv::Mat temp;
  for(size_t i=0;i<sort_indices.total();i++)
  {
    cv::Mat eigenvals_curr_col=eigenvals.col(i);
    cv::Mat eigenvecs_curr_col=eigenvecs.col(i);
    temp = eigenvals_unsorted.col(sort_indices.at<int>(i));
    temp.copyTo(eigenvals_curr_col);
    temp = eigenvecs_unsorted.col(sort_indices.at<int>(i));
    temp.copyTo(eigenvecs_curr_col);
  }

  eigenvals=Mat(eigenvals,cv::Range::all(),cv::Range(0,dimension));
  eigenvecs=Mat(eigenvecs,cv::Range::all(),cv::Range(0,dimension));

}

void SubspaceAnalysis::SSA::decompose2(cv::Mat& data_mat)
{
  
  data_mat.convertTo(data_mat,CV_64F,1/sqrt(3));
  cv::SVD svd(data_mat.t());
  svd.u.copyTo(eigenvecs);
  svd.w.copyTo(eigenvals);

  eigenvecs.t();

  cv::Mat dummy;
  std::cout<<"EV:"<<eigenvecs.rows<<","<<eigenvecs.cols<<std::endl;
  eigenvecs.copyTo(dummy);
  dummy.convertTo(dummy,CV_8UC1,1000);
  dummy=dummy.t();
  dummy =dummy.reshape(1,eigenvecs.cols*120);
  cv::equalizeHist(dummy,dummy);
  cv::imshow("eigenfaces",dummy);
  cv::imwrite("/home/goa-tz/Desktop/eigenfaces.jpg",dummy);
  cv::waitKey(0);


}

//---------------------------------------------------------------------------------
// LDA
//---------------------------------------------------------------------------------
SubspaceAnalysis::LDA::LDA(cv::Mat& input_data,std::vector<int>& input_labels,int& ss_dim): SSA(input_data,ss_dim)
{

  //class labels have to be in a vector in ascending order - duplicates are
  //removed internally
  //{0,0,1,2,3,3,4,5}

  calcClassMean(data,input_labels,class_means);
  calcModelMatrix(input_labels,model);
  decompose(model);

}
void SubspaceAnalysis::LDA::calcClassMean(cv::Mat& data_mat,std::vector<int>& label_vec,std::vector<cv::Mat>&  mean_vec)
{

  std::vector<int> distinct_vec;
  bool unique=true;
  for(int i=0;i<label_vec.size();++i)
  {
    if(i==0)distinct_vec.push_back(label_vec[i]);continue;

    unique=true;
    for(int j=0;j<distinct_vec.size();j++)
    {
      if(label_vec[i]==distinct_vec[j]) unique =false;
    }
    if(unique==true)distinct_vec.push_back(label_vec[i]);
  }
  num_classes=distinct_vec.size();



  std::vector<cv::Mat> mean_of_class(num_classes);
  std::vector<int>     samples_per_class(num_classes);

  //initialize vectors with zeros
  for( int i =0;i<num_classes;i++)
  {
   mean_of_class[i]=cv::Mat::zeros(1,data_mat.cols,CV_64FC1);
   samples_per_class[i]=0;
  }

  int class_index;
  for(int i =0;i<data_mat.rows;i++)
  {
    cv::Mat data_row=data_mat.row(i);
    class_index=label_vec[i];

    add(mean_of_class[class_index],data_row,mean_of_class[class_index]);
    samples_per_class[class_index]++;
  }

  for (int i = 0; i < num_classes; i++) {
  mean_of_class[i].convertTo(mean_vec[i],CV_64FC1,1.0/static_cast<double>(samples_per_class[i]));

  }
}

void SubspaceAnalysis::LDA::calcModelMatrix(std::vector<int>& label_vec,cv::Mat& M)
{
 //reduce data matrix with class means and compute inter class scatter
  // inter class scatter
  cv::Mat S_inter=cv::Mat::zeros(data.rows,data.rows,CV_64FC1);
  cv::Mat temp;

  int class_index;
  cv::Mat data_row;
  for(int i=0;i<num_classes;++i)
  {
    //reduce data matrix
    class_index=label_vec[i];
    data_row =data.row(i);
    cv::subtract(data_row,class_means[class_index],data_row);
    //compute interclass scatte
    cv::subtract(class_means[class_index],mean,temp);
    cv::mulTransposed(temp,temp,true);
    cv::add(S_inter,temp,S_inter);
  }

  //intra-class scatter
  cv::Mat S_intra=cv::Mat::zeros(data.rows,data.rows,CV_64FC1);
  mulTransposed(data,S_intra,true);
  cv::Mat S_intra_inv=S_intra.inv();

  gemm(S_intra_inv,S_inter,1.0,cv::Mat(),0.0,M);

  return;

}

//---------------------------------------------------------------------------------
// PCA
//---------------------------------------------------------------------------------
//
SubspaceAnalysis::PCA::PCA(cv::Mat& input_data,int& ss_dim):SSA(input_data,ss_dim)
{
  data=input_data;
  calcProjMatrix(input_data);
  eigenvecs=eigenvecs(cv::Rect(0,0,dimension-1,input_data.cols));


}

void SubspaceAnalysis::PCA::calcProjMatrix(cv::Mat& data)
{

  cv::Mat data_row;
  for(int i=0;i<data.rows;++i)
  {
    //reduce data matrix - total Scatter matrix
    data_row =data.row(i);
    cv::subtract(data_row,mean,data_row);
  }
    decompose2(data);
}


