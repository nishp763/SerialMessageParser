int read(char* buffer, unsigned int count)
{
	return 0;
}

void process_message(char* buffer, unsigned int message_id)
{
	return;
}

// for now message size = (message id * 5bytes) + 4 byte checksum + 1 byte separator
// note message header size is not included
int get_message_size_from_message_id(int message_id)
{
	// for testing/simplicity the message_id represents the number of repeated payload unit
	size_t numberOfPayloadBytes = message_id * 5;
	return numberOfPayloadBytes + 4 + 1;
}