// logic/spiro-core-sys/build.rs

use std::env;
use std::path::PathBuf;
use std::process::Command;

fn main() {
    let manifest_dir = PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap());
    let project_root = manifest_dir.join("../..").canonicalize().unwrap();
    
    let cpp_build_artifacts_dir = project_root.join("target/cpp");
    let lib_dir = cpp_build_artifacts_dir.join("lib");
    let include_dir = cpp_build_artifacts_dir.join("include");

    if !lib_dir.exists() || !include_dir.exists() {
        eprintln!("Spiro-Core library not found. Attempting to build it with 'make build-core'...");
        let status = Command::new("make")
            .arg("build-core")
            .current_dir(&project_root)
            .status()
            .expect("Failed to execute 'make build-core'. Is 'make' installed and in your PATH?");
            
        if !status.success() {
             panic!("Failed to build the C++ core library. Please check the build output.");
        }

        if !lib_dir.exists() || !include_dir.exists() {
            panic!("C++ core library build did not produce the expected artifacts in target/cpp/. Halting.");
        }
    }

    println!("cargo:rustc-link-search=native={}", lib_dir.display());
    println!("cargo:rustc-link-lib=static=spiro-core");
    println!("cargo:rustc-link-lib=static=glfw3"); // Corrected name from 'glfw' to 'glfw3'

    if cfg!(target_os = "linux") {
        println!("cargo:rustc-link-lib=dylib=stdc++");
        println!("cargo:rustc-link-lib=dylib=X11");
        println!("cargo:rustc-link-lib=dylib=Xrandr");
        println!("cargo:rustc-link-lib=dylib=Xinerama");
        println!("cargo:rustc-link-lib=dylib=Xcursor");
        println!("cargo:rustc-link-lib=dylib=Xi");
        println!("cargo:rustc-link-lib=dylib=GL");
    } else if cfg!(target_os = "macos") {
        println!("cargo:rustc-link-lib=dylib=c++");
        println!("cargo:rustc-link-lib=framework=Cocoa");
        println!("cargo:rustc-link-lib=framework=OpenGL");
        println!("cargo:rustc-link-lib=framework=IOKit");
    }

    let header_path = include_dir.join("spirographicals/spirographicals.h");
    let header_path_str = header_path.to_str().expect("Header path is not valid UTF-8");
    
    println!("cargo:rerun-if-changed={}", header_path_str);

    let bindings = bindgen::Builder::default()
        .header(header_path_str)
        .parse_callbacks(Box::new(bindgen::CargoCallbacks::new()))
        .derive_default(true)
        .derive_debug(true)
        .derive_copy(true)
        .derive_hash(true)
        .derive_eq(true)
        .clang_arg(format!("-I{}", include_dir.display()))
        
        .allowlist_type("sp_.*_t")
        .allowlist_function("sp_.*")
        
        .opaque_type("sp_canvas_t")
        .opaque_type("sp_pen_t")
        .opaque_type("sp_path_t")
        .opaque_type("sp_image_t")
        .opaque_type("sp_font_t")
        .opaque_type("sp_gradient_t")
        .opaque_type("sp_shader_t")
        
        .default_enum_style(bindgen::EnumVariation::Rust { non_exhaustive: false })
        
        .generate()
        .expect("Unable to generate bindings for spiro-core.");

    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Couldn't write bindings to file!");
}
