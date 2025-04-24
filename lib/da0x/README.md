# UUID Generator (v4)

This is a simple, lightweight **single-header library** for generating UUIDs in C++ (version 4, random-based UUIDs). The implementation follows [RFC 4122](https://www.ietf.org/rfc/rfc4122.txt), ensuring compliance with the format and standards for version 4 UUIDs.

## Features
- **Version 4 UUIDs**: Generates random-based UUIDs with version 4 specification.
- **RFC 4122 Compliant**: Ensures correct bit settings for version and variant fields.
- **Header-only**: No need for additional libraries or dependencies. Simply include the header and generate UUIDs.
- **Cross-platform**: Works across all platforms that support the C++ standard library.

## Namespace and Usage
The library uses namespaces to make the UUID generator modular and extendable. To generate a UUID, use the `uuid::v4::generate()` method.

### Example:

```
#include <iostream>
#include "uuid.hpp"

int main() {
    std::string new_uuid = uuid::v4::generate();
    std::cout << "Generated UUID: " << new_uuid << std::endl;
    return 0;
}
```

### Output:
The program will generate a UUID in the following format:

```
550e8400-e29b-41d4-a716-446655440000
```

### Installation
Since this is a single-header library, you only need to copy `uuid.hpp` into your project's include directory and start using it.

### Compliance with RFC 4122
This implementation strictly follows [RFC 4122](https://www.ietf.org/rfc/rfc4122.txt) for **version 4 UUIDs**. Here’s how it ensures compliance:

- **Version field**: The 13th through 16th bits are set to `0100`, which indicates a version 4 UUID (random-based).
- **Variant field**: The two most significant bits of the 17th hex digit are set to `10`, indicating an RFC 4122-compliant UUID variant.

For example, in the generated UUID `550e8400-e29b-41d4-a716-446655440000`:
- The **'4'** in the third section indicates that this is a **version 4 UUID**.
- The **'a'** in the fourth section indicates that the variant bits are `10xx`, confirming RFC 4122 compliance.

### How it Works
UUIDs are 128-bit identifiers made up of random bits in the case of version 4. The library uses C++'s `<random>` library to generate the random bits and structures them according to the UUID format.

The UUID format is split into five fields:

```
8-4-4-4-12
```


- **8 characters** (32 bits) – Random data.
- **4 characters** (16 bits) – Random data.
- **4 characters** (16 bits) – First digit is `4` (version).
- **4 characters** (16 bits) – First digit is `8`, `9`, `A`, or `B` (variant).
- **12 characters** (48 bits) – Random data.

### License
This library is licensed under the MIT License. Feel free to use it in your own projects!

### Contribution
Feel free to open issues or submit pull requests for improvements, optimizations, or feature requests.

