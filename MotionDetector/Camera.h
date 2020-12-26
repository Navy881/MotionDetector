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

	// Camera(); // конструктор по умолчанию

	~Camera() = default; // деструктор

	Camera(const int camera_id, int fps, int width, int height); // конструктор по умолчанию

	cv::Mat get_frame(); // возвращет видеопоток
	
	int get_camera_id(); // возвращает иднекс камеры
	
	double get_fps(); // возвращает fps видеопотока

	int get_frame_width(); // возвращает ширину изображени€

	int get_frame_height(); // возвращает высоты изображени€

	bool isOpened(); // возвращает true при наличие потока, и false при отсутствии

	int get_fourcc(); // возвращает тип кодека

	void update(); // обновление frame

	void start(); // старт чтени€ потока с камеры

	void stop();  // остановка чтени€ потока с камеры

};

/*
–еализаци€
*/

/*
//  онструктор по умолчанию
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


//  онструктор с параметрами
Camera::Camera(const int camera_id = 0, int fps = 0, int width = 0, int height = 0) {

	if (camera_id == 0)
		camera = 0;
	else
		camera = camera_id;

	cap.open(camera);  // ќткрытие видеопотока
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
		updating_thread.detach();  // «апуска потока в фоновом режиме (не блокирующем основной поток)

	}
	else {
		std::cout << "WARNING: Camera is already starting" << std::endl;
	}
}


// Stop camera
void Camera::stop() {
	updating = false;
}

// ќбновление frame
void Camera::update() {
	while (updating)
		cap.read(frame);
}


// ѕолучить видеопоток как изображение
cv::Mat Camera::get_frame() {
	if (cap.isOpened()) {
		// cap >> frame;
		// cap.read(frame);
		return frame;
	}
}


// ѕолучить индекс камеры
int Camera::get_camera_id() {
	return camera;
}


// ѕолучить fps видеопотока
double Camera::get_fps() {
	return cap.get(cv::CAP_PROP_FPS);
}


// ѕолучить ширину изображени€
int Camera::get_frame_width() {
	return cap.get(cv::CAP_PROP_FRAME_WIDTH);
}


// ѕолучить высоту изображени€
int Camera::get_frame_height() {
	return cap.get(cv::CAP_PROP_FRAME_HEIGHT);
}


// ѕроверка открыти€ потока
bool Camera::isOpened() {
	if (cap.isOpened())
		return true;
	return false;
}


// ѕолучить тип кодека
int Camera::get_fourcc() {
	return cap.get(cv::CAP_PROP_FOURCC);
}
