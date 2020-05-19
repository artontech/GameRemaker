#include <filesystem>

#include "GMFile.h"
#include "utils.h"

uint32_t SectionTextures::calcSize(GMFile* gmf, uint32_t offset) {
	this->offset = offset;

	// Calc size
	uint32_t data_offset = offset + sizeof(uint32_t) + sizeof(uint32_t) * count * 3; // including offset of offsets

	// Calc png file offset
	for (PNGFile* png_file : png_files) {
		data_offset += Util::align(data_offset, 128);

		offsets.push_back(data_offset); // record offset for toFile()

		data_offset += png_file->length;
	}

	header.size = data_offset - offset + 3; // 3 bytes padding

	return header.size;
}

bool SectionTextures::fromFile(GMFile* gmf, Header &h, FILE* f, uint32_t offset) {
	header = h;

	fseek(f, offset, 0);

	fread(&count, sizeof(uint32_t), 1, f);

	// Read offset
	for (int i = 0; i < count; i++) {
		uint32_t offset_offset;
		fread(&offset_offset, sizeof(uint32_t), 1, f);
		offsets.push_back(offset_offset);
	}

	// Read textures
	for (int i = 0; i < count; i++) {
		fseek(f, offsets[i], 0);

		TextureEntry entry;
		fread(&entry, sizeof(TextureEntry), 1, f);

		uint32_t data_offset = NULL;
		if (entry.padOrOffset != NULL) {
			data_offset = entry.padOrOffset;
		}
		else if (entry.offset != NULL) {
			data_offset = entry.offset;
		}
		else {
			cout << "Texture " << i << " has no PNG data!" << endl;
			return false;
		}

		// read header
		fseek(f, data_offset, 0);
		PNGFile* png_file = new PNGFile();
		fread(&png_file->header, sizeof(PNGHeader), 1, f);
		
		uint32_t chunk_offset = data_offset + 8 + Util::swap(png_file->header.chunk.length) + 0xC; // second chunk
		PNGChunk chunk = png_file->header.chunk;
		while (chunk.type != PNG_CHUNK_IEND) {
			fseek(f, chunk_offset, 0);
			fread(&chunk, sizeof(PNGChunk), 1, f);
			chunk_offset += Util::swap(chunk.length) + 0xC;
		}

		png_file->length = chunk_offset - data_offset;
		png_file->data = new uint8_t[png_file->length];
		fseek(f, data_offset, 0);
		fread(png_file->data, sizeof(uint8_t), png_file->length, f);

		png_files.push_back(png_file);
	}

	return true;
}

bool SectionTextures::fromDir(GMFile* gmf, Header &h, string section_path) {
	header = h;

	vector<int> ids;

	// Enum file
	for (auto& fe : filesystem::directory_iterator(section_path)) {
		filesystem::path fp = fe.path(), fn = fp.filename();
		if (!filesystem::is_directory(fe) && fn.extension() == ".png") {
			ids.push_back(stoi(fn.replace_extension()));
		}
	}
	sort(ids.begin(), ids.end());
	count = ids.size();

	// Load data
	for (const int& i : ids) {
		PNGFile* png_file = new PNGFile();

		string data_file = Util::join(section_path, to_string(i) + ".png");
		FILE* fp = fopen(data_file.c_str(), "rb");

		fseek(fp, 0L, SEEK_END);
		png_file->length = ftell(fp);
		rewind(fp);

		png_file->data = new uint8_t[png_file->length];
		fread(png_file->data, sizeof(uint8_t), png_file->length, fp);

		png_files.push_back(png_file);

		fclose(fp);
	}

	return true;
}

bool SectionTextures::toFile(GMFile* gmf, FILE* f) {
	fseek(f, offset, 0);

	fwrite(&count, sizeof(uint32_t), 1, f);

	// Calc offset of offsets
	uint32_t* buffer = new uint32_t[count * 2];
	uint32_t offset_offset = offset + sizeof(uint32_t) + sizeof(uint32_t) * count;
	for (int i = 0; i < count; i++) {
		buffer[i] = offset_offset + 2 * sizeof(uint32_t) * i;
	}
	fwrite(buffer, sizeof(uint32_t), count, f);

	// Write pre-calc png file offsets
	for (int i = 0; i < count; i++) {
		buffer[2 * i] = 0x01; // pad
		buffer[2 * i + 1] = offsets[i];
	}
	fwrite(buffer, sizeof(uint32_t), count * 2, f);
	delete buffer;

	// Write data
	for (int i = 0; i < count; i++) {
		fseek(f, offsets[i], 0);
		fwrite(png_files[i]->data, sizeof(uint8_t), png_files[i]->length, f);
	}

	return true;
}

bool SectionTextures::toDir(GMFile* gmf, string section_path) const {
	// Save to file
	for (int i = 0; i < png_files.size(); i++) {
		string data_file = Util::join(section_path, to_string(i) + ".png");

		FILE* fp = fopen(data_file.c_str(), "wb");

		fwrite(png_files[i]->data, sizeof(uint8_t), png_files[i]->length, fp);

		fclose(fp);
	}

	return true;
}

bool SectionTextures::linkFrom(GMFile* gmf) {
	return true;
}

bool SectionTextures::linkTo(GMFile* gmf, FILE* f) {
	return true;
}
