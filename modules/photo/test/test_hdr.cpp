/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                           License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000-2008, Intel Corporation, all rights reserved.
// Copyright (C) 2009, Willow Garage Inc., all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of the copyright holders may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#include "test_precomp.hpp"
#include <string>
#include <algorithm>

using namespace cv;
using namespace std;

TEST(Photo_HdrFusion, regression)
{
	string folder = string(cvtest::TS::ptr()->get_data_path()) + "hdr/";
	
	vector<string>file_names(3);
	file_names[0] = folder + "grand_canal_1_45.jpg";
	file_names[1] = folder + "grand_canal_1_180.jpg";
	file_names[2] = folder + "grand_canal_1_750.jpg";
	vector<Mat>images(3);
	for(int i = 0; i < 3; i++) {
		images[i] = imread(file_names[i]);
		ASSERT_FALSE(images[i].empty()) << "Could not load input image " << file_names[i];
	}
	
	string expected_path = folder + "grand_canal_rle.hdr";
	Mat expected = imread(expected_path, -1);
	ASSERT_FALSE(expected.empty()) << "Could not load input image " << expected_path;

	vector<float>times(3);
	times[0] = 1.0f/45.0f;
	times[1] = 1.0f/180.0f;
	times[2] = 1.0f/750.0f;
	
	Mat result;
	makeHDR(images, times, result);
	double max = 1.0;
	minMaxLoc(abs(result - expected), NULL, &max);
	ASSERT_TRUE(max < 0.01);

	expected_path = folder + "grand_canal_exp_fusion.png";
	expected = imread(expected_path);
	ASSERT_FALSE(expected.empty()) << "Could not load input image " << expected_path;
	exposureFusion(images, result);
	result.convertTo(result, CV_8UC3, 255);
	minMaxLoc(abs(result - expected), NULL, &max);
	ASSERT_FALSE(max > 0);
}

TEST(Photo_Tonemap, regression)
{
	string folder = string(cvtest::TS::ptr()->get_data_path()) + "hdr/";
	
	vector<string>file_names(TONEMAP_COUNT);
	file_names[TONEMAP_DRAGO] = folder + "grand_canal_drago_2.2.png";
	file_names[TONEMAP_REINHARD] = folder + "grand_canal_reinhard_2.2.png";
	file_names[TONEMAP_DURAND] = folder + "grand_canal_durand_2.2.png"; 
	file_names[TONEMAP_LINEAR] = folder + "grand_canal_linear_map_2.2.png";

	vector<Mat>images(TONEMAP_COUNT);
	for(int i = 0; i < TONEMAP_COUNT; i++) {
		images[i] = imread(file_names[i]);
		ASSERT_FALSE(images[i].empty()) << "Could not load input image " << file_names[i];
	}
	
	string hdr_file_name = folder + "grand_canal_rle.hdr";
	Mat img = imread(hdr_file_name, -1);
	ASSERT_FALSE(img.empty()) << "Could not load input image " << hdr_file_name;
	
	vector<float> param(1);
	param[0] = 2.2f;

	for(int i = TONEMAP_DURAND; i < TONEMAP_COUNT; i++) {
		
		Mat result;
		tonemap(img, result, static_cast<tonemap_algorithms>(i), param);
		result.convertTo(result, CV_8UC3, 255);
		double max = 1.0;
		minMaxLoc(abs(result - images[i]), NULL, &max);
		ASSERT_FALSE(max > 0);
	}
}

TEST(Photo_Align, regression)
{
	const int TESTS_COUNT = 100;
	string folder = string(cvtest::TS::ptr()->get_data_path()) + "hdr/";
	
	string file_name = folder + "grand_canal_1_45.jpg";
	Mat img = imread(file_name);
	ASSERT_FALSE(img.empty()) << "Could not load input image " << file_name;
	cvtColor(img, img, COLOR_RGB2GRAY);

	int max_bits = 6;
	int max_shift = 64;
	srand(time(0));

	for(int i = 0; i < TESTS_COUNT; i++) {
		Point shift(rand() % max_shift, rand() % max_shift);
		Mat res;
		shiftMat(img, shift, res);
		Point calc = getExpShift(img, res, max_bits);
		ASSERT_TRUE(calc == -shift);
	}
}