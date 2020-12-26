
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <ctime>  // For getting time
#include <sys/types.h>  // For checking dirs
#include <sys/stat.h>  // For checking dirs
#include "windows.h"  // For creating dirs in windows
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <chrono> 

#include "Camera.h"


// Настройки
struct Options {
	int camera_index;
	int min_contour_area;
	int blur_size;
	int blur_power;
	double threshold_low;
	std::string telegram_bot_token;
	double output_video_fps;
	std::string video_dir_path;
};


// Получение настроек из файла config.txt.
// На входе указатель на переменную структуры Options 
void get_options(Options* options) {

	std::ifstream cFile("config.txt");

	if (cFile.is_open())
	{

		std::string line;

		std::cout << "OPTIONS:" << '\n';

		while (getline(cFile, line)) {

			line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());

			if (line[0] == '#' || line.empty())
				continue;

			auto delimiterPos = line.find("=");
			auto key = line.substr(0, delimiterPos);
			auto value = line.substr(delimiterPos + 1);

			if (key == "CAMERA_INDEX")
				options->camera_index = std::stoi(value);
			else if (key == "MIN_CONTOUR_AREA")
				options->min_contour_area = std::stoi(value);
			else if (key == "BLUR_SIZE")
				options->blur_size = std::stoi(value);
			else if (key == "BLUR_POWER")
				options->blur_power = std::stoi(value);
			else if (key == "THRESHOLD_LOW")
				options->threshold_low = std::stod(value);
			else if (key == "OUTPUT_VIDEO_FPS")
				options->output_video_fps = std::stod(value);
			else if (key == "VIDEO_DIR_PATH")
				options->video_dir_path = value;

			std::cout << key << " " << value << '\n';

		}

	}
	else {
		std::cerr << "Couldn't open config file for reading.\n";
	}
}


// Получение выхода видеофайла
void create_video_file(cv::VideoWriter* writer, std::string path, double fps, int width, int height) {

	struct stat info;

	if (stat(path.c_str(), &info) != 0) {
		printf("INFO: Cannot access to: %s\n", path.c_str());

		// Преобразование string -> wstring -> const *wchar_t для LPCWSTR
		std::wstring widestr_path = std::wstring(path.begin(), path.end());
		const wchar_t* widecstr_path = widestr_path.c_str();

		if (CreateDirectory(widecstr_path, NULL))
			std::cout << "INFO: Video directory create" << std::endl;
		else
			std::cout << "ERROR: Create video directory" << std::endl;

	}
	else if (info.st_mode & S_IFDIR)  // S_ISDIR() doesn't exist on my windows 
		printf("INFO: %s exists\n", path.c_str());
	else
		printf("INFO: %s is no directory\n", path.c_str());


	// Datetime to string
	auto t = std::time(nullptr);
	auto tm = *std::localtime(&t);
	std::ostringstream oss;
	oss << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
	auto datetime_str = oss.str();

	std::string videofile_name = path + "/" + datetime_str + ".avi";

	int codec = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');

	std::cout << '\n' << "INFO: Create file: " + videofile_name << '\n';

	writer->open(videofile_name, codec, fps, cv::Size(width, height), true);

	// check if we succeeded
	if (!writer->isOpened())
		std::cerr << "Could not open the output video file for write\n";

}


