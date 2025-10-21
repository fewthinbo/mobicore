#!/usr/bin/env python3
"""
update_mobi_config.py

Usage:
  update_mobi_config.py --file /usr/mobile/config.json --bridge-ip 1.2.3.4 --db-pass "S3cr3t" [--local-ip 10.0.0.5]

If --local-ip omitted the script tries to detect the host's outward-facing IPv4.
"""

import argparse
import json
import os
import sys
import time
import socket
from pathlib import Path
from shutil import move

def detect_local_ip():
    """
    Detect an outward-facing IPv4 address by creating a UDP socket to a public IP.
    This does not send data; it's a common technique to learn the outbound IP.
    Falls back to localhost if detection fails.
    """
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
            # use a public IP (no packets sent)
            s.connect(("8.8.8.8", 80))
            ip = s.getsockname()[0]
            if ip and not ip.startswith("127."):
                return ip
    except Exception:
        pass

    # fallback: try hostname resolution
    try:
        h = socket.gethostname()
        ip = socket.gethostbyname(h)
        if ip and not ip.startswith("127."):
            return ip
    except Exception:
        pass

    return "127.0.0.1"

def backup_file(path: Path):
    ts = time.strftime("%Y%m%d-%H%M%S")
    bak = path.with_suffix(path.suffix + f".bak.{ts}")
    move(str(path), str(bak))
    return bak

def ensure_structure(obj):
    if not isinstance(obj, dict):
        return {"db": {"host": "", "user": "mobicore", "password": ""}, "server_bridge": {"host": "", "port_tcp": 32000}}
    if "db" not in obj or not isinstance(obj["db"], dict):
        obj["db"] = {"host": "", "user": "mobicore", "password": ""}
    if "server_bridge" not in obj or not isinstance(obj["server_bridge"], dict):
        obj["server_bridge"] = {"host": "", "port_tcp": 32000}
    return obj

def main():
    p = argparse.ArgumentParser(description="Update /usr/mobile JSON config: db.password, db.host, server_bridge.host")
    p.add_argument("--file", "-f", required=True, help="Path to JSON file to update")
    p.add_argument("--bridge-ip", required=True, help="Bridge server IP or hostname to set server_bridge.host")
    p.add_argument("--db-user", required=True, help="DB user to set for db.user. Default: mobicore")
    p.add_argument("--db-pass", required=True, help="Password to set for db.password")
    p.add_argument("--db-port", help="(Optional) local database port db.port. Default: 3306")
    p.add_argument("--local-ip", help="(Optional) local FreeBSD IP to use for db.host. If omitted script auto-detects")
    args = p.parse_args()

    json_path = Path(args.file)

    if args.local_ip:
        local_ip = args.local_ip
    else:
        local_ip = detect_local_ip()

    # read or create
    if json_path.exists():
        try:
            with json_path.open("r", encoding="utf-8") as fh:
                data = json.load(fh)
        except Exception as e:
            print(f"[WARN] Failed to parse JSON at {json_path}: {e}", file=sys.stderr)
            data = {}
    else:
        data = {}

    data = ensure_structure(data)

    # update fields
    data["db"]["password"] = args.db_pass
    # preserve existing db.user if present; ensure host replaced
    data["db"]["user"] = args.db_user
    data["db"]["host"] = f"{local_ip}:{args.db_port}"
    data["server_bridge"]["host"] = args.bridge_ip

    # backup original file if exists
    if json_path.exists():
        try:
            bak = backup_file(json_path)
            #print(f"[INFO] Existing file backed up to: {bak}")
        except Exception as e:
            #print(f"[ERROR] Could not back up original file: {e}", file=sys.stderr)
            sys.exit(2)
    else:
        # ensure parent dir exists
        json_path.parent.mkdir(parents=True, exist_ok=True)
        print(f"[INFO] Creating new config file at: {json_path}")

    # write to tmp and move atomically
    tmp_path = json_path.with_suffix(json_path.suffix + ".tmp")
    try:
        with tmp_path.open("w", encoding="utf-8") as out:
            json.dump(data, out, indent=4, ensure_ascii=False)
            out.write("\n")
        move(str(tmp_path), str(json_path))
    except Exception as e:
        print(f"[ERROR] Failed to write updated JSON: {e}", file=sys.stderr)
        # try to restore backup if present
        sys.exit(3)
#debug
    '''
    print(f"[OK] Updated JSON saved to: {json_path}")
    print("Summary of applied changes:")
    print(f"  db.host = {data['db']['host']}")
    print(f"  db.password = {data['db']['password']}")
    print(f"  server_bridge.host = {data['server_bridge']['host']}")
    '''
    print("info.json updated successfully")
    return 0

if __name__ == "__main__":
    sys.exit(main())
