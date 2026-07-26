# ステージ配置物テクスチャ 生成プロンプト集

外部（ChatGPT等）で写実的なテクスチャPNGを1枚ずつ生成するためのプロンプト集。
生成したPNGは `assets/model/stage/` に置き、`tools/gen_stage_models.py` の
MANIFEST に1行足して立方体(.mqo)を生成する。

## 進め方

1. 下のプロンプトを ChatGPT に貼って画像を生成する
2. できたPNGを **`assets/model/stage/<ファイル名>`** に保存する
3. `tools/gen_stage_models.py` の MANIFEST に `("<id>", "<ファイル名>")` を追加
4. `python tools/gen_stage_models.py <id>` で mqo を生成
5. `stageCatalog.json` に配置物として登録 → エディタ／実機で確認

> まず #1 か #2 を1枚だけ作り、貼り具合（向き・解像度）を確認してから量産すると安全。

## 区画と色の進行（4区画・奥ほど危険＝赤へ）

| # | 区画 | 危険度 | ベース色 |
|---|---|---|---|
| ① | Desktop（入口・ユーザー領域） | 安全 | 青 `#0067C0` |
| ② | System32（システム深層） | 中 | 緑〜琥珀 `#3FB950` → `#D29922` |
| ③ | Program Files（アプリ格納庫・UAC関門） | 高 | 橙 `#E8891C` |
| ④ | Apple アリーナ（ボス戦） | 最大 | 赤 `#E81123` |

## 進捗チェックリスト

- [ ] 1. FloorDesktop.png
- [ ] 2. BlockFolder.png
- [ ] 3. BlockRecycleBin.png
- [ ] 4. WallExplorer.png
- [ ] 5. FloorMemory.png
- [ ] 15. WallTerminal.png（旧 FloorTerminal.png を壁へ移設）
- [ ] 6. BlockDll.png
- [ ] 7. BlockExe.png
- [ ] 8. WallRegistry.png
- [ ] 9. WallData.png
- [ ] 10. GateUac.png
- [ ] 11. PillarApp.png
- [ ] 12. BlockZip.png
- [ ] 13. FloorApple.png
- [ ] 14. WallDanger.png

---

## Common note

Each prompt below is self-contained (the common spec is baked in). Every texture is
**1024x1024, square, opaque, flat orthographic (no perspective), evenly lit**, because the
engine renders with lighting disabled. Prompts marked *tileable* must have connecting edges.

**Legibility rule (important):** these textures are mapped onto small cube/wall faces in 3D, so
each face only occupies a fraction of the screen and gets minified by mipmaps. Any "densely
packed tiny text" turns into gray noise. Text-based textures must use **large, high-contrast
monospaced type — only ~8 to 15 lines per 1024px texture**, with generous line spacing and margins,
so the terminal / code / error character reads at a glance. Prefer a few big, recognizable
keywords (`ERROR`, `root@`, `0xDEADBEEF`, `segfault`, `[FATAL]`) over walls of unreadable text.

---

## 1. FloorDesktop.png — ① Desktop / floor / tileable

```
A top-down floor texture of the Windows 11 default desktop wallpaper: a deep blue gradient with the abstract "Bloom" ribbons (translucent flower-like petals of light) fanning out. Seamlessly tileable — the top/bottom and left/right edges must connect so it repeats without visible seams. No logos, no text. Calm blue overall with even, uniform brightness. Flat orthographic straight-down view, no perspective, no camera tilt. Bake only very soft shading so tiling stays clean. Fully opaque, no transparency. 1024x1024 pixels, square.
```

## 2. BlockFolder.png — ① Desktop / one face of a cube block

```
A single face texture for a cube: a Windows 11 style yellow folder icon, large and centered in the square. The folder is slightly open and dimensional, bright yellow (around #FFB900). Plain dark blue background (#0a1420) with a little margin around the icon. No text. Flat front-facing orthographic view, no perspective, no camera tilt, even lighting. Fully opaque, no transparency. 1024x1024 pixels, square.
```

## 3. BlockRecycleBin.png — ① Desktop / one face of a cube block

```
A single face texture for a cube: a Windows 11 style Recycle Bin icon, large and centered in the square. A translucent blue bin with the recycling arrows symbol. Plain dark blue background (#0a1420) with a little margin around the icon. No text. Flat front-facing orthographic view, no perspective, no camera tilt, even lighting. Fully opaque, no transparency. 1024x1024 pixels, square.
```

## 4. WallExplorer.png — ① Desktop / wall

```
A wall texture showing the Windows 11 File Explorer window UI seen straight on. Title bar and address bar across the top, a left navigation pane listing "PC, Downloads, Documents, Pictures", and a right area filled with file and folder icons in a grid. Bright light-gray Windows 11 interface. A complete single image (not tiled). Flat front-facing orthographic view, no perspective, no camera tilt, even lighting. Fully opaque, no transparency. 1024x1024 pixels, square.
```

## 5. FloorMemory.png — ② System32 / floor / tileable

> 旧 `FloorTerminal.png`（緑のログ床）から差し替え。緑のログ絵は**壁**として残すので
> 削除せず、#15 WallTerminal.png に移設する。
>
> **床に文字を置かない理由**：床は上下左右どの向きからも見るため文字が裏返って読めてしまい、
> 視線も吸われる。さらに `textureTile` が小さいと細い文字が斜めからの見えでちらつく。
> 読ませたい情報（ログ・コード・エラー）は壁に寄せ、床は向きを持たない構造だけにする。

