# kmstest-2024
testing 

Found ninja-1.10.1 at /usr/bin/ninja
root@inhnjlux0031:/home/gadibasa/Desktop/KMS/kmstest-2024/kmsxx# ninja -C build
ninja: Entering directory `build'
[4/4] Linking target kmstest
root@inhnjlux0031:/home/gadibasa/Desktop/KMS/kmstest-2024/kmsxx# sudo ./build/utils/kmstest --device=/dev/dri/card2 list
Using device: /dev/dri/card2
File descriptor: 3
Available connectors:
Connector ID: 88 Type: 10 (4)
  Status: Disconnected
Connector ID: 91 Type: 10 (5)
  Status: Disconnected
Connector ID: 95 Type: 11 (3)
  Status: Disconnected
root@inhnjlux0031:/home/gadibasa/Desktop/KMS/kmstest-2024/kmsxx# sudo ./build/utils/kmstest --device=/dev/dri/card2 test 91 "1024x768" 60
Using device: /dev/dri/card2
File descriptor: 3
Connector 91 is not connected
root@inhnjlux0031:/home/gadibasa/Desktop/KMS/kmstest-2024/kmsxx# sudo ./build/utils/kmstest --device=/dev/dri/card2 fallback
sudo: unable to resolve host inhnjlux0031.ls.ege.ds: Name or service not known
Using device: /dev/dri/card2
File descriptor: 3
Fallback modes:
  1920x1080@60
  2560x1440@60
  3840x2160@30

