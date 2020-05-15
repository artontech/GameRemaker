

#include <sstream>

#include "GMFile.h"
#include "utils.h"

string Header::getName() const {
	Parser32 parser;
	parser.uint32 = name;
	parser.chr[4] = 0;
	return parser.chr;
}

string Section::toString() const {
	stringstream ss("name: ");
	ss << header.getName() << ", size: " << header.size;
	return ss.str();
}

void operator|=(SoundFlag &self, const SoundFlag &f) {
	self = (SoundFlag)(self | f);
}