```
A top-down floor texture of a memory map / allocation grid, like a debugger's VMMap or Task Manager memory view seen from directly above. A near-black dark surface (#0b1410) covered by a regular grid of small square cells with thin dim separator lines. Most cells are very dark and barely distinguishable; roughly one in eight cells is filled with a muted green (#2d6b3a) and a rare few with a dim amber (#8a6415), scattered irregularly so no obvious pattern forms. Very LOW contrast overall — the whole image must read as a dark, calm, quiet surface, never bright or glowing. Absolutely no text, no letters, no numbers, no icons. Rotationally neutral: it must look the same viewed from any direction. Seamlessly tileable — all four edges must connect. Flat orthographic straight-down view, no perspective, no camera tilt, even and uniform brightness. Fully opaque, no transparency. 1024x1024 pixels, square.
```

あわせて `stageCatalog.json` の `floor_terminal` の `textureTile` を 600 → **1400** に上げる。
タイルが大きくなるほど繰り返しが減り、模様の空間周波数が下がって斜めのちらつきが収まる。

## 6. BlockDll.png — ② System32 / one face of a cube block

```
A single face texture for a cube: a ".dll" system library file icon, large and centered in the square. A document icon with a gear overlaid on it, and the text ".dll" at the bottom. Amber accent color (#D29922). Plain dark gray background (#14181f). Flat front-facing orthographic view, no perspective, no camera tilt, even lighting. Fully opaque, no transparency. 1024x1024 pixels, square.
```

## 7. BlockExe.png — ② System32 / one face of a cube block

```
A single face texture for a cube: a ".exe" executable file icon, large and centered in the square. An application-window style icon with the text ".exe" at the bottom. Teal-to-amber accent colors. Plain dark gray background (#14181f). Flat front-facing orthographic view, no perspective, no camera tilt, even lighting. Fully opaque, no transparency. 1024x1024 pixels, square.
```

## 8. WallRegistry.png — ② System32 / wall

```
A wall texture showing a Windows Registry Editor style UI seen straight on. A hierarchical key tree on the left (HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER, etc.) and a right panel with "Name, Type, Data" columns listing registry values. Use LARGE, clearly readable text with only a handful of tree entries and rows (roughly 6 to 10) — big legible type, not a dense cramped list. Dark background with green-to-amber text, a hard technical System32 atmosphere. A complete single image (not tiled). Flat front-facing orthographic view, no perspective, no camera tilt, even lighting. Fully opaque, no transparency. 1024x1024 pixels, square.
```

## 9. WallData.png — ②③ / wall / vertically tileable

```
A "server interior" wall texture: a black background with a single column of LARGE, bold, high-contrast source code and system logs (C++, Python, JavaScript flavored). Only about 10 to 14 lines total, big and clearly readable with generous line spacing — NOT densely packed small text and NOT multiple narrow columns. Text color mainly amber-to-orange (#D29922 to #E8891C) with one or two red [ERROR] lines standing out. Vertically tileable — top and bottom edges must connect, with lines spaced evenly so nothing is cut off at the seam when repeated vertically. Flat front-facing orthographic view, no perspective, no camera tilt, even brightness. Fully opaque, no transparency. 1024x1024 pixels, square.
```

## 10. GateUac.png — ③ Program Files / wall

```
A gate wall texture showing a Windows User Account Control (UAC) dialog seen straight on, large and centered. A blue-and-yellow shield icon, a dialog reading "Do you want to allow this app to make changes to your device?" with "Yes / No" buttons. The surrounding background is dark and sunken so only the dialog glows. Orange accent. A complete single image (not tiled). Flat front-facing orthographic view, no perspective, no camera tilt, even lighting. Fully opaque, no transparency. 1024x1024 pixels, square.
```

## 11. PillarApp.png — ③ Program Files / pillar / vertically tileable

```
A pillar texture of installed application folders stacked vertically. Each tier shows an app icon and a folder, with orange accents (#E8891C). Vertically tileable — top and bottom edges must connect so it repeats up the pillar without seams. Dark orange-tinted gray background. Flat front-facing orthographic view, no perspective, no camera tilt, even brightness. Fully opaque, no transparency. 1024x1024 pixels, square.
```

## 12. BlockZip.png — ③ Program Files / one face of a cube block

```
A single face texture for a cube: a compressed ZIP folder icon, large and centered in the square. A yellow folder with a zipper across it, and the text ".zip" at the bottom. Orange accent. Plain dark gray background (#14181f). Flat front-facing orthographic view, no perspective, no camera tilt, even lighting. Fully opaque, no transparency. 1024x1024 pixels, square.
```

## 13. FloorApple.png — ④ Apple arena / floor / tileable

```
A top-down floor texture: a silver-white brushed metal surface with Apple-style apple-shaped logo silhouettes arranged in a regular repeating pattern. A boss-arena feel, with faint red danger light (#E81123) bleeding up from below. Do NOT use any circular/radial motif — make it a uniform pattern that tiles across the whole surface. Seamlessly tileable — top/bottom and left/right edges must connect. Flat orthographic straight-down view, no perspective, no camera tilt, even brightness. Fully opaque, no transparency. 1024x1024 pixels, square.
```

## 14. WallDanger.png — ④ Apple arena / wall / tileable

```
A wall texture of a red crash screen — like a Windows Blue Screen of Death but recolored red. A dark-red to red background (#8B0000 to #E81123) with a few big, bold white warning words dominating the image: "CRITICAL" and "FATAL" in large type, plus only about 4 to 6 lines of readable error-code / stack-trace text below. Keep it bold and legible at a glance — NOT a wall of tiny streaming text. Seamlessly tileable — all four edges must connect so it repeats without seams. Flat front-facing orthographic view, no perspective, no camera tilt, even brightness. Fully opaque, no transparency. 1024x1024 pixels, square.
```
