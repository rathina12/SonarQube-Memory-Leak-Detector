# Installation and Deployment

## Requirements

- CMake 3.20+
- C++17 compiler (GCC/Clang/MSVC)
- JDK 17+
- Maven 3.9+
- SonarQube Server 2026.1+ recommended
- Optional: LLVM/Clang development packages for `MLPCA_WITH_CLANG=ON`

## Build analyzer

```bash
cmake -S analyzer -B build/analyzer -DCMAKE_BUILD_TYPE=Release
cmake --build build/analyzer --config Release
ctest --test-dir build/analyzer --output-on-failure
```

## Optional Clang LibTooling build

Generate a compilation database in the target project, commonly with CMake:

```bash
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

Then build this project with the development libraries installed:

```bash
cmake -S analyzer -B build/analyzer-clang -DMLPCA_WITH_CLANG=ON
cmake --build build/analyzer-clang
```

## Generate report

```bash
./build/analyzer/mlpca-analyzer /path/to/cpp/project --output /path/to/cpp/project/mlpca-report.json
```

Optional project memory API model:

```bash
./build/analyzer/mlpca-analyzer /path/to/project --config config/mlpca.yml --output mlpca-report.json
```

## Build plugin

```bash
cd sonar-plugin
mvn clean package
```

Copy the generated JAR into SonarQube's `extensions/plugins` directory and restart SonarQube.

## Configure scan

Add to `sonar-project.properties`:

```properties
sonar.mlpca.reportPaths=mlpca-report.json
```

Generate the MLPCA report before invoking `sonar-scanner`.
