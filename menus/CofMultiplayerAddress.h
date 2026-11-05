#pragma once
#include <cstddef>
#include <cstdio>
#include <cstring>

namespace CofMultiplayer {
// Accept one IPv4 endpoint, never a console fragment. Pasted padding is harmless.
inline bool Address(const char *input, char *output, std::size_t capacity)
{
	if (!output || !capacity) return false;
	output[0] = 0;
	if (!input) return false;
	std::size_t length = 0;
	while (length < 128 && input[length]) ++length;
	if (!length || length == 128) return false;
	std::size_t begin = 0, end = length;
	while (begin < end && (input[begin] == ' ' || input[begin] == '\t')) ++begin;
	while (end > begin && (input[end - 1] == ' ' || input[end - 1] == '\t')) --end;
	unsigned octets[4] = {}, port = 27015;
	std::size_t pos = begin;
	for (unsigned part = 0; part < 4; ++part) {
		unsigned digits = 0;
		while (pos < end && input[pos] >= '0' && input[pos] <= '9') {
			if (++digits > 3) return false;
			octets[part] = octets[part] * 10 + unsigned(input[pos++] - '0');
		}
		if (!digits || octets[part] > 255) return false;
		if (part != 3 && (pos == end || input[pos++] != '.')) return false;
	}
	if (pos < end) {
		if (input[pos++] != ':') return false;
		port = 0;
		unsigned digits = 0;
		while (pos < end && input[pos] >= '0' && input[pos] <= '9') {
			if (++digits > 5) return false;
			port = port * 10 + unsigned(input[pos++] - '0');
		}
		if (!digits || !port || port > 65535) return false;
	}
	if (pos != end || !octets[0] || octets[0] >= 224) return false;
	const int written = std::snprintf(output, capacity, "%u.%u.%u.%u:%u",
		octets[0], octets[1], octets[2], octets[3], port);
	if (written < 0 || std::size_t(written) >= capacity) { output[0] = 0; return false; }
	return true;
}
}
