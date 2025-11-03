#!/usr/bin/env python3
"""
jsonfield-value.py

Read field values from JSON files using dot notation.
Outputs only the value for easy shell script integration.

Usage:
  jsonfield-value.py --file config.json --field db.password
  jsonfield-value.py --file config.json --field server_bridge.host
  jsonfield-value.py --file config.json --field app.features.logging

Shell script usage:
  PASSWORD=$(python jsonfield-value.py --file config.json --field db.password)
  HOST=$(python jsonfield-value.py --file config.json --field server_bridge.host)
"""

import argparse
import json
import sys
from pathlib import Path

def get_nested_field(obj, field_path):
    """
    Get a nested field value from a dictionary using dot notation.
    
    Args:
        obj: Dictionary to read from
        field_path: Dot-separated path (e.g., "db.password")
    
    Returns:
        The field value or None if not found
    """
    parts = field_path.split('.')
    current = obj
    
    try:
        for part in parts:
            if not isinstance(current, dict):
                return None
            if part not in current:
                return None
            current = current[part]
        return current
    except (KeyError, TypeError):
        return None

def format_output(value, output_format):
    """
    Format the output value based on the specified format.
    """
    if value is None:
        return ""
    
    if output_format == "json":
        return json.dumps(value, ensure_ascii=False)
    elif output_format == "raw":
        if isinstance(value, bool):
            return "true" if value else "false"
        elif isinstance(value, (int, float)):
            return str(value)
        elif isinstance(value, str):
            return value
        else:
            return json.dumps(value, ensure_ascii=False)
    else:  # default format
        if isinstance(value, bool):
            return "true" if value else "false"
        elif isinstance(value, (int, float, str)):
            return str(value)
        else:
            return json.dumps(value, ensure_ascii=False)

def main():
    parser = argparse.ArgumentParser(
        description="Read field values from JSON files using dot notation",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s --file config.json --field db.password
  %(prog)s --file config.json --field server_bridge.host
  %(prog)s --file config.json --field app.features.logging
  %(prog)s --file config.json --field db --format json

Shell script usage:
  PASSWORD=$(%(prog)s --file config.json --field db.password)
  DEBUG=$(%(prog)s --file config.json --field app.debug)
        """
    )
    
    parser.add_argument("--file", "-f", required=True,
                       help="Path to JSON file to read")
    parser.add_argument("--field", required=True,
                       help="Field path to read using dot notation (e.g., 'db.password')")
    parser.add_argument("--format", choices=["default", "json", "raw"], default="default",
                       help="Output format: default (smart), json (JSON format), raw (as-is)")
    parser.add_argument("--default", 
                       help="Default value to return if field not found")
    parser.add_argument("--quiet", "-q", action="store_true",
                       help="Suppress error messages")
    
    args = parser.parse_args()
    
    json_path = Path(args.file)
    
    # Check if file exists
    if not json_path.exists():
        if not args.quiet:
            print(f"Error: File not found: {json_path}", file=sys.stderr)
        if args.default is not None:
            print(args.default)
            return 0
        return 1
    
    # Read and parse JSON file
    try:
        with json_path.open("r", encoding="utf-8") as fh:
            data = json.load(fh)
    except json.JSONDecodeError as e:
        if not args.quiet:
            print(f"Error: Invalid JSON in {json_path}: {e}", file=sys.stderr)
        if args.default is not None:
            print(args.default)
            return 0
        return 2
    except Exception as e:
        if not args.quiet:
            print(f"Error: Failed to read {json_path}: {e}", file=sys.stderr)
        if args.default is not None:
            print(args.default)
            return 0
        return 3
    
    # Get the field value
    value = get_nested_field(data, args.field)
    
    if value is None:
        if args.default is not None:
            print(args.default)
            return 0
        elif not args.quiet:
            print(f"Error: Field '{args.field}' not found", file=sys.stderr)
            return 4
        else:
            return 4
    
    # Output the value
    output = format_output(value, args.format)
    print(output)
    return 0

if __name__ == "__main__":
    sys.exit(main())