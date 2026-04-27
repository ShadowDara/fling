mod bridge;

fn main() {
    //let file = std::env::args().nth(1).unwrap();
    let file = "test.txt";
    
    bridge::ffi::run_file(&file);
}
