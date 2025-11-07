#!/usr/bin/env python3
"""
update-json.py

Generic JSON field updater that can update any nested field in a JSON file.

Usage:
  update-json.py --file config.json --field db.password:newpass --field server_bridge.host:1.2.3.4
  update-json.py --file config.json --field db.user:admin --field db.port:3306 --field app.debug:true

Field format: path.to.field:value
- Use dot notation for nested fields (e.g., db.password, server_bridge.host)
- Values are automatically typed (string, int, float, bool)
- Creates missing nested objects automatically
"""

import argparse
import json
import os
import sys
import time
from pathlib import Path
from shutil import move

def parse_value(value_str):
    """
    Parse string value to appropriate Python type.
    """
    # Handle boolean values
    if value_str.lower() in ('true', 'false'):
        return value_str.lower() == 'true'
    
    # Handle null/none
    if value_str.lower() in ('null', 'none'):
        return None
    
    # Try to parse as number
    try:
        # Try int first
        if '.' not in value_str:
            return int(value_str)
        else:
            return float(value_str)
    except ValueError:
        pass
    
    # Return as string
    return value_str

def set_nested_field(obj, field_path, value):
    """
    Set a nested field in a dictionary using dot notation.
    Creates missing intermediate objects.
    
    Args:
        obj: Dictionary to modify
        field_path: Dot-separated path (e.g., "db.password")
        value: Value to set
    """
    parts = field_path.split('.')
    current = obj
    
    # Navigate to the parent of the target field
    for part in parts[:-1]:
        if part not in current:
            current[part] = {}
        elif not isinstance(current[part], dict):
            # If existing value is not a dict, replace it
            current[part] = {}
        current = current[part]
    
    # Set the final field
    final_key = parts[-1]
    current[final_key] = value

def backup_file(path: Path):
    """Create a timestamped backup of the file."""
    ts = time.strftime("%Y%m%d-%H%M%S")
    bak = path.with_suffix(path.suffix + f".bak.{ts}")
    move(str(path), str(bak))
    return bak

def main():
    parser = argparse.ArgumentParser(
        description="Generic JSON field updater using dot notation",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s --file config.json --field db.password:secret123
  %(prog)s --file config.json --field db.host:localhost --field db.port:3306
  %(prog)s --file config.json --field server.debug:true --field server.timeout:30
  %(prog)s --file config.json --field app.name:"My App" --field app.version:1.0
        """
    )
    
    parser.add_argument("--file", "-f", required=True, 
                       help="Path to JSON file to update")
    parser.add_argument("--field", action="append", required=True,
                       help="Field to update in format 'path.to.field:value'. Can be used multiple times.")
    parser.add_argument("--no-backup", action="store_true",
                       help="Skip creating backup file")
    parser.add_argument("--create-missing", action="store_true", default=True,
                       help="Create missing intermediate objects (default: true)")
    
    args = parser.parse_args()
    
    # Parse field updates
    field_updates = {}
    for field_spec in args.field:
        if ':' not in field_spec:
            print(f"[ERROR] Invalid field format: {field_spec}. Expected 'path.to.field:value'", file=sys.stderr)
            sys.exit(1)
        
        field_path, value_str = field_spec.split(':', 1)
        field_updates[field_path] = parse_value(value_str)
    
    json_path = Path(args.file)
    
    # Read existing file or create new structure
    if json_path.exists():
        try:
            with json_path.open("r", encoding="utf-8") as fh:
                data = json.load(fh)
        except Exception as e:
            print(f"[ERROR] Failed to parse JSON at {json_path}: {e}", file=sys.stderr)
            sys.exit(2)
    else:
        data = {}
        print(f"[INFO] Creating new JSON file at: {json_path}")
    
    # Ensure data is a dictionary
    if not isinstance(data, dict):
        data = {}
    
    # Apply field updates
    for field_path, value in field_updates.items():
        try:
            set_nested_field(data, field_path, value)
        except Exception as e:
            print(f"[ERROR] Failed to set field {field_path}: {e}", file=sys.stderr)
            sys.exit(3)
    
    # Backup original file if it exists and backup is not disabled
    if json_path.exists() and not args.no_backup:
        try:
            bak = backup_file(json_path)
            # Uncomment for verbose output:
            # print(f"[INFO] Backup created: {bak}")
        except Exception as e:
            print(f"[ERROR] Could not create backup: {e}", file=sys.stderr)
            sys.exit(4)
    
    # Ensure parent directory exists
    json_path.parent.mkdir(parents=True, exist_ok=True)
    
    # Write updated JSON atomically
    tmp_path = json_path.with_suffix(json_path.suffix + ".tmp")
    try:
        with tmp_path.open("w", encoding="utf-8") as out:
            json.dump(data, out, indent=4, ensure_ascii=False)
            out.write("\n")
        move(str(tmp_path), str(json_path))
    except Exception as e:
        print(f"[ERROR] Failed to write updated JSON: {e}", file=sys.stderr)
        if tmp_path.exists():
            tmp_path.unlink()
        sys.exit(5)
    
    print(f"JSON file updated successfully: {json_path}")
    
    # Show summary of changes
    if len(field_updates) <= 5:  # Only show summary for reasonable number of changes
        print("Applied changes:")
        for field_path, value in field_updates.items():
            print(f"  {field_path} = {value}")
    else:
        print(f"Applied {len(field_updates)} field updates")
    
    return 0

if __name__ == "__main__":
    sys.exit(main())