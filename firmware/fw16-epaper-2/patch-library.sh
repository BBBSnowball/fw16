set -xe
cd ~/.platformio/packages/framework-arduinopico/lib
ar x libpico.a runtime.c.obj
../../toolchain-rp2040-earlephilhower/bin/arm-none-eabi-objcopy runtime.c.obj runtime2.c.obj -W panic
mv runtime2.c.obj runtime.c.obj
ar r libpico.a runtime.c.obj

