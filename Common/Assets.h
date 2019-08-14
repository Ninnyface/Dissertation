#pragma once
#include <string>

namespace NCL {
	namespace Assets {
		const std::string SHADERDIR("../Shaders/");
		const std::string MESHDIR("../Meshes/");
		const std::string TEXTUREDIR("../Textures/");
		const std::string SOUNDSDIR("../Sounds/");
		const std::string FONTSSDIR("../Fonts/");
		const std::string DATADIR("../Data/");
		extern bool ReadTextFile(const std::string &filepath, std::string& result);
		//static inline bool ReadBinaryFile(const std::string &filepath, std::string& result);
	}
}