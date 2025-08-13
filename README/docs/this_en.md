## System Requirements 

- CMake 3.20 or above
- Clang/LLVM or gcc
- GNU Make or Ninja
- c++17 standard for compiling

## Notes 📝

**Settings**:
- ~Before start you must be sure MT_DIRS variable in CMakeLists.txt has adjusted~.
- This project using some system libs on freebsd.
- Check dependencies of this project.
- If you are using same libs in your mt but on custom path then delete them and use system libs.

⚠️ **ABI Compatibility**: 
- Compiler version of this project and mt project must be same.
- If you are using spesific version of clang, you should put full path into CMakeLists.txt:
  ```cmake
  set(CMAKE_C_COMPILER "/spesific/clang/path/clang")
  set(CMAKE_CXX_COMPILER "/spesific/clang/path/clang++")
  ```
- All presets in cmakepresets.json can be use with rsync & vs -remote installation-.

**Guide**:
- Download this repo to your freebsd.
- Run IMPL/bsd/scripts/setup-freebsd.sh to adjust freebsd environment.
- Use IMPL/mt folder for implementation guide.

## Compiling & Installation

You can use two different generator:

### With Unix Makefiles generator:
```bash
cmake --preset freebsd-remote-debug
cd out/build/freebsd-remote-debug
make
make install
```

### With Ninja generator:
```bash
# CMakePresets.json dosyasında generator'ı "Ninja" olarak değiştirin
cmake --preset freebsd-remote-debug
cd out/build/freebsd-remote-debug
ninja
ninja install
```

## Using this library

After compiling & installation:
- Build your mt and run.
- If you don't have license you don't need to add implementation.
- If your bridge server(a.k.a mobileServer) running on another remote vds and you've license then your mt automatically connect to bridgeServer.
- All things will be ready automatically.
- Logs of this lib will be put in your each mt-core file('logs/'); log system also have an automatic backup system('/backups').
- Your players can sign-in their mt accounts with using mobileApp on App/Play Store.

---

## 📖 Navigation

**📚 Documentation**: [← Previous](./packets_en.md) | [← Back to Main](../en.md)

**🌐 Language**: [Türkçe](./this_tr.md) | **English**

---

*Complete installation and setup guide for mobi-core!*
