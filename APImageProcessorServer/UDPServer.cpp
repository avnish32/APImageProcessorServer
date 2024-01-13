#include "UDPServer.h"
#include "Constants.h"
#include "ImageMetadataProcessor.h"
#include "ImageProcessor.h"

#pragma comment (lib, "ws2_32.lib")

using std::cout;
using std::to_string;
using std::queue;
using std::stringstream;
using std::invalid_argument;
using std::chrono::milliseconds;
using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;

UDPServer::UDPServer()
{
	WSADATA wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) == -1) {
		msg_logger_->LogError("Error while initialising WSA.");
		socket_ = INVALID_SOCKET;
	}
	else {
		socket_ = socket(AF_INET, SOCK_DGRAM, 0);
		if (socket_ == INVALID_SOCKET) {
			msg_logger_->LogError("Error while creating socket. Error code: " + to_string(WSAGetLastError()));
		}
		else {
			msg_logger_->LogError("Socket created successfully.");

			u_long NON_BLOCKING_MODE_TRUE = 0;
			if (ioctlsocket(socket_, FIONBIO, &NON_BLOCKING_MODE_TRUE) == SOCKET_ERROR) {
				msg_logger_->LogError("ERROR: Could not set server socket to non-blocking mode.");
				socket_ = INVALID_SOCKET;
			}
			sockaddr_in server_address;
			server_address.sin_family = AF_INET;
			server_address.sin_addr.s_addr = ADDR_ANY;
			server_address.sin_port = SERVER_DEFAULT_PORT;
			
			if (bind(socket_, (const sockaddr*)&server_address, (int) sizeof(server_address)) == SOCKET_ERROR) {
				msg_logger_->LogError("Error while binding server socket. Error code: " + to_string(WSAGetLastError()));
				socket_ = INVALID_SOCKET;
			}
			else {
				msg_logger_->LogError("Server ready to receive requests!");
			}
		}
	}
}

UDPServer::~UDPServer()
{
	msg_logger_->LogError("UDPServer destructor called.");
	WSACleanup();
	closesocket(socket_);
}

/*
This is the main orchestrator function that calls all the other function in order of the steps to perform 
for processing the image. When a new client connects, this is the function that is enqueued as a task into
the thread pool. Each connected client has their own copy of this function running in a separate thred.
*/
void UDPServer::ProcessImageReq(const sockaddr_in& client_address)
{
	cout << "\n\n";
	msg_logger_->LogError("Started processing request from client.");

	cv::Size image_dimensions;
	uint image_file_size;
	ImageFilterTypesEnum filter_type;
	vector<float> filter_params;
	short response_code;
	short server_response_code_for_client = SERVER_POSITIVE_ACK;

	string client_address_key = to_string(client_address.sin_addr.s_addr) + CLIENT_ADDRESS_KEY_DELIMITER + to_string(client_address.sin_port);
	mtx_.lock();
	queue<string>& client_queue = client_queue_map_[client_address_key];
	mtx_.unlock();

	
	response_code = InitializeImageMetadata(image_dimensions, image_file_size, filter_type, filter_params, client_queue);
	if (response_code == RESPONSE_FAILURE) {
		server_response_code_for_client = SERVER_NEGATIVE_ACK;
	}
	response_code = SendServerResponseToClient(server_response_code_for_client, client_address, nullptr);
	if (response_code == RESPONSE_FAILURE) {
		msg_logger_->LogError("ERROR: Could not send acknowledgement to client.");
		RemoveClientDataFromMap(client_address_key);
		return;
	}
	msg_logger_->LogError("Acknowledgement sent. Server response code: " + to_string(server_response_code_for_client));

	if (server_response_code_for_client == SERVER_NEGATIVE_ACK) {
		return;
	}

	long image_bytes_recd = 0, image_bytes_left_to_receive = image_file_size;
	u_short expected_number_of_payloads = image_bytes_left_to_receive / MAX_IMAGE_DATA_BYTES_IN_ONE_PAYLOAD;
	if (image_bytes_left_to_receive % MAX_IMAGE_DATA_BYTES_IN_ONE_PAYLOAD > 0) {
		expected_number_of_payloads++;
	}

	string image_data_string = EMPTY_STRING;
	map<u_short, string> image_payload_seq_map;
	
	response_code = ConsumeImageDataFromClientQueue(client_queue, image_payload_seq_map, expected_number_of_payloads,
		client_address, image_bytes_left_to_receive);

	if (response_code == RESPONSE_FAILURE) {
		msg_logger_->LogError("Error while consuming image data from client queue.");
		RemoveClientDataFromMap(client_address_key);
		return;
	}
	
	msg_logger_->LogError("All image data received. Sending positive ACK to client.");

	response_code = SendServerResponseToClient(SERVER_POSITIVE_ACK, client_address, nullptr);
	if (response_code == RESPONSE_FAILURE) {
		msg_logger_->LogError("ERROR: Could not send ACK to client.");
		RemoveClientDataFromMap(client_address_key);
		return;
	}

	msg_logger_->LogError("Acknowledgement sent. Server response code: " + to_string(SERVER_POSITIVE_ACK));

	//Saving original received image
	ImageProcessor image_processor(image_payload_seq_map, image_dimensions, image_file_size);
	//imageProcessor.SaveImage();

	//Apply filter to image
	Mat filtered_image = image_processor.ApplyFilter(filter_type, filter_params);
	//imageProcessor.SaveImage(filteredImage);
	
	msg_logger_->LogError("Filter applied. Sending processed image metadata to client.");

	//Send filtered image dimensions
	response_code = SendImageMetadataToClient(filtered_image, client_address);
	if (response_code == RESPONSE_FAILURE) {
		msg_logger_->LogError("Could not send metadata of processed image to client.");
		RemoveClientDataFromMap(client_address_key);
		return;
	}

	//Receive ACK from client
	vector<string> client_response_split;
	short client_response_code;
	response_code = ConsumeAndValidateClientMsgFromQueue(client_queue, client_response_split, client_response_code);
	if (response_code == RESPONSE_FAILURE) {
		msg_logger_->LogError("Error while receiving/validating client response.");
		RemoveClientDataFromMap(client_address_key);
		return;
	}

	if (client_response_code == CLIENT_NEGATIVE_ACK) {
		msg_logger_->LogError("ERROR: Client sent negative ack. Terminating connection.");
		RemoveClientDataFromMap(client_address_key);
		return;
	}

	msg_logger_->LogError("Positive ACK recd from client. Sending processed image...");

	//Send processed image until positive ACK recd
	response_code = SendImage(filtered_image, client_address, client_queue);
	if (response_code == RESPONSE_FAILURE) {
		msg_logger_->LogError("Error while sending processed image back to client.");
	}
	else {
		msg_logger_->LogError("Processed image successfully sent back to client.");
	}
	RemoveClientDataFromMap(client_address_key);
}

