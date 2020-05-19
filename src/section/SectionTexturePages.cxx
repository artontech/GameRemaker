#include "GMFile.h"
#include "utils.h"

uint32_t SectionTexturePages::calcSize(GMFile* gmf, uint32_t offset) {
	this->offset = offset;

	// Calc size
	uint32_t data_offset = offset + sizeof(uint32_t) + sizeof(uint32_t) * count;

	for (int i = 0; i < count; i++) {
		offsets.push_back(data_offset);
		data_offset += sizeof(TexturePageInfo);
	}

	data_offset += Util::align(data_offset, 4);
	header.size = data_offset - offset;

	return header.size;
}

bool SectionTexturePages::fromFile(GMFile* gmf, Header &h, FILE* f, uint32_t offset) {
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

		TexturePageInfo info;
		fread(&info, sizeof(TexturePageInfo), 1, f);

		infos.push_back(info);
	}

	return true;
}
bool SectionTexturePages::fromDir(GMFile* gmf, Header &h, string section_path) {
	header = h;

	string json_file = Util::join(section_path, "data.json");

	rapidjson::Document document;

	ifstream fin(json_file, ios::binary);
	if (!fin.is_open()) {
		cout << "File not found: " << json_file << endl;
		return false;
	}

	rapidjson::IStreamWrapper isw(fin);
	document.ParseStream(isw);

	// Load data
	auto &list = document.GetArray();
	count = list.Size();
	for (int i = 0; i < count; i++) {
		auto &page = list[i];
		TexturePageInfo info;

		{
			auto &obj = page["src"].GetObject();

			info.src.x = obj["x"].GetUint();
			info.src.y = obj["y"].GetUint();
			info.src.width = obj["width"].GetUint();
			info.src.height = obj["height"].GetUint();
		}
		{
			auto &obj = page["dst"].GetObject();

			info.dst.x = obj["x"].GetUint();
			info.dst.y = obj["y"].GetUint();
			info.dst.width = obj["width"].GetUint();
			info.dst.height = obj["height"].GetUint();
		}
		
		info.size.x = page["width"].GetUint();
		info.size.y = page["height"].GetUint();

		info.spriteSheetId = page["spriteSheetId"].GetUint();

		infos.push_back(info);
	}

	return true;
}

bool SectionTexturePages::toFile(GMFile* gmf, FILE* f) {
	fseek(f, offset, 0);

	fwrite(&count, sizeof(uint32_t), 1, f);

	// Write offset
	for (int i = 0; i < count; i++) {
		fwrite(&offsets[i], sizeof(uint32_t), 1, f);
	}

	// Write data
	for (int i = 0; i < count; i++) {
		fseek(f, offsets[i], 0);
		fwrite(&infos[i], sizeof(TexturePageInfo), 1, f);
	}

	return true;
}

bool SectionTexturePages::toDir(GMFile* gmf, string section_path) const {
	string json_file = Util::join(section_path, "data.json");

	FILE* fp = fopen(json_file.c_str(), "wb");

	char writeBuffer[65536];
	rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));

	rapidjson::Writer<rapidjson::FileWriteStream> writer(os);

	writer.StartArray();
	for (int i = 0; i < infos.size(); i++) {
		TexturePageInfo info = infos[i];
		writer.StartObject();

		writer.Key("id");
		writer.Int(i);

		writer.Key("src");
		{
			writer.StartObject();

			writer.Key("x");
			writer.Uint(info.src.x);

			writer.Key("y");
			writer.Uint(info.src.y);

			writer.Key("width");
			writer.Uint(info.src.width);

			writer.Key("height");
			writer.Uint(info.src.height);

			writer.EndObject();
		}

		writer.Key("dst");
		{
			writer.StartObject();

			writer.Key("x");
			writer.Uint(info.dst.x);

			writer.Key("y");
			writer.Uint(info.dst.y);

			writer.Key("width");
			writer.Uint(info.dst.width);

			writer.Key("height");
			writer.Uint(info.dst.height);

			writer.EndObject();
		}

		writer.Key("width");
		writer.Uint(info.size.x);

		writer.Key("height");
		writer.Uint(info.size.y);

		writer.Key("spriteSheetId");
		writer.Uint(info.spriteSheetId);

		writer.EndObject();
	}
	writer.EndArray();

	fclose(fp);

	return true;
}

bool SectionTexturePages::linkFrom(GMFile* gmf) {
	return true;
}

bool SectionTexturePages::linkTo(GMFile* gmf, FILE* f) {
	return true;
}

bool SectionTexturePages::getPageByOffset(const uint32_t offset, int& id, TexturePageInfo& info) const {
	for (int i = 0; i < count; i++) {
		if (offset == offsets[i]) {
			id = i;
			info = infos[i];
			return true;
		}
	}
	return false;
}

uint32_t SectionTexturePages::getOffset(const int id) const {
	return offsets[id];
}
