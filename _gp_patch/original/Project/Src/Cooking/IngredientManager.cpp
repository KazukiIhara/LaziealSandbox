#include "IngredientManager.h"

#include "LGF/Input.h"
#include "Input/XboxController.h"

#include <imgui.h>

using namespace LGF;

namespace {

	constexpr std::size_t kRiceCountPerServing = 32u;
	constexpr float kRiceAmountPerServing = 150.0f;

}

IngredientManager::IngredientManager(Physics::PhysicsWorld& physicsWorld) :
	physicsWorld_(&physicsWorld) {}

void IngredientManager::Update(const Vector3& riceSpawnPosition) {
	const auto xboxController = XboxController::Get();
	const bool isXboxAddRiceTriggered = xboxController.has_value() &&
		xboxController->Button(GamepadButtonCode::West).Trigger();
	if (Key::Space.Trigger() || isXboxAddRiceTriggered) {
		AddRice(riceSpawnPosition);
	}
}

void IngredientManager::Draw() const {
	for (const auto& rice : riceObjects_) {
		rice->Draw();
	}
}

void IngredientManager::DrawImGuiDebug() {
#ifdef _DEBUG
	if (ImGui::Begin("Cooking Fried Rice")) {
		ImGui::Text("Space / Xbox X : Add Rice");
		ImGui::Text("Rice Amount : %.0f g", cookingFriedRice_.GetRiceAmount());
		ImGui::Text("Rice Particles : %zu", riceObjects_.size());
		ImGui::Text("Ingredients : %zu", cookingFriedRice_.GetIngredients().size());
		ImGui::Text("State : %d", static_cast<int>(cookingFriedRice_.GetState()));
	}
	ImGui::End();
#endif
}

const CookingFriedRice& IngredientManager::GetCookingFriedRice() const {
	return cookingFriedRice_;
}

void IngredientManager::AddRice(const Vector3& spawnPosition) {
	if (physicsWorld_ == nullptr) {
		return;
	}

	for (std::size_t index = 0u; index < kRiceCountPerServing; ++index) {
		const float horizontalIndex = static_cast<float>(index % 4u) - 1.5f;
		const float depthIndex = static_cast<float>((index / 4u) % 4u) - 1.5f;
		const float height = static_cast<float>(index / 16u) * 0.16f;
		const Vector3 position = spawnPosition + Vector3{
			horizontalIndex * 0.12f,
			height,
			depthIndex * 0.12f,
		};

		riceObjects_.push_back(std::make_unique<Rice>(
			*physicsWorld_,
			position,
			ObjectHandle{ nextObjectIndex_++, 1u }));
	}

	cookingFriedRice_.AddRice(kRiceAmountPerServing);
}
