# Sample models

Rendering test assets downloaded from the official
[Khronos glTF Sample Assets](https://github.com/KhronosGroup/glTF-Sample-Assets)
repository on 2026-09-11.

## Entry points

| Model | Entry point | Intended test | License |
| --- | --- | --- | --- |
| Box Textured | `BoxTextured/BoxTextured.glb` | Minimal GLB geometry and base-color texture loading | CC BY 4.0; contains the Cesium logo |
| Duck | `Duck/Duck.glb` | A small, recognizable textured mesh | SCEA Shared Source License 1.0 |
| Flight Helmet | `FlightHelmet/FlightHelmet.gltf` | External buffers/textures, multiple materials, normal maps, occlusion, roughness, and metallic PBR inputs | CC0 1.0 |

`FlightHelmet.gltf` refers to the `.bin` and `.png` files beside it. Keep that
directory together or update the URIs in the glTF file when moving it.

## Sources and license texts

- `BoxTextured/SOURCE.md` and `Licenses/CC-BY-4.0.txt`
- `Licenses/LicenseRef-LegalMark-Cesium.txt` for the Cesium logo in Box Textured
- `Duck/SOURCE.md` and `Licenses/SCEA.txt`
- `FlightHelmet/SOURCE.md` and `Licenses/CC0-1.0.txt`

Each `SOURCE.md` is the model's upstream README and contains its original legal
notice. Preserve the applicable notice and license when redistributing an asset.
