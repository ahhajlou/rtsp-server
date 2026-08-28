# RTSP Server

## Links

[RTP Header](https://www.rfc-editor.org/info/rfc6184/#section-5.1)

## build

release build

```bash
cmake -S . -B build -GNinja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

debug build

```bash
cmake -S . -B build-debug -GNinja -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug
```

## Run tests

```bash
cd build
cmake --build .
ctest
```

## Run fuzz test

```bash
cmake -S . -B build-fuzz -GNinja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=clang++ -DENABLE_FUZZING=ON
cmake --build build-fuzz

```