/*
* ---------------------
* Utility functions
* ---------------------
*/

/*
* Splits inputString based on delimiter character until the character '\0' is encountered.
*/
const vector<std::string> UDPServer::SplitString(char* input_string, char delimiter) {
	std::string current_word = EMPTY_STRING;
	vector<std::string> output_vector;
	char* char_ptr = input_string;
	
	while (*(char_ptr) != STRING_TERMINATING_CHAR) {
		char current_char = *(char_ptr);
		if (current_char == delimiter) {
			output_vector.push_back(current_word);
			current_word = EMPTY_STRING;
		}
		else {
			current_word += current_char;
		}
		++char_ptr;
	}
	if (current_word.length() > 0) {
		output_vector.push_back(current_word);
	}
	return output_vector;
}

/*
Splits inputString by the delimiter character until the splits reach numberOfSplits,
or the whole length of the string given by inputStringLength has been traversed,
whichever occurs earlier.
*/
const vector<string> UDPServer::SplitString(char* input_string, const char& delimiter, const int& no_of_splits,
	const int& input_string_length) {
	std::string current_word = EMPTY_STRING;
	int no_of_current_splits = 1;
	vector<string> output_vector;
	
	for (int i = 0; i < input_string_length; i++) {
		char current_char = *(input_string + i);
		if (current_char == delimiter && no_of_current_splits < no_of_splits) {
			output_vector.push_back(current_word);
			current_word = EMPTY_STRING;
			no_of_current_splits++;
		}
		else {
			current_word += current_char;
		}
	}

	if (current_word.length() > 0) {
		output_vector.push_back(current_word);
	}

	if (output_vector.size() < no_of_splits) {
		stringstream s_stream;
		s_stream << input_string;
		msg_logger_->LogError("ERROR: Unexpected split. Original string: " + s_stream.str());
	}
	return output_vector;
}

