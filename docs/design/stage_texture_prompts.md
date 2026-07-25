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
- [ ] 5. FloorTerminal.png
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

## 5. FloorTerminal.png — ② System32 / floor / tileable

```
A top-down floor texture of a terminal / console: a black background densely filled with green monospaced command-line logs and code, with occasional amber warning lines. Green (#3FB950) dominant. Seamlessly tileable — top/bottom and left/right edges must connect and text lines must not get cut off at the seams so it repeats cleanly. Flat orthographic straight-down view, no perspective, no camera tilt, even and uniform brightness. Fully opaque, no transparency. 1024x1024 pixels, square.
```

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
A wall texture showing a Windows Registry Editor style UI seen straight on. A hierarchical key tree on the left (HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER, etc.) and a right panel with "Name, Type, Data" columns listing registry values. Dark background with green-to-amber text, a hard technical System32 atmosphere. A complete single image (not tiled). Flat front-facing orthographic view, no perspective, no camera tilt, even lighting. Fully opaque, no transparency. 1024x1024 pixels, square.
```

## 9. WallData.png — ②③ / wall / vertically tileable

```
A "server interior" wall texture: a black background with source code from several programming languages (C++, Python, JavaScript) and system logs streaming densely in vertical columns. Text color mainly amber-to-orange (#D29922 to #E8891C) with a few red [ERROR] lines. Vertically tileable — top and bottom edges must connect, with lines spaced evenly so nothing is cut off at the seam when repeated vertically. Flat front-facing orthographic view, no perspective, no camera tilt, even brightness. Fully opaque, no transparency. 1024x1024 pixels, square.
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
A wall texture of a red crash screen — like a Windows Blue Screen of Death but recolored red. A dark-red to red background (#8B0000 to #E81123) with white warning text and error codes streaming across it: words like "CRITICAL", "FATAL", and stack-trace style lines. Seamlessly tileable — all four edges must connect so it repeats without seams. Flat front-facing orthographic view, no perspective, no camera tilt, even brightness. Fully opaque, no transparency. 1024x1024 pixels, square.
```
