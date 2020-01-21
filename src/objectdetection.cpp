/*
 * Copyright 2018 SU Technology Ltd. All rights reserved.
 *
 * MIT License
 *
 * Copyright (c) 2018 SU Technology
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
 * Some parts of this file has been inspired and influenced from this link :
 * https://www.learnopencv.com/deep-learning-based-object-detection
 * -and-instance-segmentation-using-mask-r-cnn-in-opencv-python-c/
 *
 */
#include <string>
#include <syslog.h>

#include "objectdetection_yolo.h"

ObjectDetector* ObjectDetector::GenerateDetector(std::string objectdetector_type_string)
{
	if (objectdetector_type_string == "Yolo")
		return new ObjectDetector_Yolo;

	return NULL;
}

ObjectDetector::ObjectDetector() : 	m_confidence_threshold(0.5), m_nms_threshold(0.4) {}

ObjectDetector::~ObjectDetector() {}

void ObjectDetector::setup_model_for_detector(std::string class_definition_file, std::string model_config_file, std::string model_weights_file)
{
	m_model_config_file = model_config_file;
	m_model_weights_file = model_weights_file;
	m_class_definition_file = class_definition_file;
}

void ObjectDetector::load_model_classes_for_detector()
{
	std::ifstream classes_file_stream(m_class_definition_file.c_str());
	std::string class_file_line;

	while (getline(classes_file_stream, class_file_line)) {
		classes.push_back(class_file_line);
		syslog (LOG_NOTICE, "Class Labels : %s", class_file_line.c_str());
	}

	std::ifstream colors_file_stream(colors_file.c_str());
	std::string colors_file_line;

	while (getline(colors_file_stream, colors_file_line)) {
		std::stringstream ss(colors_file_line);
		double red, green, blue;
		ss >> red >> green >> blue;
		colors.push_back(cv::Scalar(red, green, blue, 255.0));
		syslog (LOG_NOTICE, "Colors.txt Colors : %f, %f, %f", red, green, blue);
	}
}

void ObjectDetector::load_network_model_for_detector(std::string network_type)
{
	// Load the network for the model
	syslog (LOG_NOTICE, "ObjectDetector Loading Network");
	if (network_type == "Darknet") {
		net = cv::dnn::readNetFromDarknet(m_model_config_file, m_model_weights_file);
	} else if(network_type == "Caffe") {
		net = cv::dnn::readNetFromCaffe(m_model_config_file, m_model_weights_file);
	}
	net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
	net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
	syslog (LOG_NOTICE, "ObjectDetector Network Loaded");
}

std::vector < std::string > ObjectDetector::get_class_labels()
{
	return classes;
}

float ObjectDetector::get_confidence_threshold()
{
	return m_confidence_threshold;
}

float ObjectDetector::get_nms_threshold()
{
	return m_nms_threshold;
}

cv::dnn::Net ObjectDetector::get_net()
{
	return net;
}

#if 0
void ObjectDetector::loop() {
	cv::Mat frame;
	cv::VideoCapture capture;
	cv::VideoWriter outputVideo;

	syslog(LOG_NOTICE, "Opening file : %s", filename.c_str());

	capture.open(filename);
	if ( !capture.isOpened	() ) {
		throw "Error opening file.\n";
	}

	capture >> frame;

	int ex = static_cast<int>(capture.get(cv::CAP_PROP_FOURCC));	// Get Codec Type- Int form
	int codec = cv::VideoWriter::fourcc('M', 'P', 'G', '2');
	cv::Size S = cv::Size((int) capture.get(cv::CAP_PROP_FRAME_WIDTH), (int) capture.get(cv::CAP_PROP_FRAME_HEIGHT));

	syslog(LOG_NOTICE, "Input file fourcc: %d, %d", codec, ex);
	syslog(LOG_NOTICE, "Input file width: %d", S.width);
	syslog(LOG_NOTICE, "Input file height: %d", S.height);

	outputVideo.open("./output.mp4", cv::CAP_FFMPEG, codec, capture.get(cv::CAP_PROP_FPS), S, true);
	outputVideo << frame;

	cv::namedWindow("Camera1", cv::WINDOW_NORMAL);
	cv::resizeWindow("Camera1", 640, 480);

	int framecount = 0;
	std::string hash_video = "1";

	while(1) {
		syslog(LOG_NOTICE, "Frame count : %d", framecount);
		syslog(LOG_NOTICE, "Frame resolution : %d x %d", frame.rows, frame.cols);

		capture >> frame;
		framecount++;
#ifdef CATDETECTOR_ANALYSE_EVERY_24_FRAMES
		if (framecount % 24 == 0)
#endif
		{
			process_frame(frame, framecount, hash_video);
#ifdef CATDETECTOR_ENABLE_OUTPUT_TO_VIDEO_FILE
			/* Outputting captured frames to a video file */
			outputVideo << frame;
#endif
			cv::imshow("Camera1", frame);

#ifdef CATDETECTOR_ENABLE_CAPTURED_FRAMES_TO_JSON
			/* Outputting captured frames to json */
			std::ofstream myfile;
			std::string videodata_filename(hash_video + ".json");
			myfile.open (videodata_filename);
			myfile << j << std::endl;
			myfile.close();
#endif
			/* Sending the data as a Kafka producer */
			/* video_analyser_kafka_producer(j.dump().c_str(), "TutorialTopic"); */
		}
		if(cv::waitKey(30) >= 0) break;
	}
	outputVideo.release();
}
#endif