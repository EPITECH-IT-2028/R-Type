# R-Type

## Tests
To run tests:

`
cmake -B .build -DENABLE_TESTS=ON -DCMAKE_CXX_FLAGS="--coverage" && cmake --build .build && .build/unit_tests
`

`
gcovr -r . --html --html-details -o tests/html/coverage.html
open tests/html/coverage.html
`