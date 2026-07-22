#!/usr/bin/env python3
"""
 * ccpretreat.py
 * Created by XMZ <xmz-team@outlook.com> on 08/07/26
 * Copyright (c) 2025-2026 XMZ <xmz-team@outlook.com> All rights reserved.
"""
# It’s just a toy, not a serious thing
# If you think it’s cool to use it in a serious environment (such as a production environment), then you can use it. Just remember not to be beaten to death.

import re
import sys
import os
import argparse
import shutil

CPP_EXTENSIONS = {'.cc', '.cpp', '.cxx', '.c++', '.C', '.h', '.hpp', '.hxx', '.h++', '.H'}

def generate_classname(returntype, funcname, filename, counter):
    def encode_type(type_str):
        type_str = type_str.strip()
        result = []
        i = 0
        while i < len(type_str):
            c = type_str[i]
            # Handle const
            if type_str[i:].startswith('const'):
                # Check if next non-space token is * or &
                remaining = type_str[i+5:].lstrip()
                if remaining.startswith('*') or remaining.startswith('&'):
                    result.append('K')
                else:
                    result.append('K')
                i += 5
                continue
            # Handle volatile
            if type_str[i:].startswith('volatile'):
                remaining = type_str[i+8:].lstrip()
                if remaining.startswith('*') or remaining.startswith('&'):
                    result.append('V')
                else:
                    result.append('V')
                i += 8
                continue
            # Handle pointers
            if c == '*':
                # Check if preceded by const or volatile
                prev_type = type_str[:i].rstrip()
                has_const = prev_type.endswith('const')
                has_volatile = prev_type.endswith('volatile')
                
                if has_const and has_volatile:
                    result.append('PVK')
                elif has_volatile and has_const:
                    result.append('PVK')
                elif has_const:
                    result.append('PK')
                elif has_volatile:
                    result.append('PV')
                else:
                    result.append('P')
                i += 1
                # Skip const/volatile after pointer (like int* const)
                remaining = type_str[i:].lstrip()
                if remaining.startswith('const') or remaining.startswith('volatile'):
                    # These are not encoded
                    if remaining.startswith('const'):
                        i += 5
                    elif remaining.startswith('volatile'):
                        i += 8
                    # Skip any spaces
                    while i < len(type_str) and type_str[i].isspace():
                        i += 1
                continue
            # Handle references
            if c == '&':
                if i + 1 < len(type_str) and type_str[i+1] == '&':
                    result.append('O')  # &&
                    i += 2
                else:
                    result.append('R')  # &
                    i += 1
                continue
            # Handle template brackets
            if c == '<':
                result.append('I')
                i += 1
                continue
            if c == '>':
                result.append('E')
                i += 1
                continue
            # Handle basic types and identifiers
            if c.isalpha() or c == '_':
                start = i
                while i < len(type_str) and (type_str[i].isalnum() or type_str[i] == '_'):
                    i += 1
                word = type_str[start:i]
                # Check if it's a basic type
                basic_types = {
                    'void': 'v', 'int': 'i', 'char': 'c', 'bool': 'b',
                    'float': 'f', 'double': 'd', 'long': 'l', 'short': 's',
                    'unsigned': 'j', 'signed': 'a', 'wchar_t': 'w'
                }

                if word in basic_types:
                    result.append(basic_types[word])
                else:
                    # It's a user-defined type, encode with length prefix
                    result.append(f'{len(word)}{word}')
                continue
            # Skip spaces and other characters
            if c.isspace() or c in [',', '[', ']']:
                i += 1
                continue
            # For any other character, just append it
            result.append(c)
            i += 1
        return ''.join(result)

    def encode_params(params_str):
        if not params_str.strip():
            return 'E0'  # No parameters
        # Split parameters by comma, but need to handle nested templates
        params = []
        depth = 0
        current = []
        for c in params_str:
            if c in '<(':
                depth += 1
                current.append(c)
            elif c in '>)':
                depth -= 1
                current.append(c)
            elif c == ',' and depth == 0:
                params.append(''.join(current).strip())
                current = []
            else:
                current.append(c)
        if current:
            params.append(''.join(current).strip())
        # Encode each parameter
        encoded_params = [encode_type(p) for p in params if p.strip()]
        # Take first letter of first parameter's encoding
        first_char = encoded_params[0][0] if encoded_params else '0'
        return f'E{first_char}{len(params)}'
    # Clean the return type and function name
    clean_returntype = returntype.strip()
    clean_funcname = funcname.strip()
    # Encode return type and parameters
    encoded_return = encode_type(clean_returntype)
    # Build the class name
    funcname_len = len(clean_funcname)
    parts = [
        '_ZXMZN',
        'pub',
        str(funcname_len),
        clean_funcname,
        'E',
        encoded_return
    ]

    classname = ''.join(parts)
    # Add counter if needed to avoid conflicts
    if counter > 0:
        classname += f'_{counter}'
    # Clean up any invalid characters for C++ identifiers
    classname = re.sub(r'[^a-zA-Z0-9_]', '_', classname)
    classname = re.sub(r'_+', '_', classname)
    
    return classname

