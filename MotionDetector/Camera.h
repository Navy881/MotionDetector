#pragma once

#include <thread>
#include <opencv2/opencv.hpp>


class Camera {

private:
	int camera;
	int frame_rate;
	int frame_width;
	int frame_height;
	cv::VideoCapture cap;
	cv::Mat frame;
	std::thread updating_thread;
	bool updating;


public:

	// Camera(); // ����������� �� ���������

	~Camera() = default; // ����������

	Camera(const int camera_id, int fps, int width, int height); // ����������� �� ���������

	cv::Mat get_frame(); // ��������� ����������
	
	int get_camera_id(); // ���������� ������ ������
	
	double get_fps(); // ���������� fps �����������

	int get_frame_width(); // ���������� ������ �����������

	int get_frame_height(); // ���������� ������ �����������

	bool isOpened(); // ���������� true ��� ������� ������, � false ��� ����������

	int get_fourcc(); // ���������� ��� ������

	void update(); // ���������� frame

	void start(); // ����� ������ ������ � ������

	void stop();  // ��������� ������ ������ � ������

};

/*
����������
*/

/*
// ����������� �� ���������
Camera::Camera() {
	camera = 0;

	cv::VideoCapture cap(camera);

	if (cap.isOpened())
		capture = cap;

	frame_rate = capture.get(cv::CAP_PROP_FPS);

	frame_width = capture.get(cv::CAP_PROP_FRAME_WIDTH);

	frame_height = capture.get(cv::CAP_PROP_FRAME_HEIGHT);
}
*/


// ����������� � �����������
Camera::Camera(const int camera_id = 0, int fps = 0, int width = 0, int height = 0) {

	if (camera_id == 0)
		camera = 0;
	else
		camera = camera_id;

	cap.open(camera);  // �������� �����������
	//updating = true;
	//std::thread updating_thread(&Camera::update, this);
	//updating_thread.detach();

	if (fps != 0)
		frame_rate = fps;
	else
		frame_rate = cap.get(cv::CAP_PROP_FPS);

	if (width != 0)
		frame_width = width;
	else
		frame_width = cap.get(cv::CAP_PROP_FRAME_WIDTH);

	if (height != 0)
		frame_height = height;
	else
		frame_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);

	cap.set(cv::CAP_PROP_FPS, frame_rate);

	cap.set(cv::CAP_PROP_FRAME_WIDTH, frame_width);

	cap.set(cv::CAP_PROP_FRAME_HEIGHT, frame_height);

	updating = false;
}


// Start camera

void Camera::start() {
	
	if (!updating) {

		updating = true;

		// std::thread thr([this] { this->update(); });
		updating_thread = std::thread(&Camera::update, this);
		updating_thread.detach();  // ������� ������ � ������� ������ (�� ����������� �������� �����)

	}
	else {
		std::cout << "WARNING: Camera is already starting" << std::endl;
	}
}


// Stop camera
void Camera::stop() {
	updating = false;
}

// ���������� frame
void Camera::update() {
	while (updating)
		cap.read(frame);
}


// �������� ���������� ��� �����������
cv::Mat Camera::get_frame() {
	if (cap.isOpened()) {
		// cap >> frame;
		// cap.read(frame);
		return frame;
	}
}


// �������� ������ ������
int Camera::get_camera_id() {
	return camera;
}


// �������� fps �����������
double Camera::get_fps() {
	return cap.get(cv::CAP_PROP_FPS);
}


// �������� ������ �����������
int Camera::get_frame_width() {
	return cap.get(cv::CAP_PROP_FRAME_WIDTH);
}


// �������� ������ �����������
int Camera::get_frame_height() {
	return cap.get(cv::CAP_PROP_FRAME_HEIGHT);
}


// �������� �������� ������
bool Camera::isOpened() {
	if (cap.isOpened())
		return true;
	return false;
}


// �������� ��� ������
int Camera::get_fourcc() {
	return cap.get(cv::CAP_PROP_FOURCC);
}
