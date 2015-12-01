#ifndef __VIEW_POST_PROC__
#define __VIEW_POST_PROC__

#include <opencv2/opencv.hpp>
#include <string>

using namespace std;



/**
 * Perform one thinning iteration.
 * Normally you wouldn't call this function directly from your code.
 *
 * @param  im    Binary image with range = 0-1
 * @param  iter  0=even, 1=odd
*/
void thinningIteration(cv::Mat& im, int iter)
{
    cv::Mat marker = cv::Mat::zeros(im.size(), CV_8UC1);

    for (int i = 1; i < im.rows-1; i++)
    {
        for (int j = 1; j < im.cols-1; j++)
        {
            uchar p2 = im.at<uchar>(i-1, j);
            uchar p3 = im.at<uchar>(i-1, j+1);
            uchar p4 = im.at<uchar>(i, j+1);
            uchar p5 = im.at<uchar>(i+1, j+1);
            uchar p6 = im.at<uchar>(i+1, j);
            uchar p7 = im.at<uchar>(i+1, j-1);
            uchar p8 = im.at<uchar>(i, j-1);
            uchar p9 = im.at<uchar>(i-1, j-1);

            int A  = (p2 == 0 && p3 == 1) + (p3 == 0 && p4 == 1) +
                     (p4 == 0 && p5 == 1) + (p5 == 0 && p6 == 1) +
                     (p6 == 0 && p7 == 1) + (p7 == 0 && p8 == 1) +
                     (p8 == 0 && p9 == 1) + (p9 == 0 && p2 == 1);
            int B  = p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9;
            int m1 = iter == 0 ? (p2 * p4 * p6) : (p2 * p4 * p8);
            int m2 = iter == 0 ? (p4 * p6 * p8) : (p2 * p6 * p8);

            if (A == 1 && (B >= 2 && B <= 6) && m1 == 0 && m2 == 0)
                marker.at<uchar>(i,j) = 1;
        }
    }

    im &= ~marker;
}

/**
 * Function for thinning the given binary image
 * https://opencv-code.com/quick-tips/implementation-of-thinning-algorithm-in-opencv/
 *
 * @param  im  Binary image with range = 0-255
 */
void thinning(cv::Mat& im)
{
    im /= 255;

    cv::Mat prev = cv::Mat::zeros(im.size(), CV_8UC1);
    cv::Mat diff = cv::Mat::zeros(im.size(), CV_8UC1);

    do {
        thinningIteration(im, 0);
        thinningIteration(im, 1);
        cv::absdiff(im, prev, diff);
        im.copyTo(prev);
    }
    while (cv::countNonZero(diff) > 0);

    im *= 255;
}


void callCvSmooth(cv::Mat srcmtx, cv::Mat dstmtx, int smooth_type,
      int param1, int param2, double param3, double param4 ) {
   IplImage src = srcmtx;
   IplImage dst = dstmtx;
   cvSmooth( &src, &dst, smooth_type, param1, param2, param3, param4 );
}




class ViewProcessor {

public:
	ViewProcessor(cv::Mat img, int w, int h): _w(w), _h(h) {
		// imshow("img creating viewprocessor", img);
		_img = new cv::Mat();
		img.copyTo(*_img);
	}
	~ViewProcessor() {
		if (_img)
			_img->release();
		delete(_img);
	}

	ViewProcessor& gray() {
		// cout << "gray" << endl;
		cv::Mat gr;
        gr.create(_h, _w, CV_8UC1);
        cv::cvtColor(*_img, gr, CV_BGR2GRAY);
        resetImg(gr);
		return *this;
	}


	ViewProcessor& canny() {
		// cout << "canny" << endl;
		cv::Mat edge;
        edge.create(_h, _w, CV_8UC1);
        cv::Canny(*_img, edge, 150, 300, 3);
        resetImg(edge);
		return *this;
	}

	ViewProcessor& black2white() {
		// cout << "black2white" << endl;
		cv::Mat bw = (*_img) < 128;
        resetImg(bw);
		return *this;
	}

	ViewProcessor& blur() {
		// cout << "blur" << endl;
		cv::Mat gblur;
        gblur.create(_h, _w, CV_8UC1);
        callCvSmooth(*_img, gblur, CV_GAUSSIAN, 7, 0, 0, 0);
        resetImg(gblur);
		return *this;
	}

	void save(string path) {
		cout << "save() : save to :" << path << endl;
		cv::imwrite(path, *_img);
	}

private:
	void deleteImg() {
		if (_img) _img->release();
	}

	void resetImg(cv::Mat& newImg) {
		// cout << "resetImg" << endl;
		newImg.copyTo(*_img);
	}

	cv::Mat *_img;
	int _w;
	int _h;
};

#endif
