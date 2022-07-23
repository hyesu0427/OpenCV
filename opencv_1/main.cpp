#include "opencv2/opencv.hpp"
#include <iostream>
#include <string>

using namespace cv;
using namespace std;

int main(int, char**)
{
	Mat img_input, img_result, img_gray, img_dilate, img_erode;
	int re = 0;

	Mat frame; // OpenCV에서 가장 기본이 되는 Matrix 구조체(이미지를 읽어 해당 정보를 Mat형태로 변환)
	VideoCapture cap(0, CAP_DSHOW); // 동영상 불러오기
	cap.set(CAP_PROP_FRAME_WIDTH, 1920);
	cap.set(CAP_PROP_FRAME_HEIGHT, 1080);
	cap.open(0); // 동영상 열기(Camera 열기) + 카메라번호(0(내장 우선))
	int fps = (int)cap.get(CAP_PROP_FPS); //초당 프레임 수

	if (!cap.isOpened())
	{
		cout << "Error! Cannot open the camera" << endl;
		return -1;
	}

	for (;;)
	{
		cap.read(img_input);
		// check if we succeeded
		if (img_input.empty()) {
			cerr << "ERROR! blank frame grabbed\n";
			break;
		}
		//그레이스케일 이미지로 변환
		cvtColor(img_input, img_gray, COLOR_BGR2GRAY);
		GaussianBlur(img_gray, img_gray, Size(3, 3), 0, 0);

		//이진화 이미지로 변환
		Mat binary_image;
		threshold(img_gray, img_gray, 200, 255, THRESH_BINARY);

		//노이즈 제거 
		dilate(img_gray, img_dilate, Mat::ones(Size(3, 3), CV_8UC1), Point(-1, -1));
		erode(img_dilate, img_erode, Mat::ones(Size(3, 3), CV_8UC1), Point(-1, -1));

		/*erode(img_gray, img_erode, Mat::ones(Size(3, 3), CV_8UC1), Point(-1, -1));
		dilate(img_erode, img_dilate, Mat::ones(Size(3, 3), CV_8UC1), Point(-1, -1));*/

		//색 채우기
		Mat im_floodfill = img_erode.clone();
		floodFill(im_floodfill, Point(0, 0), Scalar(255));

		Mat im_floodfill_inv;
		bitwise_not(im_floodfill, im_floodfill_inv);

		Mat im_out = (img_erode | im_floodfill_inv);

		//contour정보를 출력한다. (흑백,이진화 된 이미지만 적용)
		vector<vector<Point> > contours;
		findContours(im_out, contours, RETR_TREE, CHAIN_APPROX_SIMPLE);

		//contour를 근사화한다.
		vector<Point2f> approx;
		img_result = img_input.clone();

		imshow("im_out", im_out);

		for (size_t i = 0; i < contours.size(); i++)
		{//검출한 형상(외각선)을 표현하기 위해 꼭지점 찾아서 선 연결하기
			approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true) * 0.01, true);

			if (fabs(contourArea(Mat(approx))) > 1000)  //면적이 일정크기 이상이어야 한다.
			{
				int size = approx.size();

				////Contour를 근사화한 직선을 그린다.
				if (size % 2 == 0) {
					line(img_result, approx[0], approx[approx.size() - 1], Scalar(0, 255, 0), 3);
					
					for (int k = 0; k < size - 1; k++)
						line(img_result, approx[k], approx[k + 1], Scalar(0, 255, 0), 3);

					//점 표시 원 그리기
					for (int k = 0; k < size; k++)
						circle(img_result, approx[k], 3, Scalar(0, 0, 255));
				}
				if (size > 5)
				{
					re = 1;    //사각형이면 ++
				 //setLabel(img_result, "rectangle", contours[i]); //사각형 글씨로 표시
				}
				else
					re = 0;
			}
		}//&& isContourConvex(Mat(approx))
		imshow("result", img_result);
		cout << "상자 유무 : " << re << endl;
		if (waitKey(5) == 27) //ESC 키 누를 때 까지 대기
			break;
	
	}
	return 0;
}