#include <LGF/LGF.h>

#include "Scene/ShowcaseScene.h"

using namespace LGF;

void Main() {
	// サンプルの初期化とフレーム処理はShowcaseSceneへまとめています。
	// 新しい機能を試す場合は、まずShowcaseSceneのUpdateとDrawを確認してください。
	ShowcaseScene scene;

	while (System::Update()) {
		scene.Update();
		scene.Draw();
	}
}
