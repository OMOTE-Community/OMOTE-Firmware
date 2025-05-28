# flipper_ir_to_omote.py  working version
"""Generate OMOTE-Firmware device source files (*device_<name>.h/.cpp*) from a
Flipper-IRDB **.ir** file or the legacy converted CSV.

Example
-------
python flipper_ir_to_omote.py \
    --input "https://raw.githubusercontent.com/Lucaslhm/Flipper-IRDB/main/TVs/Sony/Sony_Bravia_KD-55XF80xx-49XF80xx-43XF80xx.ir" \
    --device-name SonyBravia \
    --out-dir ./generated
"""
from __future__ import annotations

import argparse
import csv
import pathlib
import re
import sys
from textwrap import dedent
from typing import Dict, List, Tuple

import requests

# ¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦
# Protocol map: key  (OMOTE constant, total bits, default repeats)
# ¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦
PROTOCOL_MAP: Dict[str, Tuple[str, int, int]] = {
    "SIRC12":    ("IR_PROTOCOL_SONY12",      12, 2),
    "SIRC15":    ("IR_PROTOCOL_SONY15",      15, 2),
    "SIRC20":    ("IR_PROTOCOL_SONY20",      20, 2),
    "NEC":       ("IR_PROTOCOL_NEC",         32, 0),
    "NECEXT":    ("IR_PROTOCOL_NEC",         32, 0),  # Flipper NEC-ext
    "SAMSUNG32": ("IR_PROTOCOL_SAMSUNG32",   32, 0),
    "RC5":       ("IR_PROTOCOL_RC5",         14, 0),
    "RC6-0":     ("IR_PROTOCOL_RC60",        20, 0),
}

NAME_CLEAN_RE = re.compile(r"[^A-Za-z0-9]")
HEX_RE        = re.compile(r"0x([0-9A-Fa-f]+)")
HTTP_RE       = re.compile(r"https?://")

# ¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦
# Helper functions
# ¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦

def reverse_bits(value: int, bits: int) -> int:
    """Reverse *bits* low-order bits of *value*."""
    return int(f"{value:0{bits}b}"[::-1], 2)


def build_code_lsb(proto: str, device: int, sub: int, cmd: int) -> int:
    """Return the LSB-first word sent on the IR bus for *proto*."""
    if proto.startswith("SIRC"):
        b = int(proto[4:])
        if b == 12:
            return (device << 7) | (cmd & 0x7F)
        if b == 15:
            return (sub << 12) | (device << 7) | (cmd & 0x7F)
        if b == 20:
            return (sub << 13) | (device << 7) | (cmd & 0x7F)
    elif proto in ("NEC", "NECEXT", "SAMSUNG32"):
        addr = device & 0xFFFF
        c8   = cmd & 0xFF
        return (
            (addr & 0xFF)
            | ((addr >> 8) << 8)
            | (c8 << 16)
            | ((~c8 & 0xFF) << 24)
        )
    elif proto == "RC5":
        return (1 << 13) | (1 << 12) | ((device & 0x1F) << 6) | (cmd & 0x3F)
    elif proto == "RC6-0":
        return (1 << 19) | (1 << 18) | (0 << 17) | ((device & 0x1F) << 8) | (cmd & 0xFF)
    raise ValueError(f"Unsupported protocol {proto}")


def make_var(label: str) -> str:
    return NAME_CLEAN_RE.sub("_", label.upper())


def bytestr_to_int(s: str | None) -> int:
    """Convert "01 00" / "0x100" / "256" / None  int."""
    if not s:
        return 0
    s = s.strip()
    if " " in s:
        s = "0x" + "".join(s.split())
    return int(s, 0)

# ¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦
# Basic *.ir* and CSV parsers
# ¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦

def parse_ir(text: str) -> List[dict]:
    records, cur = [], {}
    def flush():
        if cur:
            records.append(cur.copy()); cur.clear()
    for line in text.splitlines():
        line = line.strip()
        if not line or line.startswith("#"):
            flush(); continue
        if ":" not in line:
            continue
        k, v = [p.strip() for p in line.split(":", 1)]
        if k.lower() == "name":
            flush(); cur["Function"] = v
        else:
            cur[k.title()] = v
    flush()
    return [r for r in records if r.get("Type", "parsed").lower() != "raw"]


def parse_csv(text: str) -> List[dict]:
    return list(csv.DictReader(text.splitlines()))

# ¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦
# Fetch helper
# ¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦

