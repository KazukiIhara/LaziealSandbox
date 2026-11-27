#pragma once

#include <LGF/LGF.h>
#include <string_view>

class LevelEditorDemoScene {
public:
	LevelEditorDemoScene();

	void Update();
	void Draw();

private:
	void LoadLevel(std::string_view fileName);

	LGF::DebugCamera3D camera_;
	LGF::DirectionalLight directionalLight_;
	LGF::Ground3D ground_;
	LGF::Font2D font_;
	LGF::LevelIoResult levelResult_;
	std::string_view levelFile_;
};