// Детектирование движения
void motion_detect(
	Camera* camera,
	cv::VideoWriter* writer,
	int min_area = 10,
	int blur_size = 11,
	int blur_power = 1,
	double threshold_low = 50) {

	cv::Mat first_frame = cv::Mat::zeros(cv::Size(1, 1), CV_64FC1);
	cv::Mat frame_copy;
	cv::Mat gray;
	cv::Mat frame_delta;
	cv::Mat thresh;
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;

	while (true)
	{
		auto start = std::chrono::high_resolution_clock::now();

		cv::Mat frame = camera->get_frame().clone();

		std::string detection_text = "Unoccupied";
		cv::Scalar detection_text_color = CV_RGB(0, 255, 0);

		// Изменение размера изображнеия на 500x500
		// cv::resize(frame, frame1, cv::Size(500, 500), 0, 0, cv::INTER_AREA);

		// Обесцвечивание изображения
		cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

		// Размытие изображения фильтром Гаусса
		cv::GaussianBlur(gray, gray, cv::Size(blur_size, blur_size), blur_power);

		// Первый кадр обесцвеченный
		if (cv::countNonZero(first_frame) < 1) {
			first_frame = gray.clone();
			continue;
		}

		// Difference between first and gray frames
		cv::absdiff(first_frame, gray, frame_delta);

		// Frame_delta binarization
		cv::threshold(frame_delta, thresh, threshold_low, 255, cv::THRESH_BINARY);  // [1]

		// Noise suppression
		cv::dilate(thresh, thresh, 0, cv::Point(-1, -1), 2, 1, 1);

		// Поиск замкнутных контуров
		cv::findContours(thresh.clone(), contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

		/*
		Добавление текста на изображение
		*/
		std::string camera_label = "Camera " + std::to_string(camera->get_camera_id() + 1) +
			" FPS:" + std::to_string(int(camera->get_fps())) +
			" Size:" + std::to_string(camera->get_frame_width()) + "x" + std::to_string(camera->get_frame_height());

		// Отображение текста на изображении
		cv::putText(frame, //target image
			camera_label, //text
			cv::Point(10, 30), //top-left position
			cv::FONT_HERSHEY_DUPLEX,
			0.7,
			CV_RGB(0, 255, 0), //font color
			1,
			cv::LINE_4);

		// Datetime to string
		auto t = std::time(nullptr);
		auto tm = *std::localtime(&t);
		std::ostringstream oss;
		oss << std::put_time(&tm, "%d-%m-%Y %H:%M:%S");
		auto datetime_str = oss.str();

		cv::putText(frame, datetime_str, cv::Point(10, frame.rows - 10), cv::FONT_HERSHEY_DUPLEX, 0.7, CV_RGB(0, 255, 0), 1, cv::LINE_4);

		// Оценка площади каждого контура
		for (size_t i = 0; i < contours.size(); i++)
		{

			if (cv::contourArea(contours[i]) < min_area)
				continue;

			detection_text = "Occupied";
			detection_text_color = CV_RGB(255, 0, 0);

			first_frame = gray.clone();

			cv::drawContours(frame, contours, (int)i, CV_RGB(255, 0, 0), 1, cv::LINE_8, hierarchy, 0);

			// Write frame to videofile
			writer->write(frame);

		}

		// Доавление на кадр статуса обнаружения движения
		cv::putText(frame, detection_text, cv::Point(10, 60), cv::FONT_HERSHEY_DUPLEX, 0.7, detection_text_color, 1, cv::LINE_4);

		cv::imshow("Camera 1", frame);

		auto stop = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
		// std::cout << duration.count() << std::endl;

		// Press ESC on keyboard to exit.
		// ONLY RELEASE MODE
		char c = (char)cv::waitKey(25);
		if (c == 27)
			break;


	}

}


int main()
{
	/*
	cv::Mat img;
	cv::VideoCapture cap(0);
	cap.set(cv::CAP_PROP_FPS, 30);
	cap.set(cv::CAP_PROP_FRAME_WIDTH, 1024);
	cap.set(cv::CAP_PROP_FRAME_HEIGHT, 720);

	while (true)
	{
		// cv::Mat img = camera->get_frame();
		cap >> img;

		cv::Mat gray;

		// Обесцвечивание изображения в матрицу gray
		cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);

		cv::imshow("Camera 1", gray);

		cv::waitKey(1);
	}
	*/

	Options options;
	get_options(&options);  // в качестве аргумента передается ссылка на переменную ("&" впереди)

	// Camera* camera = new Camera(0, 30);
	Camera* camera = new Camera(options.camera_index, 30);

	camera->start();  // Старт чтение видеопотока

	// Create writer to videofile
	cv::VideoWriter writer;

	bool writerIsOpened;

	if (camera->isOpened()) {

		// Create videofile
		create_video_file(
			&writer,
			options.video_dir_path,
			options.output_video_fps,
			camera->get_frame_width(),
			camera->get_frame_height());


		if (writer.isOpened())
			// Detect motion
			motion_detect(
				camera,
				&writer,
				options.min_contour_area,
				options.blur_size,
				options.blur_power,
				options.threshold_low);

		else
			std::cout << '\n' << "ERROR: Videofile not created" << '\n' << std::endl;

	}
	else {
		std::cout << '\n' << "ERROR: Camera not found" << '\n' << std::endl;
	}


	camera->stop(); // Остановка чтение видеопотока


	/*
	while (true)
	{

		cv::Mat gray;

		// Обесцвечивание изображения в матрицу gray
		// cv::cvtColor(camera->get_frame(), gray, cv::COLOR_BGR2GRAY);

		cv::Mat img = camera->get_frame();

		std::string camera_label = "Camera " + std::to_string(camera->get_camera_id() + 1) +
			" FPS:" + std::to_string(camera->get_fps()) +
			" Size:" + std::to_string(camera->get_frame_width()) + "x" + std::to_string(camera->get_frame_height());

		// Отображение текста на изображении
		cv::putText(img, //target image
			camera_label, //text
			cv::Point(10, 30), //top-left position
			cv::FONT_HERSHEY_DUPLEX,
			0.8,
			CV_RGB(118, 0, 0), //font color
			2);

		motion_detect(img, 10, 11, 1, 50.0);

		cv::imshow("Camera 1", img);

		// Press ESC on keyboard to exit.
		// ONLY RELEASE MODE
		char c = (char)cv::waitKey(25);
		if (c == 27)
			break;
	}
	*/

	// close the window
	cv::destroyAllWindows();

	writer.release();

	delete camera;

	return 0;
}