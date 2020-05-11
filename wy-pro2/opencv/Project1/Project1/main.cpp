#include "opencv2/opencv.hpp"
using namespace cv;
using namespace std;

/************************************************************************/
/* 说明：
*金字塔从下到上依次为 [0,1，...，level-1] 层
*blendMask 为图像的掩模
*maskGaussianPyramid为金字塔每一层的掩模
*resultLapPyr 存放每层金字塔中直接用左右两图Laplacian变换拼成的图像
*/
/************************************************************************/
/* 获取高斯分布数组               (核大小， sigma值) */
float **getGaussianArray(int arr_size, float sigma)
{
	int i, j;
	// [1] 初始化权值数组
	float **array = new float*[arr_size];
	for (i = 0; i < arr_size; i++) {
		array[i] = new float[arr_size];
	}
	// [2] 高斯分布计算
	int center_i, center_j;
	center_i = center_j = arr_size / 2;
	float pi = 3.141592653589793;
	float sum = 0.0f;
	// [2-1] 高斯函数
	for (i = 0; i < arr_size; i++) {
		for (j = 0; j < arr_size; j++) {
			array[i][j] =
				//后面进行归一化，这部分可以不用
				//0.5f *pi*(sigma*sigma) * 
				exp(-(1.0f)* (((i - center_i)*(i - center_i) + (j - center_j)*(j - center_j)) /
				(2.0f*sigma*sigma)));
			sum += array[i][j];
		}
	}
	// [2-2] 归一化求权值
	for (i = 0; i < arr_size; i++) {
		for (j = 0; j < arr_size; j++) {
			array[i][j] /= sum;
			printf(" [%.15f] ", array[i][j]);
		}
		printf("\n");
	}
	return array;
}
/* 高斯滤波 (待处理单通道图片, 高斯分布数组， 高斯数组大小(核大小) ) */
void gaussian(Mat *_src, float **_array, int _size)
{
	cv::Mat temp = (*_src).clone();
	// [1] 扫描
	for (int i = 0; i < (*_src).rows; i++) {
		for (int j = 0; j < (*_src).cols; j++) {
			// [2] 忽略边缘
			if (i >(_size / 2)-1 && j >(_size / 2)-1 &&
				i < (*_src).rows - (_size / 2)+1 && j < (*_src).cols - (_size / 2)+1) 
			{
				// [3] 找到图像输入点f(i,j),以输入点为中心与核中心对齐
				//     核心为中心参考点 卷积算子=>高斯矩阵180度转向计算
				//     x y 代表卷积核的权值坐标   i j 代表图像输入点坐标
				//     卷积算子     (f*g)(i,j) = f(i-k,j-l)g(k,l)          f代表图像输入 g代表核
				//     带入核参考点 (f*g)(i,j) = f(i-(k-ai), j-(l-aj))g(k,l)   ai,aj 核参考点
				//     加权求和  注意：核的坐标以左上0,0起点
				float sum = 0.0;
				for (int k = 0; k < _size; k++) 
				{
					for (int l = 0; l < _size; l++) 
					{
						sum += (*_src).ptr<float>(i - k + (_size / 2))[j - l + (_size / 2)] * _array[k][l];
					}
				}
				// 放入中间结果,计算所得的值与没有计算的值不能混用
				temp.ptr<float>(i)[j] = sum;
			}
		}
	}

	// 放入原图
	(*_src) = temp.clone();
}
void myGaussianFilter(Mat *src, Mat *dst, int n, float sigma)
{
	// [1] 初始化
	*dst = (*src).clone();
	// [2] 彩色图片通道分离
	vector<Mat> channels;
	split(*src, channels);
	// [3] 滤波
	// [3-1] 确定高斯正态矩阵
	float **array = getGaussianArray(n, sigma);
	// [3-2] 高斯滤波处理
	for (int i = 0; i < 3; i++) {
		gaussian(&channels[i], array, n);
	}
//		gaussian(dst, array, n);
	// [4] 合并返回
	merge(channels, *dst);
}


Mat myDown(Mat& img) {
	Mat gaussian;
	Mat dst1 = Mat((img.cols+1) / 2, (img.rows+1) / 2, img.type());
	myGaussianFilter(&img, &gaussian, 5, 1.5f);

	//缩小1/2
	Mat dst;
	for (int i = 0; i<gaussian.rows; i++)
	{
		if (i % 2 != 0) //第i行不是需要删除的
		{
			dst.push_back(gaussian.row(i)); //把message的第i行加到dst矩阵的后面
		}
	}
	int n = 0;
	for (int i = 0; i<gaussian.cols; i++)
	{
		if (i % 2 != 0) //第i行不是需要删除的
		{
			dst.col(i).copyTo(dst1.col(n)); //把message的第i行加到dst矩阵的后面
			n++;
		}

	}
	return dst1;
}
Mat myUp(Mat& img)
{
//***********扩大2倍
		Mat dst1 = Mat(img.rows * 2, img.cols * 2, CV_32FC3);
		Mat dst2 = Mat(img.rows * 2, img.cols, CV_32FC3);
		int m = 0;
		for (int i = 0; i < dst2.rows; i++)
		{
			if (i % 2 == 0) //第i行不是需要删除的
			{
				img.row(m).copyTo(dst2.row(i)); //把message的第i行加到dst矩阵的后面
				m++;
			}
			else
				dst2.row(i) = 0.0;
		}
		int n = 0;
		for (int i = 0; i < dst1.cols; i++)
		{
			if (i % 2 == 0) //第i行不是需要删除的
			{
				dst2.col(n).copyTo(dst1.col(i)); //把message的第i行加到dst矩阵的后面
				n++;
			}
			else
				dst1.row(i) = 0.0;
		}
		// [3-2] 高斯滤波处理
		Mat gaussian;
		myGaussianFilter(&dst1, &gaussian, 5, 1.5f);
		return gaussian*4;
}



