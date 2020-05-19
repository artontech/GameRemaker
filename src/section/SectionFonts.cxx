#include "GMFile.h"
#include "utils.h"

uint32_t SectionFonts::calcSize(GMFile* gmf, uint32_t offset) {
	this->offset = offset;

	// Calc size
	uint32_t data_offset = offset + sizeof(uint32_t) + sizeof(uint32_t) * count;

	for (int i = 0; i < count; i++) {
		offsets.push_back(data_offset);
		data_offset += sizeof(FontEntry) + (sizeof(uint32_t) + sizeof(FontCharacter)) * infos[i].entry.charsCount;
	}

	header.size = data_offset - offset + 512;

	return header.size;
}

bool SectionFonts::fromFile(GMFile* gmf, Header &h, FILE* f, uint32_t offset) {
	header = h;

	fseek(f, offset, 0);

	fread(&count, sizeof(uint32_t), 1, f);

	// Read offset
	for (int i = 0; i < count; i++) {
		uint32_t info_offset;
		fread(&info_offset, sizeof(uint32_t), 1, f);
		offsets.push_back(info_offset);
	}

	// Read infos
	for (int i = 0; i < count; i++) {
		fseek(f, offsets[i], 0);

		FontInfo info;
		fread(&info.entry, sizeof(FontEntry), 1, f);

		// Read characters
		uint32_t* char_offsets = new uint32_t[info.entry.charsCount];
		fread(char_offsets, sizeof(uint32_t), info.entry.charsCount, f);
		for (int j = 0; j < info.entry.charsCount; j++) {
			fseek(f, char_offsets[j], 0);
			FontCharacter ch;
			fread(&ch, sizeof(FontCharacter), 1, f);

			info.characters.push_back(ch);
		}
		delete char_offsets;

		infos.push_back(info);
	}

	return true;
}
bool SectionFonts::fromDir(GMFile* gmf, Header &h, string section_path) {
	header = h;

	vector<string> paths;

	// Enum file
	for (auto& fe : filesystem::directory_iterator(section_path)) {
		filesystem::path fp = fe.path(), fn = fp.filename();
		if (!filesystem::is_directory(fe) && fn.extension() == ".json") {
			paths.push_back(fp.string());
		}
	}
	count = paths.size();

	// Init infos
	for (int i = 0; i < count; i++) {
		FontInfo info;
		infos.push_back(info);
	}

	// Load data
	for (const string& path : paths) {
		rapidjson::Document document;

		ifstream fin(path, ios::binary);
		if (!fin.is_open()) {
			cout << "File not found: " << path << endl;
			return false;
		}

		rapidjson::IStreamWrapper isw(fin);
		document.ParseStream(isw);

		// Load data
		int id = document["id"].GetInt();
		FontInfo &info = infos[id];

		info.entry.emSize = document["emSize"].GetUint();
		info.entry._unknown = document["_unknown"].GetUint();
		info.entry.charset = document["charset"].GetUint();
		info.entry.antiAliasing = document["antiAliasing"].GetUint();
		info.entry._ignore = 0xFF1F;
		info.entry.scale.x = document["scaleX"].GetFloat();
		info.entry.scale.y = document["scaleY"].GetFloat();

		info.codeNameID = document["codeNameID"].GetInt();
		info.systemNameID = document["systemNameID"].GetInt();
		info.isBold = document["isBold"].GetBool();
		info.IsItalic = document["IsItalic"].GetBool();
		info.texturePageId = document["texturePageId"].GetInt();

		auto list = document["chars"].GetArray();
		info.entry.charsCount = list.Size();
		for (int i = 0; i < list.Size(); i++) {
			auto &obj = list[i];

			FontCharacter ch;
			string str(obj["char"].GetString());
			wstring wstr = Util::UTF8ToUnicode(str);
			ch.character = wstr[0];

			ch.texturePageFrame.x = obj["x"].GetUint();
			ch.texturePageFrame.y = obj["y"].GetUint();
			ch.texturePageFrame.width = obj["width"].GetUint();
			ch.texturePageFrame.height = obj["height"].GetUint();

			ch.shift = obj["shift"].GetUint();
			ch.offset = obj["offset"].GetUint();

			info.characters.push_back(ch);
		}
	}

	return true;
}

bool SectionFonts::toFile(GMFile* gmf, FILE* f) {
	fseek(f, offset, 0);

	fwrite(&count, sizeof(uint32_t), 1, f);

	// Write offset
	for (int i = 0; i < count; i++) {
		fwrite(&offsets[i], sizeof(uint32_t), 1, f);
	}

	// Don't know why
	fseek(f, offset + header.size - 512, 0);
	for (uint16_t c = 0; c < 0x80; c++) {
		fwrite(&c, sizeof(uint16_t), 1, f);
	}
	const uint16_t c = 0x3F;
	for (int i = 0; i < 0x80; i++) {
		fwrite(&c, sizeof(uint16_t), 1, f);
	}

	return true;
}

