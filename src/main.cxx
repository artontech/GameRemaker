#define TCLAP_SETBASE_ZERO 1
#include "stdafx.h"

#include <filesystem>

#include "tclap/CmdLine.h"

#include "utils.h"
#include "GMFile.h"

void unpack(int argc, char* argv[]) {
	TCLAP::CmdLine cmd("Command description message", ' ', "0.9", false);
	cmd.ignoreUnmatched(true);

	TCLAP::ValueArg<string> argInput("i", "input", "input path", true, "", "string");
	cmd.add(argInput);

	TCLAP::ValueArg<string> argOutput("o", "output", "output path", false, Util::join(Util::cwd(), "output"), "string");
	cmd.add(argOutput);

	TCLAP::SwitchArg argDecomp("d", "decomp", "Decompile or not", false);
	cmd.add(argDecomp);

	cmd.parse(argc, argv);

	string input = argInput.getValue();
	string output = argOutput.getValue();
	bool decomp = argDecomp.getValue();

	cout << "Input: " << input << endl;
	cout << "Output: " << output << endl;

	filesystem::create_directory(output);

	GMFile file;
	file.fromFile(input);
	file.toDir(output);
}

void repack(int argc, char* argv[]) {
	TCLAP::CmdLine cmd("Command description message", ' ', "0.9", false);
	cmd.ignoreUnmatched(true);

	TCLAP::ValueArg<string> argInput("i", "input", "input path", true, "", "string");
	cmd.add(argInput);

	TCLAP::ValueArg<string> argOutput("o", "output", "output path", false, Util::join(Util::cwd(), "data.win"), "string");
	cmd.add(argOutput);

	TCLAP::SwitchArg argDecomp("d", "decomp", "Decompile or not", false);
	cmd.add(argDecomp);

	cmd.parse(argc, argv);

	string input = argInput.getValue();
	string output = argOutput.getValue();

	cout << "Input: " << input << endl;
	cout << "Output: " << output << endl;
	cout << "Decompile: " << argDecomp.getValue() << endl;

	GMFile file;
	file.fromDir(input);
	file.toFile(output);
}

int main(int argc, char* argv[]) {
	TCLAP::CmdLine cmd("Command description message", ' ', "0.9", true);
	cmd.ignoreUnmatched(true);

	TCLAP::UnlabeledValueArg<string> argMode("mode", "exec mode: unpack / repack", true, "unpack", "string");
	cmd.add(argMode);

	cmd.parse(argc, argv);

	string mode = argMode.getValue();
	cout << "Mode: " << mode << endl;

	if ("unpack" == mode) {
		unpack(argc, argv);
	}
	else if ("repack" == mode) {
		repack(argc, argv);
	}

    return 0;
}