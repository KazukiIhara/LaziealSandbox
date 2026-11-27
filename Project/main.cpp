#include <LGF/LGF.h>

#include "Scene/LevelEditorDemoScene.h"

using namespace LGF;

void Main() {
	// LevelEditorで扱うレベルデータの確認用シーンを起動します。
	LevelEditorDemoScene scene;

	while (System::Update()) {
		scene.Update();
		scene.Draw();
	}
}
