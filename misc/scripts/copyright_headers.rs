use std::env;
use std::fs;
use std::io::{self, Write};
use std::path::Path;
use std::process;

fn main() -> io::Result<()> {
    // Collect the filename from argv
    let mut args = env::args().skip(1);
    let fname = match args.next() {
        Some(f) => f,
        None => {
            eprintln!("Usage: add_header <filename>");
            process::exit(1);
        }
    };

    // Base header template with literal "$filename"
    let header_template = r#"/**************************************************************************/
/*  $filename                                                             */
/**************************************************************************/
/*                         This file is part of:                          */
/*                              Green Godot                               */
/*             https://github.com/Wolf-Pack-Clan/green-godot              */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/* Copyright (c) 2025 Wolf Pack                                           */
/*                                                                        */
/* This program is free software: you can redistribute it and/or modify   */
/* it under the terms of the GNU General Public License as published by   */
/* the Free Software Foundation, either version 3 of the License, or      */
/* (at your option) any later version.                                    */
/*                                                                        */
/* This program is distributed in the hope that it will be useful,        */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          */
/* GNU General Public License for more details.                           */
/*                                                                        */
/* You should have received a copy of the GNU General Public License      */
/* along with this program.  If not, see <https://www.gnu.org/licenses/>. */
/**************************************************************************/
"#;

    // Extract just the filename (strip any preceding path)
    let basename = Path::new(&fname)
        .file_name()
        .and_then(|n| n.to_str())
        .unwrap_or(&fname)
        .to_string();

    // Prepare padding logic to keep the same width as "$filename"
    let placeholder = "$filename".to_string();
    let mut repl_placeholder = placeholder.clone();
    let mut repl_filename = basename.clone();
    let len_pl = placeholder.len();
    let len_fn = basename.len();
    if len_fn < len_pl {
        repl_filename.extend(std::iter::repeat(' ').take(len_pl - len_fn));
    } else if len_fn > len_pl {
        repl_placeholder.extend(std::iter::repeat(' ').take(len_fn - len_pl));
    }

    // Build the actual header: prefer replacing the padded placeholder if it exists,
    // otherwise do a plain "$filename" replace.
    let header = if header_template.contains(&repl_placeholder) {
        header_template.replace(&repl_placeholder, &repl_filename)
    } else {
        header_template.replace("$filename", &basename)
    };

    // Read the existing file into lines
    let content = fs::read_to_string(&fname)?;
    let lines: Vec<&str> = content.lines().collect();

    // If there's already a Godot mention on line 5 (index 4), exit
    if lines.len() > 4 {
        if lines[4].replace(' ', "").to_lowercase().contains("godot") {
            process::exit(0);
        }
    }

    // Prepend header plus a blank line
    let mut new_content = String::new();
    new_content.push_str(&header);
    new_content.push('\n');
    new_content.push_str(&content);

    // Write it back
    let mut file = fs::File::create(&fname)?;
    file.write_all(new_content.as_bytes())?;

    Ok(())
}