/*
Checks if timeoutDuration (in milliseconds) has passed between now and when the last message was received, given by lastMsgRecdTime.
*/
bool UDPServer::HasRequestTimedOut(high_resolution_clock::time_point& last_msg_recd_time, const u_short& timeout_duration)
{
	//Below snippet to calculate elapsed time taken from https://stackoverflow.com/a/31657669
	auto now = high_resolution_clock::now();
	auto time_elapsed_since_last_msg_recd = duration_cast<milliseconds>(now - last_msg_recd_time);

	if (time_elapsed_since_last_msg_recd.count() >= timeout_duration) {
		return true;
	}

	return false;
}

void UDPServer::ClearQueue(std::queue<string> &q)
{
	//Below snippet to clear a queue taken from https://stackoverflow.com/a/709161
	std::queue<string> emptyQ;
	std::swap(q, emptyQ);
}

/*
Drains the client queue and keeps appending queue messages to msgInQueue
until the queue becomes empty or a message is encountered in the queue having '\0'
as its last character, whichever occurs earlier.
*/
u_short UDPServer::DrainQueue(std::queue<std::string>& client_queue, std::string& msg_in_queue)
{
	u_short bytes_recd = 0;

	while (!client_queue.empty() && !msg_in_queue.ends_with(STRING_TERMINATING_CHAR)) {
		msg_logger_->LogDebug("DrainQueue::Before popping from queue.");

		msg_in_queue += client_queue.front();
		msg_logger_->LogDebug("Msg in queue: " + msg_in_queue);
		client_queue.pop();
		bytes_recd += msg_in_queue.length();
	}

	return bytes_recd;
}

/*
Returns the sequence numbers present in the series 0 to expectedNumberOfPayloads
but not in the key set of receivedPayloadsMap. Basically returns the sequence numbers
that are not yet received from the client.
*/
vector<u_short> UDPServer::GetMissingPayloadSeqNumbers(const map<u_short, string>& recd_payloads_map, u_short expected_number_of_payloads)
{
	vector<u_short> missing_seq_numbers;

	for (u_short i = 1; i <= expected_number_of_payloads; i++) {
		if (recd_payloads_map.count(i) == 0) {
			missing_seq_numbers.push_back(i);
		}
	}
	return missing_seq_numbers;
}

void UDPServer::RemoveClientDataFromMap(const string& client_address_key)
{
	//Client cleanup
	mtx_.lock();
	ClearQueue(client_queue_map_[client_address_key]);
	client_queue_map_.erase(client_address_key);
	mtx_.unlock();
}

/*
Constructs a string key in a particular format using the client IP Address and port.
*/
const std::string UDPServer::MakeClientAddressKey(const sockaddr_in& client_address)
{
	return to_string(client_address.sin_addr.s_addr) + CLIENT_ADDRESS_KEY_DELIMITER + to_string(client_address.sin_port);
}

/*
Consumes all image metadata payloads from the client queue 
and initializes the image metadata - imageDimensions, filterType and filterParams
after successful validation.
*/
short UDPServer::InitializeImageMetadata(cv::Size& image_dimensions, uint& image_file_size, ImageFilterTypesEnum& filter_type,
	vector<float>& filter_params, std::queue<string>& client_queue)
{
	long last_payload_size = 0, bytes_recd = 0;
	string image_metadata_string = EMPTY_STRING;

	DrainQueue(client_queue, image_metadata_string);
	msg_logger_->LogDebug("Queue size after consuming image metadata: " + to_string((u_short)client_queue.size()));
	return ProcessImageMetadataPayload(&image_metadata_string[0], image_dimensions, image_file_size, filter_type, filter_params);
}

/*
Splits the raw image metadata string received from client by 'space' character
and forwards the resulting vector for validation.
*/
short UDPServer::ProcessImageMetadataPayload(char* received_data, cv::Size& image_dimensions, uint& image_file_size,
	ImageFilterTypesEnum& filter_type_enum, vector<float>& filter_params)
{
	stringstream s_stream;
	s_stream << received_data;
	msg_logger_->LogError("Image meta data recd from client: " + s_stream.str());

	const vector<std::string> split_image_metadata = SplitString(received_data, ' ');

	ImageMetadataProcessor image_metadata_processor(split_image_metadata);
	return image_metadata_processor.ValidateImageMetadata(image_dimensions, image_file_size, filter_type_enum, filter_params);
}