def is_cpp_file(filename):
    ext = os.path.splitext(filename)[1]
    return ext in CPP_EXTENSIONS

def replace_func_calls(line, func_info):
    modified_line = line
    for funcname in func_info:
        # replace %call funcname(args) -> funcname.funcname(args)
        modified_line = re.sub(
            rf'%call\s+{funcname}\s*\(',
            f'{funcname}.{funcname}(',
            modified_line
        )
        # replace ordinary calls funcname(args) -> funcname.funcname(args)
        modified_line = re.sub(
            rf'(?<![.\w])(?<!\w\.)(?<!::){funcname}\s*\(',
            f'{funcname}.{funcname}(',
            modified_line
        )
    return modified_line

def process_cc_content(content, filename):
    func_pattern = re.compile(
        r'^(\s*)(pub)\s+'
        r'([\w\s*&:<>,\[\]]+?)\s+'
        r'(\w+)\s*'
        r'\(([^)]*)\)\s*'
        r'\{',
        re.MULTILINE
    )
    func_info = {}  # funcname -> classname
    counter = 0
    for match in func_pattern.finditer(content):
        returntype = match.group(3).strip()
        funcname = match.group(4).strip()
        classname = generate_classname(returntype, funcname, filename, counter)
        func_info[funcname] = classname
        counter += 1
    if not func_info:
        return content
    lines = content.split('\n')
    output = []
    i = 0
    while i < len(lines):
        line = lines[i]
        match = func_pattern.match(line)
        if match:
            # definition of replacement function
            indent = match.group(1)
            returntype = match.group(3).strip()
            funcname = match.group(4).strip()
            params = match.group(5).strip()
            classname = func_info[funcname]
            # class definition
            class_lines = [
                f'{indent}class {classname} {{',
                f'{indent}public:',
                f'{indent}    {returntype} {funcname}({params}) {{'
            ]
            output.extend(class_lines)
            brace_depth = line.count('{') - line.count('}')
            i += 1
            while i < len(lines) and brace_depth > 0:
                current_line = lines[i]
                if current_line.strip():
                    output.append(f'    {current_line}')
                else:
                    output.append(current_line)
                brace_depth += current_line.count('{') - current_line.count('}')
                i += 1
            output.append(f'{indent}}};')
        else:
            # handle the %new tag
            modified_line = line
            # handle %new { func1 func2 ... } %end
            new_block_pattern = r'^\s*%new\s*\{'
            if re.match(new_block_pattern, modified_line):
                base_indent = re.match(r'^(\s*)', modified_line).group(1)
                single_line_match = re.match(r'^\s*%new\s*\{\s*(.+?)\s*\}\s*%end\s*$', modified_line)
                if single_line_match:
                    funcnames = single_line_match.group(1).split()
                    for fn in funcnames:
                        if fn in func_info:
                            output.append(f'{base_indent}{func_info[fn]} {fn};')
                    i += 1
                    continue
                # multi-line
                """
                %new {
                    ...
                }
                %end
                """
                block_lines = [modified_line]
                i += 1
                while i < len(lines):
                    block_lines.append(lines[i])
                    if re.search(r'%end\s*$', lines[i]):
                        i += 1
                        break
                    i += 1
                block_text = '\n'.join(block_lines)
                inner_match = re.search(r'%new\s*\{\s*(.*?)\s*\}', block_text, re.DOTALL)
                if inner_match:
                    inner_content = inner_match.group(1).strip()
                    # handle %new funcname and %call funcname(args)
                    lines_inner = inner_content.split('\n')
                    for inner_line in lines_inner:
                        inner_line = inner_line.strip()
                        if not inner_line:
                            continue
                        # handle %new func1 func2
                        if inner_line.startswith('%new'):
                            funcnames = inner_line.replace('%new', '').strip().split()
                            for fn in funcnames:
                                if fn in func_info:
                                    output.append(f'{base_indent}{func_info[fn]} {fn};')
                        # handle %call func(args) or other call
                        else:
                            inner_line = replace_func_calls(inner_line, func_info)
                            output.append(f'{base_indent}{inner_line}')
                continue
            # handle %new funcname1 funcname2 ... 
            new_pattern = r'^\s*%new\s+(.+)$'
            new_match = re.match(new_pattern, modified_line)
            if new_match:
                funcnames = new_match.group(1).split()
                base_indent = re.match(r'^(\s*)', modified_line).group(1)
                for fn in funcnames:
                    if fn in func_info:
                        output.append(f'{base_indent}{func_info[fn]} {fn};')
                i += 1
                continue
            # handle function calls in ordinary rows
            if 'class ' not in modified_line and '%new' not in modified_line:
                modified_line = replace_func_calls(modified_line, func_info)
            output.append(modified_line)
            i += 1
    return '\n'.join(output)