bool SectionFonts::toDir(GMFile* gmf, string section_path) const {
	for (int i = 0; i < infos.size(); i++) {
		FontInfo info = infos[i];
		string json_file = Util::join(section_path, info.codeName + ".json");

		FILE* fp = fopen(json_file.c_str(), "wb");

		char writeBuffer[65536];
		rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));

		// rapidjson::Writer<rapidjson::FileWriteStream, rapidjson::Document::EncodingType, rapidjson::ASCII<>> writer(os);
		rapidjson::Writer<rapidjson::FileWriteStream> writer(os);
		writer.StartObject();

		writer.Key("id");
		writer.Int(i);

		writer.Key("emSize");
		writer.Uint(info.entry.emSize);

		writer.Key("_unknown");
		writer.Uint(info.entry._unknown);

		writer.Key("charset");
		writer.Uint(info.entry.charset);

		writer.Key("antiAliasing");
		writer.Uint(info.entry.antiAliasing);

		writer.Key("scaleX");
		writer.Double(info.entry.scale.x);

		writer.Key("scaleY");
		writer.Double(info.entry.scale.y);

		writer.Key("codeNameID");
		writer.Int(info.codeNameID);

		writer.Key("systemName");
		writer.String(info.systemName.c_str());

		writer.Key("systemNameID");
		writer.Int(info.systemNameID);

		writer.Key("isBold");
		writer.Bool(info.isBold);

		writer.Key("IsItalic");
		writer.Bool(info.IsItalic);

		writer.Key("texturePageId");
		writer.Int(info.texturePageId);

		writer.Key("chars");
		writer.StartArray();
		for (FontCharacter ch : info.characters) {
			writer.StartObject();

			wstring str;
			str = ch.character;

			writer.Key("char");
			writer.String(Util::unicodeToUTF8(str).c_str());

			writer.Key("x");
			writer.Uint(ch.texturePageFrame.x);

			writer.Key("y");
			writer.Uint(ch.texturePageFrame.y);

			writer.Key("width");
			writer.Uint(ch.texturePageFrame.width);

			writer.Key("height");
			writer.Uint(ch.texturePageFrame.height);

			writer.Key("shift");
			writer.Uint(ch.shift);

			writer.Key("offset");
			writer.Uint(ch.offset);

			writer.EndObject();
		}
		writer.EndArray();

		writer.EndObject();

		fclose(fp);
	}

	return true;
}

bool SectionFonts::linkFrom(GMFile* gmf) {
	SectionStrings* sectionStrings = (SectionStrings*)gmf->getSection(HEADER_STRINGS);
	SectionTexturePages* sectionTexturePages = (SectionTexturePages*)gmf->getSection(HEADER_TEXTURE_PAGES);
	TexturePageInfo tpage;

	for (FontInfo& info : infos) {
		if (!sectionStrings->getStringByOffset(info.entry.codeName, info.codeNameID, info.codeName)) {
			cout << "No valid code name found for font " << info.entry.codeName << endl;
			return false;
		}
		if (!sectionStrings->getStringByOffset(info.entry.systemName, info.systemNameID, info.systemName)) {
			cout << "No valid system name found for font " << info.entry.systemName << endl;
			return false;
		}

		info.isBold = (info.entry.bold == 1);
		info.IsItalic = (info.entry.italic == 1);

		if (!sectionTexturePages->getPageByOffset(info.entry.texturePageOffset, info.texturePageId, tpage)) {
			cout << "No valid texture page found for font " << info.entry.texturePageOffset << endl;
			return false;
		}
	}

	return true;
}

bool SectionFonts::linkTo(GMFile* gmf, FILE* f) {
	SectionStrings* sectionStrings = (SectionStrings*)gmf->getSection(HEADER_STRINGS);
	SectionTexturePages* sectionTexturePages = (SectionTexturePages*)gmf->getSection(HEADER_TEXTURE_PAGES);

	// Write fonts
	for (int i = 0; i < count; i++) {
		FontInfo &info = infos[i];
		fseek(f, offsets[i], 0);
		info.entry.codeName = sectionStrings->getOffset(info.codeNameID);
		info.entry.systemName = sectionStrings->getOffset(info.systemNameID);

		info.entry.bold = info.isBold;
		info.entry.italic = info.IsItalic;

		info.entry.texturePageOffset = sectionTexturePages->getOffset(info.texturePageId);

		fwrite(&info.entry, sizeof(FontEntry), 1, f);

		// Write offsets
		uint32_t char_offset = offsets[i] + sizeof(FontEntry) + sizeof(uint32_t) * info.entry.charsCount;
		for (int j = 0; j < info.entry.charsCount; j++) {
			fwrite(&char_offset, sizeof(uint32_t), 1, f);
			char_offset += sizeof(FontCharacter);
		}

		// Write characters
		for (int j = 0; j < info.entry.charsCount; j++) {
			fwrite(&info.characters[j], sizeof(FontCharacter), 1, f);
		}
	}

	return true;
}