def load_text(path: str) -> str:
    if HTTP_RE.match(path):
        if "/blob/" in path:
            path = path.replace("https://github.com/", "https://raw.githubusercontent.com/").replace("/blob/", "/")
        resp = requests.get(path, timeout=30)
        resp.raise_for_status()
        return resp.text
    return pathlib.Path(path).read_text(encoding="utf-8")

# ¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦
# Code generator
# ¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦

def generate(rows: List[dict], device_name: str, outdir: pathlib.Path):
    if not rows:
        sys.exit("No parsed signals found.")

    handles: List[str] = []
    statics: List[str] = []
    registers: List[str] = []

    for rec in rows:
        proto_key = (rec.get("Protocol") or rec.get("Type") or "").upper()
        proto_key = proto_key.replace("SONY", "SIRC").replace("PARSED", "").strip()
        if proto_key == "SIRC":
            proto_key = f"SIRC{rec.get('Bits', '12')}"
        if proto_key not in PROTOCOL_MAP:
            continue
        proto_const, bits, rep_default = PROTOCOL_MAP[proto_key]

        label = rec.get("Function", "KEY").strip() or "KEY"
        var   = make_var(label)

        # ---- decode "Data" or individual fields ---------------------------
        code_msb: int
        repeats: int
        data_field = rec.get("Data")
        if data_field:
            data_field = data_field.strip()
            if " " in data_field:  # space-separated bytes
                bytes_le = [int(b, 16) for b in data_field.split()]
                if proto_key.startswith("SIRC") and len(bytes_le) >= 3:
                    addr, cmd = bytes_le[0], bytes_le[-1]
                    code_lsb = build_code_lsb(proto_key, addr, 0, cmd)
                    code_msb = reverse_bits(code_lsb, bits)
                else:
                    code_msb = int("0x" + "".join(f"{b:02X}" for b in bytes_le), 16)
                repeats = int(rec.get("Repeat", rep_default))
            elif HEX_RE.fullmatch(data_field):
                code_msb = int(data_field, 16)
                repeats = int(rec.get("Repeat", rep_default))
            else:
                # Unknown Data format  skip
                continue
        else:
            addr = bytestr_to_int(rec.get("Address") or rec.get("Device"))
            sub  = bytestr_to_int(rec.get("Subdevice") or rec.get("Extended"))
            cmd  = bytestr_to_int(rec.get("Command") or rec.get("Functioncode"))
            code_lsb = build_code_lsb(proto_key, addr, sub, cmd)
            code_msb = reverse_bits(code_lsb, bits)
            repeats  = int(rec.get("Repeat", rep_default))

        handles.append(f"extern uint16_t {var};")
        statics.append(f"uint16_t {var};")
        registers.append(
            f"    register_command(&{var}, makeCommandData(IR, {{std::to_string({proto_const}), \"0x{code_msb:X}:{bits}:{repeats}\"}})); // {label}")

    # join blocks once before entering f-strings to avoid backslash issue
    handles_block   = "\n".join(handles)
    statics_block   = "\n".join(statics)
    registers_block = "\n".join(registers)

    header_text = dedent(f"""
        #pragma once
        // Auto-generated by flipper_ir_to_omote.py
        void register_device_{device_name}(void);

{handles_block}
    """)

    cpp_text = dedent(f"""
        // Auto-generated by flipper_ir_to_omote.py
        #include <string>
        #include \"applicationInternal/commandHandler.h\"
        #include \"applicationInternal/hardware/hardwarePresenter.h\"
        #include \"device_{device_name}.h\"

{statics_block}

        void register_device_{device_name}() {{
{registers_block}
        }}
    """)

    outdir.mkdir(parents=True, exist_ok=True)
    (outdir / f"device_{device_name}.h").write_text(header_text)
    (outdir / f"device_{device_name}.cpp").write_text(cpp_text)
    print(f"Generated {len(handles)} commands  {outdir}")

# ¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦
# CLI
# ¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Flipper-IRDB  OMOTE converter")
    parser.add_argument("--input", required=True, help=".ir or CSV path/URL")
    parser.add_argument("--device-name", required=True, help="Name for output files")
    parser.add_argument("--out-dir", default="./generated", help="Output directory")
    args = parser.parse_args()

    text_data = load_text(args.input)
    rows = parse_ir(text_data) if args.input.lower().endswith(".ir") else parse_csv(text_data)
    generate(rows, args.device_name, pathlib.Path(args.out_dir))
