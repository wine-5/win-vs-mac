#!/usr/bin/env python3
"""ステージエディタ用のローカル開発サーバー。

静的ファイルの配信に加え、エディタの「保存」からの POST を受けて
assets/data/stage-test.json を直接上書きする。ブラウザは任意パスへ書き込めないため、
このサーバーを介して「ダウンロードせずに上書き保存」を実現する。

使い方（リポジトリのルートで実行すること）:
    python tools/serve_editor.py [port]        # 既定ポート 8091

エディタURL: http://127.0.0.1:<port>/tools/stage-editor/
"""
import http.server
import json
import os
import sys
from urllib.parse import urlparse

ROOT = os.getcwd()

# 書き込みを許可する唯一のファイル（安全のため固定）。
# 編集中はこの stage-test.json を使い、完成したら stageData.json へ切り替える。
SAVE_TARGET = os.path.join("assets", "data", "stage-test.json")
SAVE_ENDPOINT = "/api/save-stage"


class Handler(http.server.SimpleHTTPRequestHandler):
    def do_POST(self):
        if urlparse(self.path).path != SAVE_ENDPOINT:
            self.send_error(404, "Not Found")
            return

        length = int(self.headers.get("Content-Length", 0))
        body = self.rfile.read(length)
        try:
            data = json.loads(body)  # 壊れたJSONを書き込まないよう検証する
        except json.JSONDecodeError as e:
            self._json(400, {"ok": False, "error": f"invalid JSON: {e}"})
            return

        try:
            path = os.path.join(ROOT, SAVE_TARGET)
            os.makedirs(os.path.dirname(path), exist_ok=True)
            with open(path, "w", encoding="utf-8") as f:
                json.dump(data, f, ensure_ascii=False, indent=2)
                f.write("\n")
            print(f"[save] {SAVE_TARGET} を上書きしました")
            self._json(200, {"ok": True, "path": SAVE_TARGET.replace(os.sep, "/")})
        except OSError as e:
            self._json(500, {"ok": False, "error": str(e)})

    def _json(self, code, obj):
        payload = json.dumps(obj, ensure_ascii=False).encode("utf-8")
        self.send_response(code)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(payload)))
        self.end_headers()
        self.wfile.write(payload)

    def end_headers(self):
        # 開発用途のため常に最新を取得させる（カタログ/ステージの再取得を確実にする）
        self.send_header("Cache-Control", "no-store")
        super().end_headers()

    def log_message(self, fmt, *args):
        # POSTの保存ログだけ残し、静的配信の大量ログは抑制する
        return


def main():
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 8091
    addr = ("127.0.0.1", port)
    httpd = http.server.ThreadingHTTPServer(addr, Handler)
    print(f"serving {ROOT}")
    print(f"  editor : http://{addr[0]}:{port}/tools/stage-editor/")
    print(f"  save   : POST {SAVE_ENDPOINT} -> {SAVE_TARGET}")
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        pass


if __name__ == "__main__":
    main()
