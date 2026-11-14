#include "GamePlayScene.h"

#include <imgui.h>

#include "LGF/Input.h"
#include "LGF/System.h"

#include <algorithm>

#include "Collision/CollisionManager.h"
#include "Input/XboxController.h"

using namespace LGF;

namespace {

	Physics::PhysicsWorldSettings MakePhysicsWorldSettings() {
		Physics::PhysicsWorldSettings settings{};
		settings.worldBounds = {
			.min = { -20.0f, -10.0f, -20.0f },
			.max = { 20.0f, 20.0f, 20.0f },
		};
		settings.substepCount = 8u;
		settings.solverIterationCount = 8u;
		settings.linearDamping = 0.2f;
		return settings;
	}

}

GamePlayScene::GamePlayScene(const InitData& init) :
	IScene(init),
	camera_(15.0f),
	light_({ 0.5f, -1.0f, 0.35f }, Color::White, 1.5f),
	physicsWorld_(MakePhysicsWorldSettings()) {

	camera_.translation = { 0.0f, 7.0f, -20.0f };
	camera_.yaw = 0.0f;
	camera_.pitch = 0.3f;

	/* コリジョンマネージャー設定 */
	auto* collisionManager = CollisionManager::GetInstance();
	// 一応クリア
	collisionManager->Clear();
	// タグの登録
	collisionManager->RegisterTag("Player"); // プレイヤー
	collisionManager->RegisterTag("Solid"); // 固体（プレイヤーとの押し戻しを行うオブジェクト）
	collisionManager->RegisterTag("InteractionSensor"); // インタラクト用センサー（プレイヤーの中華レンジ周辺でのモード切替用）

	player_ = std::make_unique<Player>();

	floorPhysicsHandle_ = physicsWorld_.AddStaticObject(
		Collision::AABB{
			.min = { -20.0f, -0.5f, -20.0f },
			.max = { 20.0f, 0.0f, 20.0f },
		},
		Physics::StaticObjectOptions{
			.material = { .restitution = 0.05f, .friction = 0.9f },
			.objectHandle = { 1u, 1u },
		});

	cookingController_ = std::make_unique<CookingController>(physicsWorld_);
	ingredientManager_ = std::make_unique<IngredientManager>(physicsWorld_);

	stageManager_ = std::make_unique<StageManager>("../Assets/Stages/stage.json", physicsWorld_);

	judgeManager_ = std::make_unique<JudgeManager>();
}