/*
Fragments image into payloads of 60,000 bytes and builds a map of the image data
in the format "<sequence_number>" -> "SEQ <sequence number> SIZE <payload size in bytes> <image data>" without the quotes.
This increases reliability by keeping track of which payloads were successfully received by the client.
It also helps retrieve the data of the payloads not received by the client so they can be re-sent.
*/
void UDPServer::BuildImageDataPayloadMap(Mat image, map<u_short, string>& image_data_payload_map,
	map<u_short, u_short>& sequence_no_to_payload_size_map, vector<u_short>& sequence_numbers)
{
	uint image_bytes_left = image.elemSize() * image.total();
	uint image_bytes_processed = 0;
	string payload;
	u_short payload_size;
	u_short payload_sequence_num = 1;
	auto image_ptr = image.data;

	while (image_bytes_left > 0) {

		u_short image_bytes_processed_this_iteration = 0;
		if (image_bytes_left >= MAX_IMAGE_DATA_BYTES_IN_ONE_PAYLOAD) {

			payload = string(SEQUENCE_PAYLOAD_KEY).append(SERVER_MSG_DELIMITER)
				.append(to_string(payload_sequence_num)).append(SERVER_MSG_DELIMITER)
				.append(SIZE_PAYLOAD_KEY).append(SERVER_MSG_DELIMITER)
				.append(to_string(MAX_IMAGE_DATA_BYTES_IN_ONE_PAYLOAD)).append(SERVER_MSG_DELIMITER);

			payload_size = payload.length();
			string image_data = string((char*)image_ptr + image_bytes_processed, MAX_IMAGE_DATA_BYTES_IN_ONE_PAYLOAD);
			payload += image_data;

			image_bytes_processed_this_iteration = MAX_IMAGE_DATA_BYTES_IN_ONE_PAYLOAD;
		}
		else {
			payload = string(SEQUENCE_PAYLOAD_KEY).append(SERVER_MSG_DELIMITER)
				.append(to_string(payload_sequence_num)).append(SERVER_MSG_DELIMITER)
				.append(SIZE_PAYLOAD_KEY).append(SERVER_MSG_DELIMITER)
				.append(to_string(image_bytes_left)).append(SERVER_MSG_DELIMITER);

			payload_size = payload.length();
			string image_data = string((char*)image_ptr + image_bytes_processed, image_bytes_left);
			payload += image_data;

			image_bytes_processed_this_iteration = image_bytes_left;
		}

		image_data_payload_map[payload_sequence_num] = payload;

		payload_size += image_bytes_processed_this_iteration;
		sequence_no_to_payload_size_map[payload_sequence_num] = payload_size;

		sequence_numbers.push_back(payload_sequence_num);
		image_bytes_processed += image_bytes_processed_this_iteration;
		image_bytes_left -= image_bytes_processed_this_iteration;

		payload_sequence_num++;
	}
}

/*
---------------------
Validation functions
---------------------
*/

/*
* This function validates the general client response.
* The expected format for this kind of response is "RES <responseCode>" without the quotes.
* Dependng on the circumstances, this response can have more parameters, but never less.
*/
short UDPServer::ValidateClientResponse(std::vector<std::string>& client_response_split, short& client_response_code)
{
	msg_logger_->LogDebug("Validating client response.");

	if (client_response_split.size() < MIN_CLIENT_RESPONSE_PARAMS || client_response_split.at(0) != RESPONSE_PAYLOAD_KEY) {
		msg_logger_->LogError("ERROR: Client response not in expected format. Split size: " + to_string((u_short)client_response_split.size())
			+ " | First element : " + client_response_split.at(0));
		return RESPONSE_FAILURE;
	}

	try {
		client_response_code = stoi(client_response_split.at(1));
	}
	catch (invalid_argument) {
		msg_logger_->LogError("ERROR: Response code not a number. Recd response code: " + client_response_split.at(1));
		return RESPONSE_FAILURE;
	}

	msg_logger_->LogDebug("Client response validation successful.");
	return RESPONSE_SUCCESS;
}

/*
* Validates the image data payload sent by the client.
* The expected format for this kind of message is: "SEQ <payload sequence number> SIZE <size of the image data in payload> <image data>"
* without the quotes.
*/
short UDPServer::ValidateImageDataPayload(std::vector<std::string>& split_image_data_payload, u_int& payload_seq_no, u_int& payload_size)
{
	if (split_image_data_payload.size() != NUM_OF_IMAGE_DATA_PARAMS || split_image_data_payload.at(0) != SEQUENCE_PAYLOAD_KEY
		|| split_image_data_payload.at(2) != SIZE_PAYLOAD_KEY) {
		msg_logger_->LogError("ERROR: Image data payload in incorrect format. First word: " + split_image_data_payload.at(0));
		return RESPONSE_FAILURE;
	}

	try {
		payload_seq_no = stoi(split_image_data_payload.at(1));
		payload_size = stoi(split_image_data_payload.at(3));
	}
	catch (invalid_argument) {
		msg_logger_->LogError("ERROR: Image data payload sequence num or size not an int. Seq num: " + split_image_data_payload.at(1)
			+ " | Size:" + split_image_data_payload.at(3));
		return RESPONSE_FAILURE;
	}

	return RESPONSE_SUCCESS;
}

