#pragma once
#include<iostream>

using std::string;

const short NUM_THREADS = 4;

const short SERVER_DEFAULT_PORT = 8080;

const short RESPONSE_SUCCESS = 0;
const short RESPONSE_FAILURE = -1;
const short SERVER_POSITIVE_ACK = 10;
const short SERVER_NEGATIVE_ACK = 11;
const short CLIENT_POSITIVE_ACK = 20;
const short CLIENT_NEGATIVE_ACK = 21;

const short MIN_IMAGE_METADATA_PARAMS = 5;
const short MIN_CLIENT_RESPONSE_PARAMS = 2;
const short NUM_OF_IMAGE_DATA_PARAMS = 5;
const string SIZE_PAYLOAD_KEY = "SIZE";
const string SEQUENCE_PAYLOAD_KEY = "SEQ";
const string RESPONSE_PAYLOAD_KEY = "RES";

const ushort MAX_IMAGE_DATA_BYTES_IN_ONE_PAYLOAD = 60000;
const ushort MAX_SERVER_RESPONSE_PAYLOAD_SIZE_BYTES = 5000;
const ushort MAX_CLIENT_PAYLOAD_SIZE_BYTES = 60025;
const ushort MIN_CLIENT_PAYLOAD_SIZE_BYTES = 15;

const string SERVER_MSG_DELIMITER = " ";
const char CLIENT_RESPONSE_DELIMITER = ' ';
const string CLIENT_ADDRESS_KEY_DELIMITER = ":;";
const char STRING_TERMINATING_CHAR = '\0';
const string STRING_TERMINATING_CHAR_AS_STRING = "\0";
const string EMPTY_STRING = "";

const ushort IMAGE_PAYLOAD_RECV_TIMEOUT_MILLIS = 2000;
const ushort CLIENT_MSG_RECV_TIMEOUT_MILLIS = 5000;

enum RotationDirection {CLOCKWISE, ANTI_CLOCKWISE};
enum FlipDirection {HORIZONTAL, VERTICAL};
enum ImageFilterTypesEnum
{
	NONE, RESIZE, ROTATE, CROP, FLIP, RGB_TO_GRAYSCALE, BRIGHTNESS_ADJ
};