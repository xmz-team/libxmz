# libxmz

## Introduce
a c++ library with simple functions  
**using namespace xmz;** is not recommended  
If you use **using namespace xmz;** or **using namespace xmz::fs;** etc., and use namespace other libraries or third-party libraries, the following bad situation will occur  
```cc
#include <string>
#include <filesystem>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <libxmz/io.hpp>
#include <libxmz/fs.hpp>
#include <apple-ios-objc.hpp>

using namespace xmz;
using namespace xmz::fs;
using namespace std;
using namespace std::filesystem;
int main() {
    rename("a", "b"); /* conflict with POSIX's rename */
    ios::ui::run(...); /* conflict with ios(std::ios) */
    print("XXX"); /* conflict with print(std::print) */
    println("XXX"); /* conflict with println(std::println) */
    ... /* more conflicts */
    return 0;
}
```
this is terrible, because there will be compilation conflicts, ambiguity, and unknown behavior at runtime...  

## Contact
**email: xmz-team@outlook.com**

---

