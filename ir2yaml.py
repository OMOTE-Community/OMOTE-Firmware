#!/usr/bin/env python3
"""
ir2yaml.py Â Flipper-IRDB  ?  OMOTE-Firmware

* Supports protocols: SONY SIRC12/15/20, NEC/NECx, RC5, RC6,
  Denon-15-bit, Denon-Kaseikyo-48-bit, Panasonic/Kaseikyo-48-bit
* Works with both Âtype: parsedÂ and Âtype: rawÂ files.
* Outputs:  remote_name:
              - name: Power
                protocol: NEC
                hex: 0x20DF10EF
                bits: kNECBits
                repeat: kNoRepeat
                makeCommandParams:
                       concatenateIRsendParams("0x20DF10EF",
                                               kNECBits,
                                               kNoRepeat)
"""
from __future__ import annotations
import argparse, pathlib, re, sys, textwrap, typing as T
try:
    import yaml      # pip install pyyaml
except ImportError:   # minimal fallback
    yaml = None

# ------------------------- CONSTANT MAPS --------------------------
PROTO_INFO = {
    #    Flipper tag     bits  bits-const       OMOTE enum
    "SIRC":     (12, "kSony12Bits",  "IR_PROTOCOL_SONY"),
    "SIRC12":   (12, "kSony12Bits",  "IR_PROTOCOL_SONY"),
    "SIRC15":   (15, "kSony15Bits",  "IR_PROTOCOL_SONY"),
    "SIRC20":   (20, "kSony20Bits",  "IR_PROTOCOL_SONY"),
    "NEC":      (32, "kNECBits",     "IR_PROTOCOL_NEC"),
    "NECX":     (32, "kNECBits",     "IR_PROTOCOL_NEC"),
    "RC5":      (13, "kRC5Bits",     "IR_PROTOCOL_RC5"),
    "RC6":      (20, "kRC6Bits",     "IR_PROTOCOL_RC6"),
    "DENON":    (15, "kDenonBits",   "IR_PROTOCOL_DENON"),
    "DENON48":  (48, "kDenon48Bits", "IR_PROTOCOL_DENON"),
    "KASEIKYO": (48, "kKaseikyoBits","IR_PROTOCOL_KASEIKYO"),
}

REPEAT_CONST = 0     # almost always good enough

# -------------------------- REGEX HELPERS -------------------------
RX_ITEM = re.compile(
    r"#\s*name:\s*(?P<name>[^\n#]+?)\s+"
    r"type:\s*(?P<type>parsed|raw)\s+"
    r"(?:protocol:\s*(?P<proto>\w+)\s+"
    r"address:\s*(?P<addr>(?:[0-9A-F]{2}\s*){4})\s+"
    r"command:\s*(?P<cmd>(?:[0-9A-F]{2}\s*){4})|"
    r"data:\s*(?P<raw>[\d\s]+))",
    flags=re.I,
)
RX_REMOTE = re.compile(r"#\s+#\s*(?P<label>.+?\.ir)", re.I)

hexbyte = lambda b: int(b, 16)     # tiny helper


# ------------------------- BUILDER FUNCTIONS ----------------------

def _rev(val: int, nbits: int) -> int:
    out = 0
    for _ in range(nbits):
        out = (out << 1) | (val & 1)
        val >>= 1
    return out

def build_sirc(addr_b: list[str], cmd_b: list[str], bits: int) -> int:
    addr = int(addr_b[0], 16)
    cmd  = int(cmd_b[0], 16)

    if bits == 12:                         # 7-bit cmd, 5-bit addr
        return _rev(addr, 5) | (_rev(cmd, 7) << 5)

    if bits == 15:                         # 7-bit cmd, 8-bit addr
        return _rev(addr, 8) | (_rev(cmd, 7) << 8)

    if bits == 20:                         # 7-bit cmd, 5-bit ext, 8-bit addr
        ext = int(addr_b[1], 16) & 0x1F    # second byte carries the ÂextendedÂ field
        return (_rev(addr, 8) |
                (_rev(ext, 5) << 8) |
                (_rev(cmd, 7) << 13))

    raise ValueError("Unsupported SIRC bit-length")

def build_nec(addr_b: list[str], cmd_b: list[str]) -> int:
    """NEC 32-bit: addr, ÂŹaddr, cmd, ÂŹcmd."""
    a = hexbyte(addr_b[0])
    c = hexbyte(cmd_b[0])
    return a | ((~a & 0xFF) << 8) | (c << 16) | ((~c & 0xFF) << 24)