def main():
    parser = argparse.ArgumentParser(
        description='Convert pub functions to class wrappers in C++ files'
    )
    parser.add_argument('files', nargs='+', help='C++ files to process')
    parser.add_argument('-b', '--backup', action='store_true', 
                       help='Create backup files (.bak)')
    parser.add_argument('-i', '--inplace', nargs='?', const='.cc', default=None,
                       help='Add extension to output file (default: .cc)')
    parser.add_argument('-e', '--extensions', nargs='+',
                       help='Additional file extensions to treat as C++')
    args = parser.parse_args()
    # add an additional extension
    if args.extensions:
        for ext in args.extensions:
            if not ext.startswith('.'):
                ext = '.' + ext
            CPP_EXTENSIONS.add(ext)
    processed_count = 0
    skipped_count = 0
    error_count = 0
    for filepath in args.files:
        try:
            if not is_cpp_file(filepath):
                print(f"Skipping non-C++ file: {filepath}")
                skipped_count += 1
                continue
            # create a backup
            if args.backup:
                backup_path = filepath + '.bak'
                shutil.copy2(filepath, backup_path)
                print(f"Backup: {backup_path}")
            # read file
            with open(filepath, 'r', encoding='utf-8') as f:
                content = f.read()
            # process the content
            processed = process_cc_content(content, filepath)
            # determine the output file
            if args.inplace:
                ext = args.inplace if args.inplace.startswith('.') else '.' + args.inplace
                output_file = filepath + ext
            else:
                output_file = filepath
            # check whether there is an actual modification
            if processed != content:
                with open(output_file, 'w', encoding='utf-8') as f:
                    f.write(processed)
                print(f"Processed: {output_file}")
                processed_count += 1
            else:
                print(f"No changes: {filepath}")
                skipped_count += 1
        except UnicodeDecodeError:
            print(f"Encoding error (not UTF-8): {filepath}")
            error_count += 1
        except Exception as e:
            print(f"Error processing {filepath}: {e}")
            error_count += 1
    print(f"\nSummary: {processed_count} processed, {skipped_count} skipped, {error_count} errors")
    if error_count > 0:
        sys.exit(1)

if __name__ == '__main__':
    main()
