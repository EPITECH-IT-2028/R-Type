# R-Type

## Tests
To run tests:

```sh
cmake -B .build -DENABLE_TESTS=ON -DCMAKE_CXX_FLAGS="--coverage"
cmake --build .build
.build/unit_tests
```

```sh
gcovr -r . --html --html-details -o tests/html/coverage.html
# macOS
open tests/html/coverage.html
# Linux
xdg-open tests/html/coverage.html || true
# Windows (PowerShell)
start tests/html/coverage.html
```