#!/usr/bin/env python3
"""
ir2yaml.py  convert a Flipper-IRDB *.ir file into a YAML description
suitable for OMOTE-Firmware.
"""
import argparse
import pathlib
import re
import sys
from typing import List, Dict

try:
    import yaml          # pip install pyyaml
except ModuleNotFoundError:        # fall back to std-lib emitter
    yaml = None


# ¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦ helpers ¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦
_CMD_RE = re.compile(
    r'#\s*name:\s*(?P<name>[^\s]+)\s+'
    r'type:\s*[^\s]+\s+'
    r'protocol:\s*(?P<proto>\w+)\s+'
    r'address:\s*(?P<addr>(?:[0-9A-F]{2}\s*){4})\s+'
    r'command:\s*(?P<cmd>(?:[0-9A-F]{2}\s*){4})',
    flags=re.I,
)

_REMOTE_RE = re.compile(r'#\s+#\s*(?P<label>.+?)\.ir', re.I)


def _remote_name(buf: str) -> str:
    m = _REMOTE_RE.search(buf)
    raw = m.group('label') if m else 'unknown_remote'
    # sanitise for filename / YAML key
    return (
        raw.replace(',', '')
           .replace(' ', '_')
           .replace('__', '_')
           .strip('_')
    )

def _reverse_bits(value: int, nbits: int) -> int:
    """Return *value* with exactly *nbits* reversed (LSB?MSB)."""
    out = 0
    for i in range(nbits):
        out |= ((value >> i) & 1) << (nbits - 1 - i)
    return out


def make_sirc_hex(address: int, command: int, nbits: int) -> str:
    """Return the *MSB-first* Sony word for 12/15/20-bit frames as `0x`."""
    if nbits not in (12, 15, 20):
        raise ValueError("SIRC only defined for 12, 15 or 20 bits")

    raw_lsb_first = (address << 7) | command  # command in lower 7 bits
    word = _reverse_bits(raw_lsb_first, nbits)
    hex_digits = (nbits + 3) // 4  # 123 digits, 15/164, 205
    return f"0x{word:0{hex_digits}X}"

def _parse_commands(buf: str) -> List[Dict]:
    commands = []
    for m in _CMD_RE.finditer(buf):
        name = m['name']
        proto = m['proto'].upper()
        addr_bytes = m['addr'].strip().split()
        cmd_bytes = m['cmd'].strip().split()

        # First byte is all OMOTE needs for Sony / NEC / RC-5 
        addr_hex = f"0x{addr_bytes[0]}"
        cmd_hex = f"0x{cmd_bytes[0]}"

        # Map Flipper protocol names to the constants you pass to
        # makeCommandData().  Extend this dict as needed.
        proto_const = {
            "SIRC":    "IR_SONY",
            "SIRC15":  "IR_SONY",   # 15-bit Sony
            "NEC":     "IR_NEC",
            "RC5":     "IR_RC5",
            "RC6":     "IR_RC6",
        }.get(proto, f"IR_{proto}")

        nbits = 12
        if proto == "SIRC15":
            nbits = 15
        commands.append(
            {
                "name": name,
                "protocol": proto,
                "address": addr_hex,
                "command": cmd_hex,
                # This is exactly the list you hand over when you build the
                # command with makeCommandData(IR, payloads)
                "makeCommandPayload": [proto_const, addr_hex, cmd_hex],
                "full": make_sirc_hex(int(m['addr'].split()[0],16), int(m['cmd'].split()[0], 16), nbits)
            }
        )
    return commands


def _dump_yaml(data, stream=None):
    if yaml:
        yaml.safe_dump(data, stream or sys.stdout,
                       sort_keys=False,
                       default_flow_style=False)
    else:
        import json      # very tiny fallback
        json.dump(data, stream or sys.stdout, indent=2)


# ¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦ main ¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦
def main() -> None:
    ap = argparse.ArgumentParser(
        description="Convert Flipper *.ir file  OMOTE YAML."
    )
    ap.add_argument("src", help="Path or URL to the *.ir file")
    ap.add_argument("-o", "--output", metavar="FILE",
                    help="write YAML to file instead of stdout")
    ns = ap.parse_args()

    # Read source (URL or local)
    if ns.src.startswith(("http://", "https://")):
        import requests
        text = requests.get(ns.src, timeout=15).text
    else:
        text = pathlib.Path(ns.src).read_text(encoding="utf-8")

    data = {_remote_name(text): _parse_commands(text)}

    if ns.output:
        with open(ns.output, "w", encoding="utf-8") as fp:
            _dump_yaml(data, fp)
    else:
        _dump_yaml(data)


if __name__ == "__main__":
    main()
