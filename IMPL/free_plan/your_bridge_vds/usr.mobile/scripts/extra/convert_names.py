#!/usr/bin/env python3
import json
import os
import sys

# Configuration
languages = ["tr", "en", "de", "es", "pl", "pt", "it", "ro"]
file_item_names = "item_names"
file_mob_names = "mob_names"
file_extension = ".txt"
file_output = "mt_constants.json"
field_mob = "mob_"
field_item = "item_"

def parse_names_file(file_path):
    """Parse a names file and return a dictionary of vnum -> name mappings"""
    names_dict = {}
    
    if not os.path.exists(file_path):
        print(f"Warning: File {file_path} not found")
        return names_dict
    
    # Try different encodings
    encodings = ['utf-8', 'windows-1254', 'iso-8859-1', 'latin-1', 'cp1252', ]
    
    for encoding in encodings:
        try:
            with open(file_path, 'r', encoding=encoding) as file:
                lines = file.readlines()
            print(f"  - Successfully read {file_path} with {encoding} encoding")
            break
        except UnicodeDecodeError:
            continue
    else:
        print(f"Error: Could not read {file_path} with any encoding")
        return names_dict
    
    try:
            
        # Skip header line if it exists
        for line in lines[1:]:  # Skip first line (VNUM	LOCALE_NAME)
            line = line.strip()
            if not line:
                continue
                
            parts = line.split('\t', 1)  # Split on first tab only
            if len(parts) >= 2:
                try:
                    vnum = parts[0].strip()
                    name = parts[1].strip()
                    if vnum and name:
                        names_dict[vnum] = name
                except (ValueError, IndexError):
                    continue
                    
    except Exception as e:
        print(f"Error reading {file_path}: {e}")
    
    return names_dict

def generate_constants():
    """Generate the mt_constants.json file from all name files"""
    constants = {}
    
    # Process each language
    for lang in languages:
        print(f"Processing language: {lang}")
        
        # Process item names
        item_file = f"{file_item_names}_{lang}{file_extension}"
        item_names = parse_names_file(item_file)
        if item_names:
            constants[f"{field_item}{lang}"] = item_names
            print(f"  - Loaded {len(item_names)} item names from {item_file}")
        
        # Process mob names
        mob_file = f"{file_mob_names}_{lang}{file_extension}"
        mob_names = parse_names_file(mob_file)
        if mob_names:
            constants[f"{field_mob}{lang}"] = mob_names
            print(f"  - Loaded {len(mob_names)} mob names from {mob_file}")
    
    # Write to JSON file
    try:
        with open(file_output, 'w', encoding='utf-8') as f:
            json.dump(constants, f, indent='\t', ensure_ascii=False)
        print(f"\nSuccessfully generated {file_output}")
        print(f"Total sections: {len(constants)}")
        
        # Print summary
        for key, value in constants.items():
            print(f"  - {key}: {len(value)} entries")
            
    except Exception as e:
        print(f"Error writing {file_output}: {e}")

def validate_output():
    """Validate the generated JSON file"""
    if not os.path.exists(file_output):
        print(f"Error: {file_output} was not created")
        return False
    
    try:
        with open(file_output, 'r', encoding='utf-8') as f:
            data = json.load(f)
        
        file_size = os.path.getsize(file_output)
        print(f"\nValidation successful:")
        print(f"  - File size: {file_size:,} bytes ({file_size/1024:.1f} KB)")
        print(f"  - JSON structure is valid")
        
        return True
    except Exception as e:
        print(f"Validation failed: {e}")
        return False

def show_help():
    """Show help information"""
    help_text = """
This script converts item and mob names from text files to a unified JSON format.
Usage: $>python convert_names.py
File format: Each .txt file should have tab-separated values:
"""
    print(help_text)
    print("Expected files in current directory:")
    for lang in languages:
        print(f"  - {file_item_names}_{lang}{file_extension}")
        print(f"  - {file_mob_names}_{lang}{file_extension}")
    print(f"\nOutput file: {file_output}")

if __name__ == "__main__":
    # Check for help argument
    if len(sys.argv) > 1 and sys.argv[1] in ['-h', '--help', 'help']:
        show_help()
        sys.exit(0)
    
    print("=" * 50)
    print("Names to JSON Converter")
    print("=" * 50)
    print("Converting names files to mt_constants.json...")
    
    generate_constants()
    
    if validate_output():
        print("\n✅ Conversion completed successfully!")
    else:
        print("\n❌ Conversion failed!")
    
    print("=" * 50)