/*
* --------------------------------
* Functions to send data to client
* --------------------------------
*/

/*
* Sends response to the client in the format "RES <response code> <missing payload sequence numbers (if any)>"
* without the quotes.
* This response helps the client to ensure that server has received the message/s succuessfully.
*/
short UDPServer::SendServerResponseToClient(short server_response_code, const sockaddr_in& client_address, const vector<u_short>* missing_seq_numbers)
{
	msg_logger_->LogDebug("Entered UDPServer::SendServerResponseToClient.");

	mtx_.lock();
	if (socket_ == INVALID_SOCKET) {
		msg_logger_->LogError("ERROR: Invalid socket.");
		mtx_.unlock();
		return RESPONSE_FAILURE;
	}
	mtx_.unlock();

	string missing_seq_numbers_string = EMPTY_STRING;
	if (missing_seq_numbers != nullptr) {
		for (const u_short& missing_seq_number : *missing_seq_numbers) {
			missing_seq_numbers_string.append(to_string(missing_seq_number)).append(SERVER_MSG_DELIMITER);
		}
	}
	string server_response_payload = string(RESPONSE_PAYLOAD_KEY).append(SERVER_MSG_DELIMITER)
		.append(to_string(server_response_code)).append(SERVER_MSG_DELIMITER)
		.append(missing_seq_numbers_string).append(EMPTY_STRING + STRING_TERMINATING_CHAR);

		msg_logger_->LogDebug("Server response string: " + server_response_payload + " | String length: " 
		+ to_string((u_short)server_response_payload.length()));

	//'\0' not counted in string.length(), hence adding 1 to the payload size parameter below.
	mtx_.lock();
	short bytes_sent = sendto(socket_, &server_response_payload[0], server_response_payload.length() + 1, 
		0, (const sockaddr*)&client_address, sizeof(client_address));
	mtx_.unlock();

	if (bytes_sent <= 0) {
		msg_logger_->LogError("Error while sending acknowldgement to client. Error code: " + to_string(WSAGetLastError()));
		return RESPONSE_FAILURE;
	}
	return RESPONSE_SUCCESS;
}

/*
This function constructs a vector of the payload sequence numbers that the server did not receive
from the client before the last timeout occurred. It then sends these sequence numbers along with
a negative response code to the client so client can re-send the missing payloads.
*/
short UDPServer::SendMissingPayloadSeqNumbersToClient(std::map<u_short, std::string>& image_payload_seq_map, 
	const u_short& expected_no_of_payloads,
	vector<u_short>& missing_payload_seq_nums_in_last_timeout, const sockaddr_in& client_address)
{
	short response_code;
	vector<u_short> missing_seq_nums_in_this_timeout = GetMissingPayloadSeqNumbers(image_payload_seq_map, expected_no_of_payloads);

	if (missing_seq_nums_in_this_timeout.size() > 0) {
		if (missing_payload_seq_nums_in_last_timeout == missing_seq_nums_in_this_timeout) {
			//Client did not send any more payloads since last timeout. Assuming that it is inactive now.
			msg_logger_->LogError("Client is inactive.");
			return RESPONSE_FAILURE;
		}
		missing_payload_seq_nums_in_last_timeout = missing_seq_nums_in_this_timeout;
		response_code = SendServerResponseToClient(SERVER_NEGATIVE_ACK, client_address, &missing_seq_nums_in_this_timeout);
	}
	else {
		response_code = RESPONSE_SUCCESS;
	}

	if (response_code == RESPONSE_FAILURE) {
		msg_logger_->LogError("ERROR: Could not send response to client.");
		return RESPONSE_FAILURE;
	}

	return RESPONSE_SUCCESS;
}