void GamePlayScene::Update() {
	if (openUIWorldDemoCommand_.Trigger()) {
		GetData().previousScene = GetState();
		ChangeScene(SceneNames::UIWorldDemo);
		return;
	}
	if (Key::L.Trigger()) {
		ChangeScene(SceneNames::ColliderWorldDemo);
		return;
	}
	if (Key::P.Trigger()) {
		ChangeScene(SceneNames::PhysicsWorldDemo);
		return;
	}
	if (Key::K.Trigger()) {
		ChangeScene(SceneNames::SkinningAnimationDemo);
		return;
	}

	/* カメラ操作 */
	static bool prevCookingMode = false;
	bool currentCookingMode = player_->IsCookingMode();

	if (!player_->IsCookingMode()) {
		// 料理モード中でなければ追従カメラ
		UpdateCamera();

		// 料理モードが解除された瞬間、pitchを調整
		if (prevCookingMode && !currentCookingMode) {
			camera_.pitch = 0.3f;
		}
	} else {
		// 料理モード中はカメラ固定
		camera_.translation = {0.0f, 4.7f, -2.6f};
		camera_.yaw = 0.0f;
		camera_.pitch = 0.7f;
	}
	prevCookingMode = currentCookingMode;

	/* プレイヤー更新 */
	player_->Update(camera_.yaw);
	// カメラの位置をプレイヤーに追従させる
	if (Key::Num1.Trigger()) { // Num1キーで有効状態切替
		isCameraFollowEnabled_ = !isCameraFollowEnabled_;
	}
	if (isCameraFollowEnabled_) {
		camera_.translation = player_->GetEyePosition();
	}
	/* 調理器具マネージャーの更新 */
	cookingController_->Update();
	/* 食材マネージャーの更新 */
	ingredientManager_->Update(cookingController_->GetWok().GetRiceSpawnPosition());
	/* 物理ワールドの更新 */
	physicsWorld_.Update(static_cast<float>(System::DeltaTime()));
	/* ステージオブジェクト管理クラスの更新 */
	stageManager_->Update();
	// 審査員マネージャーの更新
	judgeManager_->Update(player_.get(), camera_);

	/* コリジョンマネージャー更新 */
	CollisionManager::GetInstance()->Step();

#ifdef _DEBUG
	if (ImGui::Begin("GamePlay Debug")) {

		ImGui::Text("Num1 : Toggle Camera Follow");
		ImGui::SameLine();
		ImGui::Text("[%s]", isCameraFollowEnabled_ ? "Enabled" : "Disabled");

		// 現在の設定状態
		ImGui::Text("Num2 : Toggle Mouse Camera"); 
		ImGui::SameLine();
		ImGui::Text("[%s]", isMouseCameraEnabled_ ? "Enabled" : "Disabled");
		ImGui::Text("K : Open Skinning Animation Demo");

		ImGui::Separator();

		// カメラ情報
		ImGui::DragFloat3("Camera Position", &camera_.translation.x, 0.05f);
		ImGui::DragFloat("Camera Yaw", &camera_.yaw, 0.01f, -6.28f, 6.28f);
		ImGui::DragFloat("Camera Pitch", &camera_.pitch, 0.01f, -1.57f, 1.57f);
	}
	ImGui::End();

	player_->DrawImGuiDebug();
	cookingController_->DrawImGuiDebug();
	ingredientManager_->DrawImGuiDebug();
	stageManager_->DrawImGuiDebug();

	CollisionManager::GetInstance()->DrawImGuiDebug();
#endif
}

void GamePlayScene::Draw() const {
	skybox_.SetScene();

	camera_.SetScene();
	light_.SetScene();

	// プレイヤー描画
	player_->Draw();
	// 調理器具マネージャーの描画
	cookingController_->Draw();
	// 食材マネージャーの描画
	ingredientManager_->Draw();
	// ステージオブジェクト管理クラスの描画
	stageManager_->Draw();
	// 審査員マネージャーの描画
	judgeManager_->Draw();
}

void GamePlayScene::UpdateCamera() {
	float yawDelta = 0.0f;
	float pitchDelta = 0.0f;
	const float deltaTime = static_cast<float>(System::DeltaTime());

	auto processStickInput = [&](const Gamepad& gamepad) {
		Vector2 rightStick = gamepad.RightStick();

		// デッドゾーン処理
		if (std::abs(rightStick.x) < kDeadZone)
			rightStick.x = 0.0f;
		if (std::abs(rightStick.y) < kDeadZone)
			rightStick.y = 0.0f;

		yawDelta += rightStick.x * kStickSensitivity * deltaTime;
		pitchDelta -= rightStick.y * kStickSensitivity * deltaTime;
	};

	/* 右スティックによる視点操作 */
	auto rightJoyCon = JoyCon::Right();
	if (rightJoyCon.has_value() && rightJoyCon->IsConnected()) {
		processStickInput(*rightJoyCon);
	}

	const auto xboxController = XboxController::Get();
	if (xboxController.has_value()) {
		processStickInput(*xboxController);
	}

	/* マウスによる視点操作（デバッグ） */
	if (Key::Num2.Trigger()) { // Num2キーで入力有効状態切替
		isMouseCameraEnabled_ = !isMouseCameraEnabled_; 
	}
	if (isMouseCameraEnabled_) {
		Vector2 mouseDelta = Mouse::Delta();
		yawDelta += mouseDelta.x * kMouseSensitivity * deltaTime;
		pitchDelta += mouseDelta.y * kMouseSensitivity * deltaTime;
	}

	cookingController_->DrawImGuiDebug();
	judgeManager_->DrawImGuiDebug();

	// 最終的なカメラ角度に反映
	camera_.yaw += yawDelta;
	camera_.pitch += pitchDelta;

	// Pitchの回転制限
	camera_.pitch = std::clamp(camera_.pitch, -kPitchLimit, kPitchLimit);
}
