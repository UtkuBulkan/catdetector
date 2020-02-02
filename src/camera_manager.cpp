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
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <chrono>
#include <syslog.h>
#include "camera_manager.h"
#include <algorithm>
#include <vector>

#include <opencv2/core/utils/filesystem.hpp>

std::string output_directory="data/";

Camera::Camera(std::string input_device_name, std::string input_device_name_uuid)
{
	syslog(LOG_NOTICE, "Camera::Camera Begin");
	m_input_device_name = input_device_name;
	m_input_device_name_uuid = input_device_name_uuid;

	capture.open(m_input_device_name);
	if ( !capture.isOpened	() ) {
		throw "Error opening file.\n";
	}
	framecount = 0;
	catdetector_skip_this_number_of_frames = 24;

#if defined (CATDETECTOR_DEVELOPMENT)
	cv::namedWindow("Camera1", cv::WINDOW_NORMAL);
	cv::resizeWindow("Camera1", 640, 480);
#endif
	cv::utils::fs::createDirectory(output_directory);
	cv::utils::fs::createDirectory(output_directory + m_input_device_name_uuid + ".data");

	syslog(LOG_NOTICE, "Camera::Camera End");
}

Camera::~Camera()
{
	syslog(LOG_NOTICE, "Camera::~Camera Begin");
	outputVideo.release();
	syslog(LOG_NOTICE, "Camera::~Camera End");
}

void Camera::set_models(std::vector<ObjectDetector*> object_detectors)
{
	m_object_detectors = object_detectors;

}

std::string Camera::get_input_device_name()
{
	return m_input_device_name;
}

void Camera::generate_json(cv::Mat &frame, std::string class_name, const int &framecount, const int &itemid)
{
	syslog(LOG_NOTICE, "ObjectDetector::generate_json Begin");

	json local_j;

	local_j["class"] = class_name.c_str();
	local_j["frame"] = framecount;

	std::vector<uchar> buffer;
#define MB 1024 * 1024
	std::string frame_path, frame_name, frame_directory;
	frame_directory.append(output_directory);
	frame_directory.append(m_input_device_name_uuid);
	frame_directory.append(".data");
	frame_directory.append("/");
	frame_name.append(std::to_string(framecount/24));
	frame_name.append("-");
	frame_name.append(std::to_string(itemid));
	frame_name.append("-");
	frame_name.append(class_name.c_str());
	frame_name.append(".jpg");

	frame_path.append(frame_directory);
	frame_path.append(frame_name);

	imwrite(frame_path, frame);
	local_j["image"] = frame_path;
	j.push_back(local_j);

	generate_html_thumbnail(frame_path, frame_name);

	syslog(LOG_NOTICE, "ObjectDetector::generate_json End");
}

void Camera::generate_detected_object_count_map(std::string class_name)
{
	if (detected_object_count_map.find(class_name) != detected_object_count_map.end()) {
		detected_object_count_map[class_name] += 1;
	} else {
		detected_object_count_map[class_name] = 1;
	}
}

void Camera::generate_detected_object_timeline_map(std::string class_name, int framecount)
{
	if (detected_object_timeline_map.find(class_name) != detected_object_timeline_map.end()) {
		detected_object_timeline_map[class_name].push_back(framecount);
	} else {
		std::vector<int> timeline;
		timeline.push_back(framecount);
		detected_object_timeline_map[class_name] = timeline;
	}
}

template <typename T1, typename T2>
struct less_second {
    typedef std::pair<T1, T2> type;
    bool operator ()(type const& a, type const& b) const {
        return a.second > b.second;
    }
};

static std::pair<std::string, int> get_pair( std::pair<std::string, int> i )
{
	return i;
}

void Camera::generate_metadata_json()
{
	json metadata_j;
	std::vector<std::pair<std::string,int> > v( detected_object_count_map.size() );
	transform( detected_object_count_map.begin(), detected_object_count_map.end(), v.begin(), get_pair );
	std::sort( v.begin(), v.end(), less_second<std::string, int>());
	for (int i=0; i<(int)v.size(); i++) {
		json local_j;
		local_j["class"] = v[i].first;
		local_j["count"] = v[i].second;
		std::vector<int> timeline = detected_object_timeline_map[v[i].first];
		json timeline_j;
		for (int k=0; k<(unsigned int)timeline.size(); k++) {
			timeline_j[k] = timeline[k];
		}
		local_j["interval"].push_back(timeline_j);
		metadata_j.push_back(local_j);
	}
	std::ofstream json_metadata_file;
	std::string json_metadata_filename(output_directory + m_input_device_name_uuid + ".data" + "/" + m_input_device_name_uuid + "-metadata.json"); 
	json_metadata_file.open (json_metadata_filename);
	json_metadata_file << metadata_j.dump(0) << std::endl;
	json_metadata_file.close();
}

void Camera::generate_html_thumbnail(std::string frame_directory, std::string frame_name)
{
	html_file << "<html>" << std::endl;
	html_file << "<body>" << std::endl;
	html_file << "<a target=\"_blank\" href=\"" << frame_name << "\">" << std::endl;
	html_file << "<img src=\"" << frame_name << "\" style=\"float: left;display: block;max-width:150px;max-height:150px;width: auto;height: auto;\">" << std::endl;
	html_file << "</a>" << std::endl;
	html_file << "</body>" << std::endl;
	html_file << "</html>" << std::endl;
}

int Camera::process_frame()
{
	syslog(LOG_NOTICE, "Camera::loop Begin");

	std::string html_videodata_filename(output_directory + m_input_device_name_uuid + ".data" + "/" + m_input_device_name_uuid + ".html");
	html_file.open (html_videodata_filename);

	while (1) {
	if(framecount == 0) {
		m_fps = capture.get(cv::CAP_PROP_FPS);
		start_time = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		syslog(LOG_NOTICE, "Frame count : %d", framecount);
		syslog(LOG_DEBUG, "Frame resolution : %d x %d", frame.rows, frame.cols);
	}

	for(int k = 0; k < catdetector_skip_this_number_of_frames; k++) {
		capture >> frame;
		frame.copyTo(output_frame);
		framecount++;
		if (frame.empty()) {
			syslog(LOG_NOTICE, "Last read frame is empty, quitting.");
			return 0; //break;
		}
	}

	std::vector<std::pair<cv::Mat, std::string> > detected_objects;
	m_object_detectors[0]->process_frame(frame, output_frame, detected_objects);
	for(int i = 0; i < (int) detected_objects.size(); i++) {
		generate_json(detected_objects[i].first, detected_objects[i].second , framecount, i);
		generate_detected_object_count_map(detected_objects[i].second);
		generate_detected_object_timeline_map(detected_objects[i].second, framecount);
	}

	/* Outputting captured frames to json */
	std::ofstream json_file;
	std::string json_videodata_filename(output_directory + m_input_device_name_uuid + ".data" + "/" + m_input_device_name_uuid + ".json"); 
	json_file.open (json_videodata_filename);
	json_file << j.dump(0) << std::endl;
	json_file.close();

	/* Outputting video metadata to json */
	generate_metadata_json();
#if defined (CATDETECTOR_DEVELOPMENT)
	cv::imshow("Camera1", output_frame);
	if(cv::waitKey(30) >= 0) break;
#endif
	}
	html_file.close();


	syslog(LOG_NOTICE, "Camera::loop End");
	return 1;
}
