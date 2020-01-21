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
*/
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <string>
#include "json.hpp"

#include <fstream>
#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using json = nlohmann::json;

class ObjectDetector {

public:
	static ObjectDetector* GenerateDetector(std::string objectdetector_type);

	void setup_model_for_detector(std::string class_definition_file, std::string model_config_file, std::string model_weights_file);
	void load_model_classes_for_detector();
	void load_network_model_for_detector(std::string network_type);
	std::vector < std::string > get_class_labels();

	float get_confidence_threshold();
	float get_nms_threshold();
	cv::dnn::Net get_net();

	virtual std::string process_frame(cv::Mat &frame, cv::Mat &output_frame, std::vector<std::pair<cv::Mat, std::string> > &detections) = 0;

	void draw_box(cv::Mat& frame, int classId, float conf, cv::Rect box, cv::Mat& objectMask);
	void post_process(cv::Mat& frame, const std::vector<cv::Mat>& outs, int framecount, std::string hash_video);

	void generate_json(cv::Mat &frame, const int &classId, const int &framecount, const int &itemid, std::string frame_md5, std::string video_md5);
	void generate_html_thumbnail(std::string frame_directory, std::string frame_name);

	std::string filename;
	std::string uuid;

protected :
	ObjectDetector();
	~ObjectDetector();
	virtual void post_process(cv::Mat& frame, cv::Mat &output_frame, std::vector<cv::Mat> detection_data, std::vector<std::pair<cv::Mat, std::string> > & detections) = 0;
	virtual void draw_prediction_indicators(int classId, float conf, int left, int top, int right, int bottom, cv::Mat& output_frame) = 0;

private:
	// Initialize the parameters
	std::vector < std::string > classes;
	std::vector<cv::Scalar> colors;
	// Give the configuration and weight files for the model
	std::string colors_file;
	std::string m_class_definition_file;
	std::string m_model_config_file;
	std::string m_model_weights_file;

	// Initialize the parameters
	float m_confidence_threshold; // Confidence threshold
	float m_nms_threshold; // non-maximum suppression threshold

	cv::dnn::Net net;

	json j;
	std::ofstream html_file;
};
