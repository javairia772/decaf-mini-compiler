from flask import Flask, request, jsonify
from flask_cors import CORS
import subprocess, tempfile, os, json, re

app = Flask(__name__)
CORS(app)

COMPILER_PATH = os.environ.get("COMPILER_PATH", "../submission/decaf_compiler.exe")

def run_compiler(source_code: str) -> dict:
    with tempfile.NamedTemporaryFile(
        mode="w", suffix=".decaf",
        delete=False, encoding="utf-8"
    ) as f:
        f.write(source_code)
        tmp = f.name

    try:
        result = subprocess.run(
            [COMPILER_PATH, tmp],
            capture_output=True, text=True, timeout=10
        )
        raw = result.stdout + result.stderr
        return parse_output(raw, source_code)
    except subprocess.TimeoutExpired:
        return {"error": "Compiler timed out"}
    except FileNotFoundError:
        return {"error": f"Compiler not found at: {COMPILER_PATH}"}
    finally:
        os.unlink(tmp)

def parse_output(raw: str, source: str) -> dict:
    sections = {
        "tokens":    [],
        "rd":        {"success": False, "trace": [], "symtable": []},
        "ll1":       {"success": False, "first": {}, "follow": {},
                      "table": [], "trace": []},
        "lalr":      {"success": False, "action": [], "goto": [], "trace": []},
        "errors":    [],
        "raw":       raw
    }

    lines = raw.splitlines()
    mode  = None

    for line in lines:
        s = line.strip()
        if not s:
            continue

        # ── Section headers ──────────────────────────────────────────
        if "TOKEN STREAM" in s:          mode = "tokens";    continue
        if "RD Parser" in s and "Start" in s: mode = "rd";  continue
        if "FIRST SETS" in s:            mode = "first";     continue
        if "FOLLOW SETS" in s:           mode = "follow";    continue
        if "LL(1) PARSING TABLE" in s:   mode = "ll1table";  continue
        if "LL(1) Parser" in s and "Start" in s: mode = "ll1trace"; continue
        if "LALR(1) ACTION TABLE" in s:  mode = "lalr_act";  continue
        if "LALR(1) GOTO TABLE" in s:    mode = "lalr_goto"; continue
        if "LALR(1) Parser" in s and "Start" in s: mode = "lalr_trace"; continue
        if "SYMBOL TABLE" in s:          mode = "symtable";  continue
        if "ERROR SUMMARY" in s:         mode = "errors";    continue
        if s.startswith("=") or s.startswith("-"): continue

        # ── Token stream ─────────────────────────────────────────────
        if mode == "tokens":
            parts = s.split()
            if len(parts) >= 4:
                sections["tokens"].append({
                    "lexeme": parts[0],
                    "type":   parts[1],
                    "line":   parts[2],
                    "col":    parts[3]
                })

        # ── RD Parser trace ──────────────────────────────────────────
        elif mode == "rd":
            if "SUCCESS" in s:
                sections["rd"]["success"] = True
            elif "FAILED" in s:
                sections["rd"]["success"] = False
            else:
                sections["rd"]["trace"].append(s)

        # ── FIRST sets ───────────────────────────────────────────────
        elif mode == "first":
            m = re.match(r"FIRST\((\w+)\)\s*=\s*\{(.+)\}", s)
            if m:
                nt = m.group(1)
                ts = m.group(2).strip().split()
                sections["ll1"]["first"][nt] = ts

        # ── FOLLOW sets ──────────────────────────────────────────────
        elif mode == "follow":
            m = re.match(r"FOLLOW\((\w+)\)\s*=\s*\{(.+)\}", s)
            if m:
                nt = m.group(1)
                ts = m.group(2).strip().split()
                sections["ll1"]["follow"][nt] = ts

        # ── LL(1) table ──────────────────────────────────────────────
        elif mode == "ll1table":
            parts = s.split(None, 2)
            if len(parts) == 3:
                sections["ll1"]["table"].append({
                    "nt": parts[0], "terminal": parts[1], "production": parts[2]
                })

        # ── LL(1) trace ──────────────────────────────────────────────
        elif mode == "ll1trace":
            if "SUCCESS" in s:
                sections["ll1"]["success"] = True
            elif "FAILED" in s:
                sections["ll1"]["success"] = False
            else:
                parts = s.split(None, 2)
                if len(parts) >= 2:
                    sections["ll1"]["trace"].append({
                        "stack":  parts[0] if len(parts) > 0 else "",
                        "input":  parts[1] if len(parts) > 1 else "",
                        "action": parts[2] if len(parts) > 2 else ""
                    })

        # ── LALR action table ────────────────────────────────────────
        elif mode == "lalr_act":
            parts = s.split(None, 2)
            if len(parts) == 3 and parts[0].isdigit():
                sections["lalr"]["action"].append({
                    "state": int(parts[0]),
                    "terminal": parts[1],
                    "action": parts[2]
                })

        # ── LALR goto table ──────────────────────────────────────────
        elif mode == "lalr_goto":
            parts = s.split(None, 2)
            if len(parts) == 3 and parts[0].isdigit():
                sections["lalr"]["goto"].append({
                    "state": int(parts[0]),
                    "nt": parts[1],
                    "goto": parts[2]
                })

        # ── LALR trace ───────────────────────────────────────────────
        elif mode == "lalr_trace":
            if "SUCCESS" in s:
                sections["lalr"]["success"] = True
            elif "FAILED" in s:
                sections["lalr"]["success"] = False
            elif re.match(r'^\d', s):  # starts with a number = trace row
                # Format: "0 1 4   $ CLASS IDENTIFIER   int x {   SHIFT 7"
                # Split by 2+ spaces to preserve columns
                parts = re.split(r'\s{2,}', s.strip(), maxsplit=3)
                if len(parts) >= 4:
                    sections["lalr"]["trace"].append({
                        "states":  parts[0],
                        "symbols": parts[1],
                        "input":   parts[2],
                        "action":  parts[3]
                    })
                elif len(parts) == 3:
                    sections["lalr"]["trace"].append({
                        "states":  parts[0],
                        "symbols": parts[1],
                        "input":   parts[2],
                        "action":  ""
                    })

        # ── Symbol table ─────────────────────────────────────────────
        elif mode == "symtable":
            if s.startswith("---"):
                continue
            parts = s.split(None, 4)
            if len(parts) >= 4:
                sections["rd"]["symtable"].append({
                    "name":  parts[0],
                    "kind":  parts[1],
                    "type":  parts[2],
                    "line":  parts[3],
                    "extra": parts[4] if len(parts) > 4 else ""
                })

        # ── Errors ───────────────────────────────────────────────────
        elif mode == "errors":
            m = re.match(
                r"\[(LEXICAL|SYNTACTIC|SEMANTIC)\].*?Line\s+(\d+).*?Col\s+(\d+).*?=>\s+(.+)",
                s
            )
            if m:
                sections["errors"].append({
                    "type": m.group(1),
                    "line": int(m.group(2)),
                    "col":  int(m.group(3)),
                    "msg":  m.group(4)
                })

    return sections

@app.route("/compile", methods=["POST"])
def compile_code():
    body = request.get_json(silent=True)
    if not body or "source" not in body:
        return jsonify({"error": "Missing 'source' field"}), 400
    result = run_compiler(body["source"])
    return jsonify(result)

@app.route("/health", methods=["GET"])
def health():
    return jsonify({"status": "ok", "compiler": COMPILER_PATH})

if __name__ == "__main__":
    app.run(port=5000, debug=True)