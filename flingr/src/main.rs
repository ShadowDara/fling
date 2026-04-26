mod bridge;

fn main() {
    let file = std::ffi::CString::new("test.txt").unwrap();
    bridge::ffi::run_file(&file);
}