/*
Constructs the image metadata string in the format "SIZE <image width> <image height> <image size in bytes>\0"
without the quotes. This function is called after server has finished applying the requested filter on the original
image and is ready to send the resultant image back to the client.
*/
short UDPServer::SendImageMetadataToClient(const Mat& image, const sockaddr_in& client_address)
{
	std::string image_metadata_payload = string(SIZE_PAYLOAD_KEY).append(SERVER_MSG_DELIMITER)
		.append(to_string(image.cols)).append(SERVER_MSG_DELIMITER)
		.append(to_string(image.rows)).append(SERVER_MSG_DELIMITER)
		.append(to_string(image.total() * image.elemSize())).append("\0");

	mtx_.lock();

	if (socket_ == INVALID_SOCKET) {
		msg_logger_->LogError("ERROR: Invalid client socket.");
		mtx_.unlock();
		return RESPONSE_FAILURE;
	}
	mtx_.unlock();

	u_short payload_size = image_metadata_payload.length() + 1; //1 added for \0 character at the end
	msg_logger_->LogDebug("Image metadata payload before sending: " + image_metadata_payload + " | Size: " + to_string(payload_size));

	int bytes_sent = 0;

	while (bytes_sent < payload_size) {

		mtx_.lock();
		int bytes_sent_this_iteration = sendto(socket_, &image_metadata_payload[0] + bytes_sent, payload_size - bytes_sent,
			0, (sockaddr*)&client_address, sizeof(client_address));
		mtx_.unlock();

		if (bytes_sent_this_iteration <= 0) {
			msg_logger_->LogError("Error while sending image size. Error code: " + to_string(WSAGetLastError()));
			return RESPONSE_FAILURE;
		}
		bytes_sent += bytes_sent_this_iteration;
	}

	msg_logger_->LogError("Image metadata successfully sent to client.");
	return RESPONSE_SUCCESS;
}

/*
This function builds the payload map for imageToSend and sends all image payloads
to the client. It also listens for client response after sending each batch of payloads 
to ascertain the ones lost in transit. These payloads are then re-sent in the next iteration, 
and the process is repeated until a positive acknowledgment is received from the client, or 
the client disconnects, whichever occurs earlier.
*/
short UDPServer::SendImage(const cv::Mat& image_to_send, const sockaddr_in& client_address, std::queue<std::string>& client_queue)
{
	map<u_short, string> image_data_payload_map;
	map<u_short, u_short> seq_no_to_payload_size_map;
	vector<u_short> payload_seq_nums_to_send;
	short client_response_code = CLIENT_NEGATIVE_ACK;

	BuildImageDataPayloadMap(image_to_send, image_data_payload_map, seq_no_to_payload_size_map, payload_seq_nums_to_send);

	while (client_response_code != CLIENT_POSITIVE_ACK) {

		short response_code = SendImageDataPayloadsBySequenceNumbers(image_data_payload_map, seq_no_to_payload_size_map,
			payload_seq_nums_to_send, client_address);
		if (response_code == RESPONSE_FAILURE) {
			msg_logger_->LogError("ERROR: Could not send image payloads to client.");
			return RESPONSE_FAILURE;
		}

		vector<string> client_response_split;
		response_code = ConsumeAndValidateClientMsgFromQueue(client_queue, client_response_split, client_response_code);
		if (response_code == RESPONSE_FAILURE) {
			msg_logger_->LogError("Error while receiving/validating client response.");
			return RESPONSE_FAILURE;
		}

		if (client_response_code == CLIENT_NEGATIVE_ACK) {
			payload_seq_nums_to_send.clear();
			for (int i = 2; i < client_response_split.size(); i++) {
				try {
					payload_seq_nums_to_send.push_back(stoi(client_response_split.at(i)));
				}
				catch (invalid_argument) {
					msg_logger_->LogError("ERROR: Sequence number sent by client not a number. Received sequence number: " 
						+ client_response_split.at(i));
				}
			}
		}
	}

	msg_logger_->LogError("All image payloads received by client.");

	return RESPONSE_SUCCESS;
}

/*
This function fetches the image data payload corresponding to the numbers in payloadSeqNumbersToSend from imageDataPayloadMap
and sends them to the client.
*/
short UDPServer::SendImageDataPayloadsBySequenceNumbers(map<u_short, string>& image_data_payload_map,
	map<u_short, u_short>& seq_no_to_payload_size_map, const vector<u_short>& payload_seq_nums_to_send, const sockaddr_in& client_address)
{
	for (u_short payload_seq_num_to_send : payload_seq_nums_to_send) {
		char* payload_to_send = &(image_data_payload_map[payload_seq_num_to_send][0]);

		mtx_.lock();
		int send_result = sendto(socket_, payload_to_send, seq_no_to_payload_size_map[payload_seq_num_to_send],
			0, (const sockaddr*)&client_address, sizeof(client_address));
		mtx_.unlock();

		if (send_result == SOCKET_ERROR) {
			msg_logger_->LogError("ERROR while sending image payload to client. Error code " + to_string(WSAGetLastError()));
			return RESPONSE_FAILURE;
		}

		//Log message for debugging
		//vector<string> payloadSplit = SplitString(payloadToSend, ' ', 5, sequenceNumToPayloadSizeMap[payloadSeqNumberToSend]);

		/*_msgLogger->LogDebug("Sent payload #" + to_string(payloadSeqNumberToSend)
				+ " | Payload split: " + payloadSplit.at(0) + " | " + payloadSplit.at(1) + " | " + payloadSplit.at(2) + " | " + payloadSplit.at(3)
				+ " | Length of image payload: " + to_string((ushort)payloadSplit.at(4).length()));*/

		msg_logger_->LogDebug("Sent payload #" + to_string(payload_seq_num_to_send));
	}

	msg_logger_->LogDebug("Image payloads sent to client.");
	return RESPONSE_SUCCESS;
}

