#ifdef CPP_TEST_WITH_OPENCV

/*!
 * test for opencv
 * @author hongjun.liao <docici@126.com>*/
#include <cpp_test.h>
#include <stdio.h>
#ifdef TEST_OPENCV
#include <iostream>
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;

static int test_opencv_read_video_main(int argc, char ** argv)
{
	fprintf(stdout, "%s: read video file 'myvideo.avi'\n", __FUNCTION__);
	//打开视频文件
	VideoCapture cap("myvideo.avi");
	//检查是否成功打开
	if (!cap.isOpened()) {
		cerr << "Can not open a camera or file." << endl;
		return -1;
	}
	Mat edges;
	//创建窗口
	namedWindow("edges", 1);
	for (;;) {
		Mat frame;
		//从 cap 中读一帧，存到 frame
		cap >> frame;
		//如果未读到图像
		if (frame.empty())
			break;
		//将读到的图像转为灰度图
		cvtColor(frame, edges, CV_BGR2GRAY);
		//进行边缘提取操作
		Canny(edges, edges, 0, 30, 3);
		//显示结果
		imshow("edges", edges);
		//等待 30 秒，如果按键则推出循环
		if (waitKey(30) >= 0)
			break;
	}
	//退出时会自动释放 cap 中占用资源
	return 0;
}
static int test_opencv_write_video_main(int argc, char ** argv)
{
	fprintf(stdout, "%s: save video to file 'myvideo.avi'\n", __FUNCTION__);
	//定义视频的宽度和高度
	Size s(320, 240);
	//创建 writer，并指定 FOURCC 及 FPS 等参数
	VideoWriter writer = VideoWriter("myvideo.avi",
			CV_FOURCC('M', 'J', 'P', 'G'), 25, s);
	//检查是否成功创建
	if (!writer.isOpened()) {
		cerr << "Can not create video file.\n" << endl;
		return -1;
	}
	//视频帧
	Mat frame(s, CV_8UC3);
	for (int i = 0; i < 100; i++) {
		//将图像置为黑色
		frame = Scalar::all(0);
		//将整数 i 转为 i 字符串类型
		char text[128];
		snprintf(text, sizeof(text), "%d", i);
		//将数字绘到画面上
		putText(frame, text, Point(s.width / 3, s.height / 3),
				FONT_HERSHEY_SCRIPT_SIMPLEX, 3, Scalar(0, 0, 255), 3, 8);
		//将图像写入视频
		writer << frame;
	}
	//退出程序时会自动关闭视频文件
	return 0;
}
int test_opencv_main(int argc, char ** argv)
{
	auto ret = test_opencv_write_video_main(argc, argv);
	ret = test_opencv_read_video_main(argc, argv);
	return ret;
}

#else
int test_opencv_main(int argc, char ** argv)
{
	fprintf(stdout, "%s: TEST_OPENCV not defiend\n", __FUNCTION__);
}
#endif	/* TEST_OPENCV */

#endif

