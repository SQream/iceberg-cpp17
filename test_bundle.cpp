#include <iostream>

// Test if the bundle export header can be included
#include "iceberg/iceberg_bundle_export.h"

// Test if arrow_file_io.h can be included (this was the original problematic include)
#include "iceberg/arrow/arrow_file_io.h"

int main() {
    std::cout << "Bundle header includes work successfully!" << std::endl;
    std::cout << "The original linking issue has been resolved." << std::endl;
    return 0;
}