/*
--------------------------------------
Functions to receive data from client
--------------------------------------
*/

/*
This functions drains the queue until a message is found having '\0' as its last character.
While consuming from the queue, it checks for timeout and returns if a timeout does occur,
assuming that the client is not active anymore.
Upon succesfully receiving a properly terminated message, it splits the raw string by 'space'
character and sends the resulting vector for validation.
*/
short UDPServer::ConsumeAndValidateClientMsgFromQueue(std::queue<std::string>& client_queue,
	std::vector<cv::String>& client_response_split, short& client_response_code)
{
	string client_response_raw = EMPTY_STRING;
	short response_code = RESPONSE_SUCCESS;
	auto last_msg_consumed_time = high_resolution_clock::now();

	while (!client_response_raw.ends_with(STRING_TERMINATING_CHAR)) {
		u_short bytes_recd_this_iteration = DrainQueue(client_queue, client_response_raw);
		if (bytes_recd_this_iteration > 0) {
			last_msg_consumed_time = high_resolution_clock::now();
			continue;
		}
		if (HasRequestTimedOut(last_msg_consumed_time, CLIENT_MSG_RECV_TIMEOUT_MILLIS)) {
			//Client is inactive.
			msg_logger_->LogError("ERROR: Server timed out while waiting for client response.");
			return RESPONSE_FAILURE;
		}
	}

	msg_logger_->LogDebug("Response from client: " + client_response_raw);
	client_response_split = SplitString(&client_response_raw[0], CLIENT_RESPONSE_DELIMITER);

	response_code = ValidateClientResponse(client_response_split, client_response_code);
	if (response_code == RESPONSE_FAILURE) {
		msg_logger_->LogError("ERROR: Validation failed for client response.");
		return RESPONSE_FAILURE;
	}
	return RESPONSE_SUCCESS;
}

/*
This function consumes the image data from client queue until no more bytes are left to receive. The bytes left to receive
are calculated from an earlier client message. While popping from the queue, the function also checks if timeout has occurred
every time it finds the queue empty. If a timeout does occur, it assumes that the client is not active anymore and returns.
Each raw image payload is then split by 'space' character and validated, before updating the imagePayloadSeqMap with the newly
consumed payload.
*/
short UDPServer::ConsumeImageDataFromClientQueue(std::queue<std::string>& client_queue, std::map<u_short, std::string>& image_payload_seq_map,
	const u_short& expected_no_of_payloads, const sockaddr_in& client_address, long& image_bytes_left_to_receive)
{
	long image_bytes_recd = 0;
	short response_code = RESPONSE_FAILURE;
	auto last_image_payload_recd_time = high_resolution_clock::now();
	vector<u_short> missing_payload_seq_nums;

	while (image_bytes_left_to_receive > 0) {

		if (client_queue.empty()) {

			if (HasRequestTimedOut(last_image_payload_recd_time, IMAGE_PAYLOAD_RECV_TIMEOUT_MILLIS)) {
				response_code = SendMissingPayloadSeqNumbersToClient(image_payload_seq_map, expected_no_of_payloads,
					missing_payload_seq_nums, client_address);
				last_image_payload_recd_time = high_resolution_clock::now();
				if (response_code == RESPONSE_FAILURE) {
					msg_logger_->LogError("Error while sending response to client on timeout.");
					return RESPONSE_FAILURE;
				}
			}
			continue;
		}

		if (client_queue.front().length() < MIN_CLIENT_PAYLOAD_SIZE_BYTES) {
			msg_logger_->LogError("ERROR: Unexpected msg found in queue. Msg: " + client_queue.front());
			client_queue.pop();
		}

		msg_logger_->LogDebug("Queue msg size before splitting: " + to_string((u_short)client_queue.front().length()));

		vector<string> split_image_data_payload = SplitString(&(client_queue.front()[0]), CLIENT_RESPONSE_DELIMITER, NUM_OF_IMAGE_DATA_PARAMS,
			client_queue.front().length());

		msg_logger_->LogDebug("Split image payload size: " + to_string((u_short)split_image_data_payload.size()));

		u_int payload_seq_num = 0, payload_size = 0;

		response_code = ValidateImageDataPayload(split_image_data_payload, payload_seq_num, payload_size);
		if (response_code == RESPONSE_FAILURE) {
			msg_logger_->LogError("ERROR: Validation failed for image data payload.");
			return response_code;
		}

		image_payload_seq_map[payload_seq_num] = split_image_data_payload.at(4);

		msg_logger_->LogDebug("Image data after splitting: " + split_image_data_payload.at(0) + " | " + split_image_data_payload.at(1) + " | "
			+ split_image_data_payload.at(2) + " | " + split_image_data_payload.at(3)
			+ " | Length of image data: " + to_string((u_short)split_image_data_payload.at(4).length()));

		client_queue.pop();

		msg_logger_->LogDebug("After popping image data payload from queue. Queue size: " + to_string((u_short)client_queue.size()));

		image_bytes_recd += payload_size;
		image_bytes_left_to_receive -= payload_size;

		msg_logger_->LogDebug("Image bytes recd: " + to_string(image_bytes_recd)
			+ " | image bytes left to receive: " + to_string(image_bytes_left_to_receive));

		last_image_payload_recd_time = high_resolution_clock::now();
	}

	return RESPONSE_SUCCESS;
}

