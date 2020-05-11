#include "opencv2/opencv.hpp"
using namespace cv;
using namespace std;

/************************************************************************/
/* ˵����
*���������µ�������Ϊ [0,1��...��level-1] ��
*blendMask Ϊͼ�����ģ
*maskGaussianPyramidΪ������ÿһ�����ģ
*resultLapPyr ���ÿ���������ֱ����������ͼLaplacian�任ƴ�ɵ�ͼ��
*/
/************************************************************************/
/* ��ȡ��˹�ֲ�����               (�˴�С�� sigmaֵ) */
float **getGaussianArray(int arr_size, float sigma)
{
	int i, j;
	// [1] ��ʼ��Ȩֵ����
	float **array = new float*[arr_size];
	for (i = 0; i < arr_size; i++) {
		array[i] = new float[arr_size];
	}
	// [2] ��˹�ֲ�����
	int center_i, center_j;
	center_i = center_j = arr_size / 2;
	float pi = 3.141592653589793;
	float sum = 0.0f;
	// [2-1] ��˹����
	for (i = 0; i < arr_size; i++) {
		for (j = 0; j < arr_size; j++) {
			array[i][j] =
				//������й�һ�����ⲿ�ֿ��Բ���
				//0.5f *pi*(sigma*sigma) * 
				exp(-(1.0f)* (((i - center_i)*(i - center_i) + (j - center_j)*(j - center_j)) /
				(2.0f*sigma*sigma)));
			sum += array[i][j];
		}
	}
	// [2-2] ��һ����Ȩֵ
	for (i = 0; i < arr_size; i++) {
		for (j = 0; j < arr_size; j++) {
			array[i][j] /= sum;
			printf(" [%.15f] ", array[i][j]);
		}
		printf("\n");
	}
	return array;
}
/* ��˹�˲� (������ͨ��ͼƬ, ��˹�ֲ����飬 ��˹�����С(�˴�С) ) */
void gaussian(Mat *_src, float **_array, int _size)
{
	cv::Mat temp = (*_src).clone();
	// [1] ɨ��
	for (int i = 0; i < (*_src).rows; i++) {
		for (int j = 0; j < (*_src).cols; j++) {
			// [2] ���Ա�Ե
			if (i >(_size / 2)-1 && j >(_size / 2)-1 &&
				i < (*_src).rows - (_size / 2)+1 && j < (*_src).cols - (_size / 2)+1) 
			{
				// [3] �ҵ�ͼ�������f(i,j),�������Ϊ����������Ķ���
				//     ����Ϊ���Ĳο��� �������=>��˹����180��ת�����
				//     x y �������˵�Ȩֵ����   i j ����ͼ�����������
				//     �������     (f*g)(i,j) = f(i-k,j-l)g(k,l)          f����ͼ������ g�����
				//     ����˲ο��� (f*g)(i,j) = f(i-(k-ai), j-(l-aj))g(k,l)   ai,aj �˲ο���
				//     ��Ȩ���  ע�⣺�˵�����������0,0���
				float sum = 0.0;
				for (int k = 0; k < _size; k++) 
				{
					for (int l = 0; l < _size; l++) 
					{
						sum += (*_src).ptr<float>(i - k + (_size / 2))[j - l + (_size / 2)] * _array[k][l];
					}
				}
				// �����м���,�������õ�ֵ��û�м����ֵ���ܻ���
				temp.ptr<float>(i)[j] = sum;
			}
		}
	}

	// ����ԭͼ
	(*_src) = temp.clone();
}
void myGaussianFilter(Mat *src, Mat *dst, int n, float sigma)
{
	// [1] ��ʼ��
	*dst = (*src).clone();
	// [2] ��ɫͼƬͨ������
	vector<Mat> channels;
	split(*src, channels);
	// [3] �˲�
	// [3-1] ȷ����˹��̬����
	float **array = getGaussianArray(n, sigma);
	// [3-2] ��˹�˲�����
	for (int i = 0; i < 3; i++) {
		gaussian(&channels[i], array, n);
	}
//		gaussian(dst, array, n);
	// [4] �ϲ�����
	merge(channels, *dst);
}


