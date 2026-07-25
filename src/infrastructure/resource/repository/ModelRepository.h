#pragma once
#include <string>
#include <string_view>
#include <optional>
#include <unordered_map>
#include <vector>
#include "core/data/ModelMetadata.h"
#include "thirdparty/nlohmann/json.hpp"

namespace infrastructure::resource::repository
{
	/**
	 * @brief 3Dモデルリソースを管理するリポジトリクラス
	 *
	 * resources.jsonからモデルメタデータを読み込み、
	 * モデル識別子でモデルハンドルおよびメタデータを取得する
	 */
	class ModelRepository
	{
	public:
		/**
		 * @brief コンストラクタ
		 *
		 * コンストラクト時にresources.jsonからすべてのモデルメタデータを読み込む
		 * @throw std::runtime_error ファイルが見つからないか、JSONパースに失敗した場合
		 */
	  ModelRepository(const nlohmann::json& j);

	  /**
	   * @brief IDでモデルを読み込みハンドルをキャッシュする
	   *
	   * @param modelId モデル識別子
	   * @return DxLibモデルハンドル、失敗時は-1
	   */
	  int loadModelById(std::string_view modelId);

	  /**
	   * @brief ファイルパスで直接モデルを読み込みハンドルをキャッシュする
	   *
	   * resources.jsonに登録しない配置物モデルなど、パスで解決するモデル用。
	   * 同じパスは1度だけロードして使い回す。
	   * @param path モデルファイルのパス
	   * @return DxLibモデルハンドル、失敗時は-1
	   */
	  int loadModelByPath(std::string_view path);

	  /**
	   * @brief IDでモデルのメタデータを取得する
	   *
	   * @param modelId モデル識別子
	   * @return 見つかった場合はモデルメタデータ、見つからない場合はstd::nullopt
	   */
	  [[nodiscard]] std::optional<core::data::ModelMetadata> getMetadata(std::string_view modelId) const;

	  /**
	   * @brief モデルハンドルを複製する
	   *
	   * 同じモデルを複数体で使う場合、アニメーションやスケールの状態が
	   * 競合しないようにインスタンスごとに複製ハンドルを使う
	   * @param modelHandle 複製元のモデルハンドル
	   * @return 複製したモデルハンドル、失敗時は-1
	   */
	  [[nodiscard]] int duplicateModel(int modelHandle);

	  /**
	   * @brief モデルにアタッチされている全アニメーションをデタッチする
	   * @param modelHandle 対象のモデルハンドル
	   */
	  void detachAllAnimations(int modelHandle);

	  /**
	   * @brief モデルの水平方向の外接半径を計算する（弾などの当たり判定サイズ自動取得用）
	   *
	   * 参照メッシュのAABB（X/Zの大きい方）の半分に scale を掛けて返す。
	   * @param modelHandle モデルハンドル
	   * @param scale 適用するスケール
	   * @return 水平方向の外接半径。失敗時は 0.0f
	   */
	  [[nodiscard]] float computeBoundingRadius(int modelHandle, float scale) const noexcept;

	  /**
	   * @brief モデルのAABB中心（ローカル座標・スケール未適用）を計算する
	   *
	   * モデル原点が見た目の中心とズレていると、原点まわりの回転で
	   * 円軌道を描いてしまう。その逆補正（中心まわりの回転）に使う。
	   * @param modelHandle モデルハンドル
	   * @return AABB中心のローカル座標。失敗時はゼロベクトル
	   */
	  [[nodiscard]] core::Vector3 computeBoundingCenter(int modelHandle) const noexcept;

	private:
	  struct ResourceDefinition
	  {
		  std::string m_id;
		  std::string m_path;
		};

		std::vector<ResourceDefinition> loadResourceList(const nlohmann::json& json);
		std::vector<ResourceDefinition> loadRawModelList(const nlohmann::json& json);
		core::data::ModelMetadata parseJsonFile(const std::string& filePath);

		/**
		 * @brief JSONの"mac"要素をMacMetadataへ変換する
		 * @param j mac要素のJSONオブジェクト
		 * @return 変換したMacMetadata
		 */
		core::data::MacMetadata parseMac(const nlohmann::json& j);

		/**
		 * @brief JSONの1フェーズ要素（phase1/phase2）をMacPhaseDataへ変換する
		 * @param p フェーズ要素のJSONオブジェクト
		 * @return 変換したMacPhaseData
		 */
		core::data::MacPhaseData parseMacPhase(const nlohmann::json& p);

		std::unordered_map<std::string, int> m_modelHandles;
		std::unordered_map<std::string, core::data::ModelMetadata> m_metadata;
		std::unordered_map<std::string, std::string> m_rawModelPaths;
	};
} // namespace infrastructure::resource::repository