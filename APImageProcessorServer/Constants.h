#pragma once
#include<iostream>

using std::string;

const short NUM_THREADS = 4;

const short RESPONSE_SUCCESS = 0;
const short RESPONSE_FAILURE = -1;
const short SERVER_POSITIVE_ACK = 10;
const short SERVER_NEGATIVE_ACK = 11;

const string SIZE_PAYLOAD_KEY = "SIZE";
const string SEQUENCE_PAYLOAD_KEY = "SEQ";
const string RESPONSE_PAYLOAD_KEY = "RES";

const u_short MAX_SERVER_RESPONSE_PAYLOAD_SIZE_BYTES = 5000;

const int IMAGE_PAYLOAD_RECV_TIMEOUT_MILLIS = 2000;

enum RotationDirection {CLOCKWISE, ANTI_CLOCKWISE};
enum FlipDirection {HORIZONTAL, VERTICAL};
enum ImageFilterTypesEnum
{
	NONE, RESIZE, ROTATE, CROP, FLIP, RGB_TO_GRAYSCALE, BRIGHTNESS_ADJ
};