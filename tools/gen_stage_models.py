#!/usr/bin/env python3
"""外部で用意したテクスチャ(.png)を貼った、UV付き立方体(.mqo)を生成する。

テクスチャの中身（絵）は生成しない。ChatGPT等で1枚ずつ作った写実的なPNGを
`assets/model/stage/` に置き、このツールはその上に貼るための幾何（立方体＋UV）
だけを吐く。テクスチャの質は外部生成に任せ、UVの当て方はコード側で保証する、
という分業のための土台。

＝ テクスチャ生成（旧 gen_stage_textures.py の描画部分）から幾何生成を切り離した版。

前提:
    ゲームはライティング無効（Main.cpp の SetUseLighting(FALSE)）で動くため、
    テクスチャの色はそのまま出る。陰影が欲しければ絵に焼き込んでおく。

出力（assets/model/stage/ 配下）:
    <id>.mqo … MANIFESTで指定したPNGを貼った 100x100x100 の立方体（UV付き）

使い方（リポジトリのルートで実行）:
    python tools/gen_stage_models.py            # MANIFEST全件を生成
    python tools/gen_stage_models.py block_folder   # 指定IDだけ生成
"""
import os
import sys

OUT_DIR = os.path.join("assets", "model", "stage")

# ---- 生成する配置物モデルの一覧 ----
# id           … 出力する .mqo のファイル名（拡張子なし）。カタログのmodelPathと合わせる
# texture      … 貼るPNGのファイル名（OUT_DIR 内にある想定。外部で用意する）
#
# 立方体1個をXYZに引き伸ばして床・壁・柱・ブロックを兼ねるため、
# ここに1行足してPNGを置くだけで新しい配置物が増える。
# idはカタログ(stageCatalog.json)のmodel/textureパスと一致させる（PascalCase）。
# ---- 材質プリセット（光沢の強さ, 鋭さ）----
# 光沢は「床だから」ではなく「何でできているか・プレイヤーが干渉できるか」で決める。
# 動くとハイライトが流れるため、光る物＝触れる物、という手掛かりとして使う。
#
# powerは大きいほどハイライトが小さく鋭くなる（金属寄り）。
BACKGROUND = (0.06, 12.0)    # 壁・柱・床。動かせない背景なのでごく僅かに留める
DESTRUCTIBLE = (0.35, 25.0)  # 壊せるブロック。「殴れる物」だと一目で分かるように光らせる
SPECIAL = (0.55, 40.0)       # 端末・扉・ギャンブル箱。特別な設置物として更に目立たせる

MANIFEST = [
    # (id,               texture png,          材質)
    # ⓪ 全エリア共通。エリア同士をつなぐ通路（動く歩道としても使う）
    ("PathCorridor",     "PathCorridor.png",     BACKGROUND),
    # ① Desktop（入口・ユーザー領域）
    ("FloorDesktop",     "FloorDesktop.png",     BACKGROUND),
    ("BlockFolder",      "BlockFolder.png",      DESTRUCTIBLE),
    ("BlockRecycleBin",  "BlockRecycleBin.png",  DESTRUCTIBLE),
    ("WallExplorer",     "WallExplorer.png",     BACKGROUND),
    # ② System32（システム深層）
    ("FloorMemory",      "FloorMemory.png",      BACKGROUND),
    ("WallTerminal",     "WallTerminal.png",     BACKGROUND),
    ("WallRegistry",     "WallRegistry.png",     BACKGROUND),
    ("WallData",         "WallData.png",         BACKGROUND),
    # ③ Program Files（アプリ格納庫・UAC関門）
    ("GateUac",          "GateUac.png",          SPECIAL),
    ("PillarApp",        "PillarApp.png",        BACKGROUND),
    ("BlockZip",         "BlockZip.png",         DESTRUCTIBLE),
    # ④ Apple アリーナ（ボス戦）
    ("FloorApple",       "FloorApple.png",       BACKGROUND),
    ("WallDanger",       "WallDanger.png",       BACKGROUND),
    # ⑤ 拡張子ブロック（gen_block_textures.py が吐くテクスチャを貼る）。
    #    どの拡張子が出るかはステージ生成時に重み付き抽選で決まるため、
    #    種類ぶんのモデルを用意しておく
    ("BlockExtArchive",    "BlockExtArchive.png",    DESTRUCTIBLE),
    ("BlockExtAudio",      "BlockExtAudio.png",      DESTRUCTIBLE),
    ("BlockExtDocument",   "BlockExtDocument.png",   DESTRUCTIBLE),
    ("BlockExtExecutable", "BlockExtExecutable.png", DESTRUCTIBLE),
    ("BlockExtImage",      "BlockExtImage.png",      DESTRUCTIBLE),
    ("BlockExtShortcut",   "BlockExtShortcut.png",   DESTRUCTIBLE),
    ("BlockExtSourceCode", "BlockExtSourceCode.png", DESTRUCTIBLE),
    ("BlockExtVideo",      "BlockExtVideo.png",      DESTRUCTIBLE),
    ("BlockExtUnknown",    "BlockExtUnknown.png",    DESTRUCTIBLE),
    # ⑥ 中身のない普通のブロック（gen_plain_block_texture.py が吐くテクスチャを貼る）。
    #    壊せない足場・地形として置く物なので、ひび・破片は用意しない
    ("BlockPlain",         "BlockPlain.png",         BACKGROUND),
    # ⑦ RAMブロック（gen_ram_block_texture.py が吐くテクスチャを貼る）。
    #    壊すと拡張子を挿せる枠が1つ増える
    ("BlockRam",           "BlockRam.png",           DESTRUCTIBLE),
    # ⑧ 拡張子の付け替え端末（gen_rename_texture.py が吐くテクスチャを貼る）。
    #    壊せない設置物なので、ひび・破片は用意しない
    ("BlockRename",        "BlockRename.png",        SPECIAL),
    # ⑨ ギャンブルボックス。壊すと高確率で敵が出るが、低確率で装備中の拡張子の効果が2倍になる
    ("BlockGamble",        "BlockGamble.png",        SPECIAL),
]