void buildLaplacianPyramid(Mat& img, vector<Mat_<Vec3f> >& lapPyr, Mat& HighestLevel,int levels) {
	lapPyr.clear();
	Mat currentImg = img;
	for (int l = 0; l<levels; l++) {
		
		Mat down=myDown(currentImg);
//		Mat down;  pyrDown(currentImg,down);
		Mat up;  pyrUp(down, up,currentImg.size());
//		Mat up0 = myUp(down);

		Mat lap = currentImg - up;
		lapPyr.push_back(lap);
		currentImg = down;
	}
	currentImg.copyTo(HighestLevel);
}



int main() {
	Mat l8u = imread("myface3.jpg");
	Mat r8u = imread("myhand2.jpg");
	Mat mymask = imread("mymask.png");
	Mat_<Vec3f> l; l8u.convertTo(l, CV_32F, 1.0 / 255.0);//Vec3f表示有三个通道，即 l[row][column][depth]
	Mat_<Vec3f> r; r8u.convertTo(r, CV_32F, 1.0 / 255.0);
	Mat_<Vec3f> m1; mymask.convertTo(m1, CV_32F, 1.0 / 255.0);


	imshow("myface", l8u);
	imshow("myhand", r8u);
	imshow("m", m1);
	vector<Mat_<Vec3f> > leftLapPyr, rightLapPyr, resultLapPyr;//Laplacian Pyramids
	Mat leftHighestLevel, rightHighestLevel, resultHighestLevel;
	vector<Mat_<Vec3f> > maskGaussianPyramid; //masks are 3-channels for easier multiplication with RGB
	int levels=4;



	//********************************************************
	//***********LaplacianBlending lb(l, r, m, 4);
	assert(l.size() == r.size());
	assert(l.size() == mymask.size());
//**************buildPyramids();	//construct Laplacian Pyramid and Gaussian Pyramid 
  	buildLaplacianPyramid(l, leftLapPyr, leftHighestLevel,levels);
	buildLaplacianPyramid(r, rightLapPyr, rightHighestLevel,levels);
//***********************buildGaussianPyramid();
//金字塔内容为每一层的掩模 maskgaussienpyramid 
	assert(leftLapPyr.size()>0);

	maskGaussianPyramid.clear();
//	Mat currentImgg;
	Mat_<Vec3f> currentImgg;
//	cvtColor(mymask, currentImgg, CV_GRAY2BGR);//store color img of blend mask into maskGaussianPyramid
	maskGaussianPyramid.push_back(m1); //0-level在maskGaussianPyramid后面+currentImgg
	currentImgg = m1;

	for (int l = 1; l<levels + 1; l++) {
		Mat_<Vec3f> _downg;
		if (leftLapPyr.size() > l)
			_downg= myDown(currentImgg);//input:currentImg out:down, 下采样后的图片大小 
		else
			_downg= myDown(currentImgg); //lowest level

//		Mat downg;
//		cvtColor(_downg, downg, CV_GRAY2BGR);
		maskGaussianPyramid.push_back(_downg);//add color blend mask into mask Pyramid
		currentImgg = _downg;
	}

//*************blendLapPyrs();	//blend left & right Pyramids into one Pyramid
//获得每层金字塔中直接用左右两图Laplacian变换拼成的图像resultLapPyr
	resultHighestLevel = leftHighestLevel.mul(maskGaussianPyramid.back()) +
		rightHighestLevel.mul(Scalar(1.0, 1.0, 1.0) - maskGaussianPyramid.back());
	for (int l = 0; l<levels; l++) {
		Mat A = leftLapPyr[l].mul(maskGaussianPyramid[l]); //mul:矩阵对应位的乘积 
		Mat antiMask = Scalar(1.0, 1.0, 1.0) - maskGaussianPyramid[l];
		Mat B = rightLapPyr[l].mul(antiMask);
		Mat_<Vec3f> blendedLevel = A + B;

		resultLapPyr.push_back(blendedLevel);
	}


//***************************************reconstructImgFromLapPyramid();
	//将左右laplacian图像拼成的resultLapPyr金字塔中每一层
	//从上到下插值放大并相加，即得blend图像结果
	Mat currentImg = resultHighestLevel;
	for (int l = levels - 1; l >= 0; l--) {
		Mat up=myUp(currentImg);
//		Mat up; pyrUp(currentImg, up);
		imshow("laplacian", up);
		waitKey(0);
		currentImg = up + resultLapPyr[l];  //??????????????????????????????????????????????/ 
	}
	Mat_<Vec3f> blend = currentImg;



	imshow("blended", blend);
	waitKey(0);
	return 0;
}
