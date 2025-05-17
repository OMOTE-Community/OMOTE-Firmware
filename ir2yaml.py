#!/usr/bin/env python3
"""ir2yaml.py  Convert Flipper-Zero *.ir* dumps to an ESPHome-friendly YAML list

The script now understands three kinds of input in **one file**:

1. **Flipper Zero .ir** (as kept in the IRDB)  detected by extension *or* the
   leading line `Filetype: IR signals file`.
2. A plain YAML sequence of dictionaries.
3. A JSON array of objects.

For **Sony SIRC / SIRC15 / SIRC20** entries the helper builds the *normal* MSB-
first hex word (e.g. `0xC90`) and drops the low-level `address`/`command`
fields.  All other protocols are passed through unchanged  they still keep the
raw address/command bytes because the conversion rules differ per protocol.

Example
=======
```bash
$ python ir2yaml.py Sony_Bravia.ir > sony.yaml
```

Input (excerpt)
```
# name: Vol_dn type: parsed protocol: SIRC address: 01 00 00 00 command: 13 00 00 00
```
Output
```
- name: Vol_dn
  protocol: SIRC
  data: 0xC90
  nbits: 12
```
"""
from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path
from typing import Any, Dict, List

###############################################################################
# Bit helpers
###############################################################################

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

###############################################################################
# .ir parser
###############################################################################

_RE_ITEM = re.compile(
    r"#\s*name:\s*(?P<name>.+?)\s+type:\s*parsed\s+protocol:\s*(?P<proto>\S+)"
    r"\s+address:\s*(?P<addr>(?:[0-9A-Fa-f]{2}\s+){0,3}[0-9A-Fa-f]{2})"
    r"\s+command:\s*(?P<cmd>(?:[0-9A-Fa-f]{2}\s+){0,3}[0-9A-Fa-f]{2})",
    re.I | re.S,
)


def parse_ir_file(text: str) -> List[Dict[str, Any]]:
    """Return a list of command dicts extracted from a Flipper *.ir* dump."""
    out: List[Dict[str, Any]] = []
    for m in _RE_ITEM.finditer(text):
        name = m.group("name").strip().replace(" ", "_")
        proto = m.group("proto").upper()
        addr_b = int(m.group("addr").split()[0], 16)
        cmd_b = int(m.group("cmd").split()[0], 16)
        out.append({
            "name": name,
            "protocol": proto,
            "address": addr_b,
            "command": cmd_b,
        })
    if not out:
        raise ValueError("No IR items found  malformed .ir file?")
    return out

###############################################################################
# Generic loader  JSON, YAML, or .ir
###############################################################################


def load_input(path: Path) -> List[Dict[str, Any]]:
    text = path.read_text(encoding="utf-8", errors="replace")

    if path.suffix.lower() == ".ir" or text.lstrip().startswith("Filetype:"):
        return parse_ir_file(text)

    try:
        return json.loads(text)
    except json.JSONDecodeError:
        pass

    try:
        import yaml  # type: ignore

        data = yaml.safe_load(text)
        if isinstance(data, list):
            return data
    except Exception:
        pass

    raise ValueError("Input file is not .ir, JSON, or YAML list")

###############################################################################
# Transformation per entry
###############################################################################

_SIRC_PATTERN = re.compile(r"SIRC(\d+)?", re.I)


def process_entry(entry: Dict[str, Any]) -> Dict[str, Any]:
    proto_raw = entry.get("protocol", "")
    m = _SIRC_PATTERN.fullmatch(proto_raw)
    if not m:
        return entry  # untouched for non-Sony protocols

    nbits = int(m.group(1)) if m.group(1) else 12
    address = int(entry["address"])
    command = int(entry["command"])

    entry = entry.copy()
    entry["data"] = make_sirc_hex(address, command, nbits)
    entry["nbits"] = nbits
    # Clean up legacy fields
    entry.pop("address", None)
    entry.pop("command", None)
    # Normalise protocol label to plain SIRC for YAML cleanliness
    entry["protocol"] = "SIRC"
    return entry

###############################################################################
# YAML emitter
###############################################################################


def dump_yaml(obj: Any):
    import yaml  # type: ignore

    yaml.safe_dump(obj, sys.stdout, sort_keys=False, default_flow_style=False)

###############################################################################
# Main CLI
###############################################################################


def main(argv: List[str] | None = None) -> None:
    p = argparse.ArgumentParser(description="Convert Flipper .ir / JSON / YAML to ESPHome YAML")
    p.add_argument("input", type=Path, help="Input file (.ir, .yaml, .json)")
    args = p.parse_args(argv)

    commands = [process_entry(e) for e in load_input(args.input)]
    dump_yaml(commands)


if __name__ == "__main__":  # pragma: no cover
    main()
