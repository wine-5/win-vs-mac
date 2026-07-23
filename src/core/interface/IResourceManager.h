#pragma once
#include <string>
#include <string_view>
#include <optional>
#include "core/data/ModelMetadata.h"
#include "core/data/StageMetadata.h"
#include "core/data/ProjectileMetadata.h"

namespace core::iface
{
	/**
	 * @brief リソースの読み込み・管理を行う純粋仮想クラス
	 * game層がInfrastructure層に直接依存しないようにするための抽象化
	*/
	class IResourceManager
	{
	public:
		virtual ~IResourceManager() = default;

		/**
		 * @brief modelIDからモデルを読み込み、ハンドルを返す
		 * @param modelId モデルID
		 * @return モデルハンドル
		 */
		virtual int loadModelById(const std::string_view modelId) = 0;

		/**
		 * @brief modelIDからメタデータを取得する
		 * @param modelId モデルID
		 * @return メタデータ（存在しない場合nullopt）
		 */
		[[nodiscard]] virtual std::optional<core::data::ModelMetadata> getMetadata(const std::string_view modelId) const = 0;

		/**
		 * @brief フォントIDからフォント名を取得する
		 * @param fontId フォントID
		 * @return フォントファミリー名（存在しない場合nullopt）
		 */
		[[nodiscard]] virtual std::optional<std::string> getFontName(const std::string_view fontId) const = 0;

		/**
		 * @brief 画像IDから画像を読み込みハンドルを返す（キャッシュ付き）
		 * @param imageId 画像ID（resources.json の images セクションで定義）
		 * @return DxLib 画像ハンドル、失敗時は -1
		 */
		virtual int loadImageById(std::string_view imageId) = 0;

		/**
		 * @brief アニメーションIDからアニメーションモデルを読み込みハンドルを返す（キャッシュ付き）
		 * @param animationId アニメーションID（resources.json の animations セクションで定義）
		 * @return DxLib モデルハンドル、失敗時は -1
		 */
		virtual int loadAnimationById(std::string_view animationId) = 0;

		/**
		 * @brief ステージの配置定義を取得する
		 * @return ステージ配置定義
		 */
		[[nodiscard]] virtual const core::data::StageMetadata& getStageMetadata() const noexcept = 0;

		/**
		 * @brief 弾IDから弾定義を取得する
		 * @param projectileId 弾ID（projectileData.json で定義）
		 * @return 弾定義
		 */
		[[nodiscard]] virtual const core::data::ProjectileMetadata& getProjectileMetadata(std::string_view projectileId) const = 0;

		/**
		 * @brief モデルハンドルを複製する
		 *
		 * 同じモデルを複数体で使う場合、アニメーションやスケールの状態が
		 * 競合しないようにインスタンスごとに複製ハンドルを使う
		 * @param modelHandle 複製元のモデルハンドル
		 * @return 複製したモデルハンドル、失敗時は-1
		 */
		[[nodiscard]] virtual int duplicateModel(int modelHandle) = 0;

		/**
		 * @brief モデルにアタッチされている全アニメーションをデタッチする
		 *
		 * 敵の死亡後にモデルハンドルをプールへ返却し使い回す際、
		 * 前の持ち主がアタッチしたアニメーションが残ったままだと
		 * 新しい持ち主のアニメーションと二重アタッチになるため、
		 * 返却前に呼んでクリーンな状態に戻す
		 * @param modelHandle 対象のモデルハンドル
		 */
		virtual void detachAllAnimations(int modelHandle) = 0;

		/**
		 * @brief モデルの水平方向の外接半径を計算する（弾などの当たり判定サイズ自動取得用）
		 *
		 * モデルのバウンディングボックス（X/Z の大きい方）の半分に scale を掛けて返す。
		 * @param modelHandle モデルハンドル
		 * @param scale 適用するスケール
		 * @return 水平方向の外接半径。失敗時は 0.0f
		 */
		[[nodiscard]] virtual float computeBoundingRadius(int modelHandle, float scale) const noexcept = 0;

		/**
		 * @brief モデルのAABB中心（ローカル座標・スケール未適用）を計算する
		 *
		 * モデル原点が見た目の中心とズレていると、原点まわりの回転で円軌道を描く。
		 * その逆補正（中心まわりの回転）に使う。
		 * @param modelHandle モデルハンドル
		 * @return AABB中心のローカル座標。失敗時はゼロベクトル
		 */
		[[nodiscard]] virtual core::Vector3 computeBoundingCenter(int modelHandle) const noexcept = 0;
	};
} // namespace core::iface
