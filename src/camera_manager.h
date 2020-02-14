/*
 * Copyright 2019 SU Technology Ltd. All rights reserved.
 *
 * MIT License
 *
 * Copyright (c) 2019 SU Technology
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */
#ifndef _CAMERA_MANAGER_H_
#define _CAMERA_MANAGER_H_

#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <map>
#include <opencv2/opencv.hpp>

#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include <fstream>

#include "objectdetection.h"

class Camera
{
public:
	Camera(std::string input_device_name, std::string input_device_name_uuid, std::string input_username);
	~Camera();
	void set_models(std::vector<ObjectDetector*> object_detectors);
	std::string get_input_device_name();
	int process_frame();

	void generate_json(cv::Mat &frame, std::string class_name, const int &framecount, const int &itemid);
	void generate_html_thumbnail(std::string frame_directory, std::string frame_name);
	void generate_metadata_json();
	void generate_detected_object_count_map(std::string class_name);
	void generate_detected_object_timeline_map(std::string class_name, int framecount);

private:
	cv::Mat frame, output_frame;
	int framecount;
	double m_fps;
	int catdetector_skip_this_number_of_frames;
	int64_t start_time;

	std::string output_directory;
	std::string m_input_device_name, m_input_device_name_uuid, m_input_username;
	std::string m_output_file_path;
	cv::VideoCapture capture;
	cv::VideoWriter outputVideo;

	std::vector<ObjectDetector*> m_object_detectors;

	json j;
	std::ofstream html_file;

	std::map<std::string, int> detected_object_count_map;
	std::map<std::string, std::vector<int> > detected_object_timeline_map;
};
#endif /* _CAMERA_MANAGER_H_ */