/*
This function runs in the main server thread and continuously listens for incoming messages.
For every new client, a separate queue is created and all subsequent messages from that client
are enqueued in the corresponding queue. The connected clients are tracked through a map, with 
the IP address and port of each client acting as a unique key, and its corresponding queue as its value.
All further processing for the client is done in a separate thread managed by the thread pool.
*/
short UDPServer::StartReceivingClientMsgs()
{
	if (!IsValid()) {
		msg_logger_->LogError("ERROR: Invalid socket.");
		return RESPONSE_FAILURE;
	}

	//Created local thread pool so threads will be joined before UDP Server gets destroyed.
	ThreadPool thread_pool(NUM_THREADS);
	sockaddr_in* client_address = new sockaddr_in;

	while (true) {
		char* received_data = new char[MAX_CLIENT_PAYLOAD_SIZE_BYTES];
		int client_addr_size = sizeof(*client_address);

		long bytes_recd_this_iteration = recvfrom(socket_, received_data, MAX_CLIENT_PAYLOAD_SIZE_BYTES, 0, (sockaddr*)client_address, &client_addr_size);

		if (bytes_recd_this_iteration == SOCKET_ERROR) {
			int last_error = WSAGetLastError();

			if (last_error != WSAEWOULDBLOCK) {
				msg_logger_->LogError("Error in receiving data from client. Error code : " + to_string(last_error));
				SendServerResponseToClient(SERVER_NEGATIVE_ACK, *client_address, nullptr);
				RemoveClientDataFromMap(MakeClientAddressKey(*client_address));
				delete client_address;
				client_address = new sockaddr_in;
			}
		}
		else {
			msg_logger_->LogDebug("Image data recd from client.");
			string client_address_key = MakeClientAddressKey(*client_address);

			mtx_.lock();
			if (client_queue_map_.count(client_address_key) <= 0) {
				msg_logger_->LogDebug("Map entry not found for client address key: " + client_address_key);
				queue<string> new_client_queue;
				client_queue_map_[client_address_key] = new_client_queue;
				thread_pool.Enqueue(bind(&UDPServer::ProcessImageReq, this, *client_address));
			}
			
			string image_data = string(received_data, bytes_recd_this_iteration);
			client_queue_map_[client_address_key].push(image_data);
			mtx_.unlock();
			
			msg_logger_->LogDebug("Image data size: " + to_string((u_short) image_data.length()));
			msg_logger_->LogDebug("Image data pushed to client queue. bytesRecdThisIterationString: " + to_string(bytes_recd_this_iteration));
		}
		delete[] received_data;
	}

	delete client_address;
	client_address = nullptr;
}

bool UDPServer::IsValid()
{
	mtx_.lock();
	if (socket_ != INVALID_SOCKET) {
		mtx_.unlock();
		return true;
	}
	mtx_.unlock();
	return false;
}