def build_rc5(addr_b: list[str], cmd_b: list[str]) -> int:
    """RC5 13-bit word (two start bits, field, 5-bit addr, 6-bit cmd)."""
    addr = hexbyte(addr_b[0]) & 0x1F
    cmd  = hexbyte(cmd_b[0]) & 0x3F
    field = 0 if cmd < 0x20 else 1
    cmd &= 0x1F
    return (0b11 << 11) | (field << 10) | (addr << 6) | cmd


def build_rc6(addr_b: list[str], cmd_b: list[str]) -> int:
    """RC6 mode-0 20-bit (start, mode, trailer, addr, cmd)."""
    addr = hexbyte(addr_b[0])
    cmd  = hexbyte(cmd_b[0])
    return (0b1 << 19) | (0 << 18) | (0b0 << 17) | (addr << 8) | cmd


def build_denon15(addr_b: list[str], cmd_b: list[str]) -> int:
    """Denon 15-bit frame: 5 addr + 8 cmd + '00' frame bits."""
    addr = hexbyte(addr_b[0]) & 0x1F
    cmd  = hexbyte(cmd_b[0])
    return (cmd << 7) | (addr << 2) | 0b00



def build_kaseikyo(addr_b: list[str], cmd_b: list[str]) -> int:
    """
    Convert analyser output

        address: 'B0 02 20 00'
        command: 'D0 03 00 00'

    to the 48‑bit Panasonic / Kaseikyo frame expected by the
    legacy IRremote API (0xVVVVPPAA CC SS).

    Returns a hex string such as '0x40040D00BCB1'.
    """

    # --- helpers ------------------------------------------------------------
    def revbits(val: int, n: int) -> int:
        out = 0
        for i in range(n):
            if val & (1 << i):
                out |= 1 << (n - 1 - i)
        return out

    rev8  = lambda b: revbits(b, 8)     # byte reverse
    rev16 = lambda w: revbits(w, 16)    # word reverse

    # --- parse the four space‑separated bytes from each line ----------------
    adr = [int(b, 16) for b in addr_b]
    cmd = [int(b, 16) for b in cmd_b]

    # ------------------------------------------------------------------------
    # 1.  Vendor (16 bits)  … bytes 1–2 of the address word (little‑endian)
    #     Reverse the whole 16 bits because Kaseikyo is sent LSB‑first
    vendor_le = adr[1] | (adr[2] << 8)
    vendor    = rev16(vendor_le)              # 0x4004 for Panasonic TV

    # 2.  Vendor‑parity nibble (4 bits) = XOR of all four nibbles of vendor
    v_parity  = (vendor >> 12) ^ (vendor >> 8) ^ (vendor >> 4) ^ vendor
    v_parity &= 0xF

    # 3.  12‑bit address:
    #     ‑ upper 4 bits come from the LSB‑first reversed BYTE‑0
    #     ‑ lower 8 bits come from BYTE‑3 (also bit‑reversed)
    adr_hi_nib = rev8(adr[0]) & 0xF            # 0xD
    adr_lo_byte = rev8(adr[3])                 # 0x00

    byte2 = (v_parity << 4) | adr_hi_nib       # 0x0D
    byte3 = adr_lo_byte                        # 0x00

    # 4.  8‑bit command:
    #     The analyser gave a 16‑bit little‑endian word; reverse 16 bits,
    #     then ignore the low 4 padding bits.
    cmd16_le = cmd[0] | (cmd[1] << 8)
    command  = (rev16(cmd16_le) >> 4) & 0xFF   # 0xBC

    # 5.  8‑bit checksum = address_hi_byte ^ address_lo_byte ^ command
    checksum = byte2 ^ byte3 ^ command         # 0xB1

    # 6.  Pack into 48 bits: VVVV PPAA CC SS
    frame48 = (vendor << 32) | (byte2 << 24) | (byte3 << 16) \
              | (command << 8) | checksum

    # return f"0x{frame48:012X}"                 # '0x40040D00BCB1'
    return frame48


