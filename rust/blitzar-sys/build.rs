use std::env;
use std::fs::File;
use std::io::copy;
use std::path::PathBuf;
use std::process::Command;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    // If we are building docs.rs, we don't want to build the shared library.
    if std::env::var("DOCS_RS").is_ok() {
        return Ok(());
    }

    if !cfg!(target_os = "linux") {
        panic!("Unsupported OS. Only Linux is supported.");
    }

    if !cfg!(target_arch = "x86_64") {
        panic!("Unsupported architecture. Only x86_64 is supported.");
    }

    const LIB_NAME: &str = "blitzar-linux-x86_64";
    const SHARED_LIB: &str = "libblitzar-linux-x86_64.so";
    const PKG_VERSION: &str = env!("CARGO_PKG_VERSION");

    let out_dir = PathBuf::from(env::var("OUT_DIR")?);
    let lib_path = out_dir.join(SHARED_LIB);

    if PKG_VERSION == "0.0.0" {
        // This is used solely with local tests. It will build the shared library from source.
        assert!(Command::new("bash")
            .current_dir("../../")
            .arg("ci/build.sh")
            .arg(&lib_path)
            .arg("0.0.0")
            .status()
            .expect("Failed to run the build script")
            .success());
    } else {
        // Download the shared library from GitHub releases and place it in the `OUT_DIR`.
        let mut lib_file = File::create(&lib_path)?;
        let release_url = format!("http://github.com/spaceandtimefdn/blitzar/releases/download/v{PKG_VERSION}/{SHARED_LIB}");
        let mut response = reqwest::blocking::get(release_url)?;
        copy(&mut response, &mut lib_file)?;

        // Re-build the sys crate only under the following conditions
        println!("cargo:rerun-if-changed=build.rs");
        println!("cargo:rerun-if-env-changed=CARGO_PKG_VERSION");
    }

    println!("cargo:rustc-link-search=native={}", out_dir.display());
    println!("cargo:rustc-link-lib=dylib={LIB_NAME}");

    Ok(())
}
