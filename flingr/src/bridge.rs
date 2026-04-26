#[cxx::bridge]
mod ffi {
    unsafe extern "C++" {
        include!("ffi.hpp");

        fn run_file(filename: &CxxString);
    }
}