Mat myDown(Mat& img) {
	Mat gaussian;
	Mat dst1 = Mat((img.cols+1) / 2, (img.rows+1) / 2, img.type());
	myGaussianFilter(&img, &gaussian, 5, 1.5f);

	//��С1/2
	Mat dst;
	for (int i = 0; i<gaussian.rows; i++)
	{
		if (i % 2 != 0) //��i�в�����Ҫɾ����
		{
			dst.push_back(gaussian.row(i)); //��message�ĵ�i�мӵ�dst����ĺ���
		}
	}
	int n = 0;
	for (int i = 0; i<gaussian.cols; i++)
	{
		if (i % 2 != 0) //��i�в�����Ҫɾ����
		{
			dst.col(i).copyTo(dst1.col(n)); //��message�ĵ�i�мӵ�dst����ĺ���
			n++;
		}

	}
	return dst1;
}
Mat myUp(Mat& img)
{
//***********����2��
		Mat dst1 = Mat(img.rows * 2, img.cols * 2, CV_32FC3);
		Mat dst2 = Mat(img.rows * 2, img.cols, CV_32FC3);
		int m = 0;
		for (int i = 0; i < dst2.rows; i++)
		{
			if (i % 2 == 0) //��i�в�����Ҫɾ����
			{
				img.row(m).copyTo(dst2.row(i)); //��message�ĵ�i�мӵ�dst����ĺ���
				m++;
			}
			else
				dst2.row(i) = 0.0;
		}
		int n = 0;
		for (int i = 0; i < dst1.cols; i++)
		{
			if (i % 2 == 0) //��i�в�����Ҫɾ����
			{
				dst2.col(n).copyTo(dst1.col(i)); //��message�ĵ�i�мӵ�dst����ĺ���
				n++;
			}
			else
				dst1.row(i) = 0.0;
		}
		// [3-2] ��˹�˲�����
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
	Mat_<Vec3f> l; l8u.convertTo(l, CV_32F, 1.0 / 255.0);//Vec3f��ʾ������ͨ������ l[row][column][depth]
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
//����������Ϊÿһ�����ģ maskgaussienpyramid 
	assert(leftLapPyr.size()>0);

	maskGaussianPyramid.clear();
//	Mat currentImgg;
	Mat_<Vec3f> currentImgg;
//	cvtColor(mymask, currentImgg, CV_GRAY2BGR);//store color img of blend mask into maskGaussianPyramid
	maskGaussianPyramid.push_back(m1); //0-level��maskGaussianPyramid����+currentImgg
	currentImgg = m1;

	for (int l = 1; l<levels + 1; l++) {
		Mat_<Vec3f> _downg;
		if (leftLapPyr.size() > l)
			_downg= myDown(currentImgg);//input:currentImg out:down, �²������ͼƬ��С 
		else
			_downg= myDown(currentImgg); //lowest level

//		Mat downg;
//		cvtColor(_downg, downg, CV_GRAY2BGR);
		maskGaussianPyramid.push_back(_downg);//add color blend mask into mask Pyramid
		currentImgg = _downg;
	}

//*************blendLapPyrs();	//blend left & right Pyramids into one Pyramid
//���ÿ���������ֱ����������ͼLaplacian�任ƴ�ɵ�ͼ��resultLapPyr
	resultHighestLevel = leftHighestLevel.mul(maskGaussianPyramid.back()) +
		rightHighestLevel.mul(Scalar(1.0, 1.0, 1.0) - maskGaussianPyramid.back());
	for (int l = 0; l<levels; l++) {
		Mat A = leftLapPyr[l].mul(maskGaussianPyramid[l]); //mul:�����Ӧλ�ĳ˻� 
		Mat antiMask = Scalar(1.0, 1.0, 1.0) - maskGaussianPyramid[l];
		Mat B = rightLapPyr[l].mul(antiMask);
		Mat_<Vec3f> blendedLevel = A + B;

		resultLapPyr.push_back(blendedLevel);
	}


//***************************************reconstructImgFromLapPyramid();
	//������laplacianͼ��ƴ�ɵ�resultLapPyr��������ÿһ��
	//���ϵ��²�ֵ�Ŵ���ӣ�����blendͼ����
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