# Quick jump-table
BUILDERS: dict[str, T.Callable[[list[str], list[str], int | None], int]] = {
    "SIRC":     lambda a, c, _: build_sirc(a, c, 12),
    "SIRC12":   lambda a, c, _: build_sirc(a, c, 12),
    "SIRC15":   lambda a, c, _: build_sirc(a, c, 15),
    "SIRC20":   lambda a, c, _: build_sirc(a, c, 20),
    "NEC":      lambda a, c, _: build_nec(a, c),
    "NECX":     lambda a, c, _: build_nec(a, c),
    "RC5":      lambda a, c, _: build_rc5(a, c),
    "RC6":      lambda a, c, _: build_rc6(a, c),
    "DENON":    lambda a, c, _: build_denon15(a, c),
    "DENON48":  lambda a, c, _: build_kaseikyo(a, c),
    "KASEIKYO": lambda a, c, _: build_kaseikyo(a, c),
}

# -------------- MINIMAL Âraw-DENON-48Â THRESHOLD DECODER ----------
RX_DENON_RAW = re.compile(r"data:\s*(?P<pulses>[\d\s]+)", re.I)
def raw_to_denon48(pulses: str) -> int:
    """Best-effort decode of 48-bit Denon-Kaseikyo frames from Flipper raw."""
    times = list(map(int, pulses.split()))
    # pick marks/spaces after header (= after first 2 entries)
    bits = []
    for mark, space in zip(times[2::2], times[3::2]):
        if len(bits) >= 48:
            break
        bits.append(1 if space > 1000 else 0)
    value = 0
    for b in bits[::-1]:          # invert to MSB first
        value = (value << 1) | b
    return value


# ----------------------------- CORE PARSER ------------------------
def parse_file(buf: str) -> tuple[str, str, list[dict]]:
    m = RX_REMOTE.search(buf)
    remote_name = (m.group("label").replace(",", "")
                                  .replace(" ", "_")
                                  .strip("_")
                   if m else "unknown_remote")

    entries = []
    for m in RX_ITEM.finditer(buf):
        name = m["name"].strip().replace(" ", "_")
        if m["type"].lower() == "parsed":
            proto = m["proto"].upper()
            addr  = m["addr"].strip().split()
            cmd   = m["cmd"].strip().split()

            # special-case Denon: Flipper uses plain ÂDENONÂ for both 15 & 48
            if proto == "DENON" and len(cmd) >= 2 and hexbyte(cmd[1]) != (~hexbyte(cmd[0]) & 0xFF):
                proto_key = "DENON48"
            else:
                proto_key = proto

            if proto_key not in BUILDERS:
                continue  # Unsupported yet
            hex_int = BUILDERS[proto_key](addr, cmd, None)
            bits, bits_const, _ = PROTO_INFO[proto_key]
        else:                       # raw entry (often DENON48)
            raw = m["raw"]
            hex_int = raw_to_denon48(raw)
            proto_key = "DENON48"
            bits, bits_const, _ = PROTO_INFO[proto_key]

        hex_str = f"0x{hex_int:0{bits//4}X}"
        entries.append({
            "name": name,
            "data": hex_str,
            "nbits": bits,
            "repeats": REPEAT_CONST,
        })
    return remote_name, proto, entries


# ------------------------------- MAIN -----------------------------
def main() -> None:
    ap = argparse.ArgumentParser(
        formatter_class=argparse.RawDescriptionHelpFormatter,
        description=textwrap.dedent("""\
            Convert Flipper IRDB *.ir ? OMOTE YAML with concatenateIRsendParams.
            Examples
            --------
            python ir2yaml.py Sony_Bravia.ir > bravia.yaml
            python ir2yaml.py https://..../Panasonic_SC-HC58.ir -o panasonic.yaml
        """))
    ap.add_argument("src", help="Path *or* URL of the .ir file")
    ap.add_argument("-o", "--output", help="Write YAML to file")
    args = ap.parse_args()

    if args.src.startswith(("http://", "https://")):
        import requests
        buf = requests.get(args.src, timeout=20).text
    else:
        buf = pathlib.Path(args.src).read_text(encoding="utf-8")

    rname, proto, commands = parse_file(buf)
    yaml_out = {"protocol": proto, "commands": commands}

    ostream = open(args.output, "w", encoding="utf-8") if args.output else sys.stdout
    if yaml:
        yaml.safe_dump(yaml_out, ostream, sort_keys=False, default_flow_style=False)
    else:   # tiny fallback
        import json
        json.dump(yaml_out, ostream, indent=2)
    if args.output:
        ostream.close()


if __name__ == "__main__":
    main()
