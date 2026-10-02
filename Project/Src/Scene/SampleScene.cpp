#include "Scene/SampleScene.h"

using namespace LGF;

namespace {

	Camera3D::Param MakeCameraParam() {
		Camera3D::Param param{};
		param.farClip = 100.0f;
		return param;
	}

}

SampleScene::SampleScene(const InitData& init) :
	IScene(init),
	camera_(25.0f, MakeCameraParam()),
	light_({ 0.5f, -1.0f, 0.35f }, Color::White, 1.5f),
	ground_(30.0f),
	grid_(30.0f, 1.0f) {
	camera_.pitch = 22.0f * Math::DEG_TO_RAD;
}

void SampleScene::UpdateCamera() {
	camera_.Update();
}

void SampleScene::DrawStage() const {
	camera_.SetScene();
	light_.SetScene();
	ground_.Draw(
		Math::MakeTranslateMatrix({ 0.0f, -0.02f, 0.0f }),
		Color::DarkGray);
	grid_.Draw(Math::MakeTranslateMatrix({ 0.0f, 0.01f, 0.0f }));
}
