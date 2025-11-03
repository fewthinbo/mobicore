#!/usr/bin/env python3

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

def main():
    print(detect_local_ip())
    sys.exit(0)

if __name__ == "__main__":
    sys.exit(main())