# ---- 100x100x100 立方体の頂点 ----
VERTS = [
    (-50, -50, -50), (50, -50, -50), (50, 50, -50), (-50, 50, -50),
    (-50, -50, 50), (50, -50, 50), (50, 50, 50), (-50, 50, 50),
]
# 各面の4頂点を「外から見たときの 左下→右下→右上→左上」の順で並べる。
# UVを面ごとに考えず1組に固定できるので、どの面でも絵が同じ向きで貼られる。
#
# 【重要】以前は逆順（左上→右上→右下→左下）で並べており、DxLibからは全面が
# 「裏面」に見えていた。裏面には DxLib がシャドウマップを適用しないため、
# 背面カリングを切る（DX_CULLING_NONE）ことで絵は合わせられても影が一切落ちなかった。
# 巻き方向を正しくすることで、既定のカリングのまま正しい面が描かれ、影も落ちる。
FACES = [
    (0, 1, 2, 3),  # -Z（正面）
    (5, 4, 7, 6),  # +Z（背面）
    (1, 5, 6, 2),  # +X（右）
    (4, 0, 3, 7),  # -X（左）
    (3, 2, 6, 7),  # +Y（上：床の天面）
    (4, 5, 1, 0),  # -Y（下）
]
# 頂点順（左下→右下→右上→左上）に対してUを反転させて割り当てる。
# 素直に (0,1)(1,1)(1,0)(0,0) にすると、実機で絵が左右反転して鏡文字になる
# （DxLibがMQO読み込み時にZを反転させるため）。巻き方向とは別の話なので、
# 巻き方向を直したあともこの補正は必要。
UV = [(1, 1), (0, 1), (0, 0), (1, 0)]


def mqo_text(tex_filename, material):
    """指定PNGを貼った立方体のmqoテキストを返す。

    material は (光沢の強さ, 鋭さ) のタプル（BACKGROUND / DESTRUCTIBLE / SPECIAL）。
    """
    # 法線マップ（<名前>_normal.png）が置いてあれば bump として一緒に貼る。
    # DxLibはbumpを法線マップとして読み込むので、平らな面に凹凸を持たせられる。
    # 「ファイルを置いて作り直すだけで有効になる」形にして、対応表を二重に持たない
    normal_filename = os.path.splitext(tex_filename)[0] + "_normal.png"
    has_normal = os.path.exists(os.path.join(OUT_DIR, normal_filename))
    bump = ' bump("%s")' % normal_filename if has_normal else ""

    spc, power = material

    lines = [
        "Metasequoia Document",
        "Format Text Ver 1.0",
        "",
        "Material 1 {",
        '\t"tex" shader(3) col(1.000 1.000 1.000 1.000) dif(1.000) '
        'amb(1.000) emi(0.000) spc(%.3f) power(%.2f) tex("%s")%s'
        % (spc, power, tex_filename, bump),
        "}",
        'Object "cube" {',
        "\tvisible 15",
        "\tlocking 0",
        "\tshading 1",
        "\tcolor 0.6 0.6 0.6",
        "\tcolor_type 0",
        "\tvertex %d {" % len(VERTS),
    ]
    for v in VERTS:
        lines.append("\t\t%.4f %.4f %.4f" % v)
    lines.append("\t}")
    lines.append("\tface %d {" % len(FACES))
    uvflat = " ".join("%.4f %.4f" % (UV[i][0], UV[i][1]) for i in range(4))
    for f in FACES:
        lines.append("\t\t4 V(%d %d %d %d) M(0) UV(%s)" % (f[0], f[1], f[2], f[3], uvflat))
    lines.append("\t}")
    lines.append("}")
    return "\n".join(lines) + "\n"


def main():
    os.makedirs(OUT_DIR, exist_ok=True)

    # 引数でIDを絞れる（指定なしなら全件）
    wanted = set(sys.argv[1:])
    targets = [m for m in MANIFEST if not wanted or m[0] in wanted]
    if wanted:
        unknown = wanted - {m[0] for m in MANIFEST}
        for u in unknown:
            print("[warn] MANIFEST に無いID:", u)

    for model_id, texture, material in targets:
        png_path = os.path.join(OUT_DIR, texture)
        if not os.path.exists(png_path):
            # テクスチャは外部で用意する運用なので、無ければ知らせるだけで止めない
            print("[warn] テクスチャ未配置:", png_path, "→ mqoは出力するが実機では貼られない")

        mqo_path = os.path.join(OUT_DIR, model_id + ".mqo")
        with open(mqo_path, "w", encoding="utf-8") as f:
            f.write(mqo_text(texture, material))
        print("generated", mqo_path, "→", texture)


if __name__ == "__main__":
    main()
