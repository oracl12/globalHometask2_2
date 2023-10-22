# Requirements:
Standard: **c++17**
Compliler: **g++**
OS: **Windows > 8.1; Linux**
Soft: **Qt libs || Qt Creator last-v** -> **Qt > 6**
Maker: **cmake**

# Diagram:
https://miro.com/app/board/uXjVNXsXl_Y=/?share_link_id=55229525742

# Description of classes:
## Main:
Start point of app, includes basic libs and starts general services
## MainWindow:
Main class of window, which handles all the ui elements on screen
## MainWindowUi:
Set of predefined buttons and other ui elements
## Loadability:
Class-handler of threads:
1. **Updating resources**
2. **Writing logs to the file**
## Other:
Small size header that contains function which is not compatible on all OS's
